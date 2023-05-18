#ifndef SCRIPTING_OPERATORS_SET_CHAIN_NAMES_AND_RESIDUE_NUMBERS_BY_SEQUENCES_H_
#define SCRIPTING_OPERATORS_SET_CHAIN_NAMES_AND_RESIDUE_NUMBERS_BY_SEQUENCES_H_

#include <deque>

#include "../operators_common.h"

#include "../../common/sequence_utilities.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetChainNamesAndResidueNumbersBySequences : public OperatorBase<SetChainNamesAndResidueNumbersBySequences>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<std::string> done_mappings;

		void store(HeterogeneousStorage& heterostorage) const
		{
			std::vector<VariantValue>& va=heterostorage.variant_object.values_array("done_mappings");
			for(std::size_t i=0;i<done_mappings.size();i++)
			{
				va.push_back(VariantValue(done_mappings[i]));
			}
		}
	};

	std::string sequences_file;
	std::vector<int> stoichiometry_vector;
	std::string stoichiometry_string;
	double min_sequence_identity;
	bool require_all_chains;

	SetChainNamesAndResidueNumbersBySequences() : min_sequence_identity(0.0), require_all_chains(false)
	{
	}

	void initialize(CommandInput& input)
	{
		sequences_file=input.get_value<std::string>("sequences-file");
		stoichiometry_vector=input.get_value_vector_or_default<int>("stoichiometry-vector", std::vector<int>());
		stoichiometry_string=input.get_value_or_default<std::string>("stoichiometry-string", std::string());
		min_sequence_identity=input.get_value_or_default<double>("min-sequence-identity", 0.0);
		require_all_chains=input.get_flag("require-all-chains");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("sequences-file", CDOD::DATATYPE_STRING, "sequences input file"));
		doc.set_option_decription(CDOD("stoichiometry-vector", CDOD::DATATYPE_INT_ARRAY, "chain counts for every sequence", ""));
		doc.set_option_decription(CDOD("stoichiometry-string", CDOD::DATATYPE_STRING, "stoichiometry descriptor to derive chain counts for every sequence", ""));
		doc.set_option_decription(CDOD("min-sequence-identity", CDOD::DATATYPE_FLOAT, "min allowed sequence identity", "0.0"));
		doc.set_option_decription(CDOD("require-all-chains", CDOD::DATATYPE_BOOL, "flag to require all chains to be present"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(stoichiometry_vector.empty() && stoichiometry_string.empty())
		{
			throw std::runtime_error(std::string("Not stoichiometry info provided"));
		}

		if(!stoichiometry_vector.empty() && !stoichiometry_string.empty())
		{
			throw std::runtime_error(std::string("Conflicting stoichiometry info provided"));
		}

		std::vector<int> chain_counts;
		if(!stoichiometry_vector.empty())
		{
			chain_counts=stoichiometry_vector;
		}
		else
		{
			std::string input_str=stoichiometry_string;
			for(std::size_t i=0;i<input_str.size();i++)
			{
				if(input_str[i]<'0' || input_str[i]>'9')
				{
					input_str[i]=' ';
				}
			}
			std::istringstream input_stream(input_str);
			while(input_stream.good())
			{
				int chain_count=0;
				input_stream >> chain_count;
				chain_counts.push_back(chain_count);
			}
		}

		if(chain_counts.empty())
		{
			throw std::runtime_error(std::string("Empty list of chain counts derived from stoichiometry info"));
		}

		int sum_of_chain_counts=0;
		for(std::size_t i=0;i<chain_counts.size();i++)
		{
			if(chain_counts[i]<=0)
			{
				throw std::runtime_error(std::string("Not every chain count is positive"));
			}
			sum_of_chain_counts+=chain_counts[i];
		}

		const std::vector<std::string> sequences=common::SequenceUtilities::read_sequences_from_file(sequences_file);

		if(sequences.empty())
		{
			throw std::runtime_error(std::string("No sequences from file '")+sequences_file+"'");
		}

		if(sequences.size()!=chain_counts.size())
		{
			throw std::runtime_error(std::string("Number of sequences is not equal to the number of chain counts"));
		}

		for(std::size_t i=0;i<sequences.size();i++)
		{
			if(sequences[i].empty())
			{
				throw std::runtime_error(std::string("Some empty sequences in file '")+sequences_file+"'");
			}
		}

		std::vector<ChainInfo> chain_infos(data_manager.primary_structure_info().chains.size());
		for(std::size_t i=0;i<data_manager.primary_structure_info().chains.size();i++)
		{
			const common::ConstructionOfPrimaryStructure::Chain& chain=data_manager.primary_structure_info().chains[i];
			ChainInfo& chain_info=chain_infos[i];
			chain_info.original_chain_name=chain.name;
			chain_info.residue_sequence_vector.reserve(chain.residue_ids.size());
			for(std::size_t j=0;j<chain.residue_ids.size();j++)
			{
				chain_info.residue_sequence_vector.push_back(data_manager.primary_structure_info().residues[chain.residue_ids[j]].chain_residue_descriptor);
			}
			chain_info.sequence_str=common::SequenceUtilities::convert_residue_sequence_container_to_string(chain_info.residue_sequence_vector);
		}

		for(std::size_t i=0;i<chain_infos.size();i++)
		{
			ChainInfo& chain_info=chain_infos[i];
			std::set< std::pair<double, std::size_t> > sequence_identities;
			for(std::size_t j=0;j<sequences.size();j++)
			{
				sequence_identities.insert(
						std::pair<double, std::size_t>(
								(0.0-common::SequenceUtilities::calculate_sequence_identity(chain_info.sequence_str, sequences[j])), j));
			}
			chain_info.best_sequence_identity=(0.0-sequence_identities.begin()->first);
			chain_info.closest_sequence_id=sequence_identities.begin()->second;
			chain_info.sequence_mapping=common::SequenceUtilities::construct_sequence_mapping(chain_info.residue_sequence_vector, sequences[chain_info.closest_sequence_id], true, false, 0, "");
		}

		std::vector< std::deque<std::string> > mapping_of_sequences_to_chain_names(sequences.size());
		{
			char current_chain_name='A';
			std::size_t j=0;
			bool something_left=true;
			while(something_left)
			{
				if(mapping_of_sequences_to_chain_names[j].size()<static_cast<std::size_t>(chain_counts[j]))
				{
					mapping_of_sequences_to_chain_names[j].push_back(std::string(1, current_chain_name));
					if(current_chain_name=='Z')
					{
						current_chain_name='a';
					}
					else if(current_chain_name=='z')
					{
						throw std::runtime_error("Too many chains to guess their names.");
					}
					else
					{
						current_chain_name++;
					}
				}
				j++;
				if(j>=mapping_of_sequences_to_chain_names.size())
				{
					j=0;
					something_left=false;
					for(std::size_t i=0;i<mapping_of_sequences_to_chain_names.size() && !something_left;i++)
					{
						if(mapping_of_sequences_to_chain_names[i].size()<static_cast<std::size_t>(chain_counts[i]))
						{
							something_left=true;
						}
					}
				}
			}
		}

		std::vector< std::deque<std::size_t> > mapping_of_sequences_to_chains(sequences.size());
		{
			std::vector< std::set< std::pair<double, std::size_t> > > mapping_of_sequences_to_chains_ordered(sequences.size());
			for(std::size_t i=0;i<chain_infos.size();i++)
			{
				mapping_of_sequences_to_chains_ordered[chain_infos[i].closest_sequence_id].insert(std::make_pair(0.0-chain_infos[i].best_sequence_identity, i));
			}
			for(std::size_t i=0;i<mapping_of_sequences_to_chains_ordered.size();i++)
			{
				for(std::set< std::pair<double, std::size_t> >::const_iterator it=mapping_of_sequences_to_chains_ordered[i].begin();it!=mapping_of_sequences_to_chains_ordered[i].end();++it)
				{
					mapping_of_sequences_to_chains[i].push_back(it->second);
				}
			}
		}

		std::vector<std::size_t> order_of_chains;
		{
			order_of_chains.reserve(chain_infos.size());
			std::size_t j=0;
			bool something_left=true;
			while(something_left)
			{
				if(!mapping_of_sequences_to_chains[j].empty())
				{
					if(!mapping_of_sequences_to_chain_names[j].empty())
					{
						const std::size_t chain_id=mapping_of_sequences_to_chains[j].front();
						mapping_of_sequences_to_chains[j].pop_front();
						const std::string chain_name=mapping_of_sequences_to_chain_names[j].front();
						mapping_of_sequences_to_chain_names[j].pop_front();
						order_of_chains.push_back(chain_id);
						chain_infos[chain_id].new_chain_name=chain_name;
					}
					else
					{
						mapping_of_sequences_to_chains[j].pop_front();
					}
				}
				j++;
				if(j>=mapping_of_sequences_to_chains.size())
				{
					j=0;
					something_left=false;
					for(std::size_t i=0;i<mapping_of_sequences_to_chains.size() && !something_left;i++)
					{
						if(!mapping_of_sequences_to_chains[i].empty())
						{
							something_left=true;
						}
					}
				}
			}
		}

		if(require_all_chains && static_cast<int>(order_of_chains.size())!=sum_of_chain_counts)
		{
			throw std::runtime_error("Final chains count does not match the requested chains count.");
		}

		std::vector<Atom> fixed_atoms;
		fixed_atoms.reserve(data_manager.atoms().size());

		for(std::size_t i=0;i<order_of_chains.size();i++)
		{
			const std::size_t chain_id=order_of_chains[i];
			const ChainInfo& chain_info=chain_infos[chain_id];
			const common::ConstructionOfPrimaryStructure::Chain& chain=data_manager.primary_structure_info().chains[chain_id];
			for(std::size_t j=0;j<chain.residue_ids.size();j++)
			{
				const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[chain.residue_ids[j]];
				std::map<common::ChainResidueAtomDescriptor, int>::const_iterator sequence_mapping_it=chain_info.sequence_mapping.find(residue.chain_residue_descriptor);
				if(sequence_mapping_it!=chain_info.sequence_mapping.end())
				{
					if(min_sequence_identity>0.0)
					{
						if(chain_info.best_sequence_identity<min_sequence_identity)
						{
							throw std::runtime_error("Not all chains satisfy minimum sequence identity constraint");
						}
					}
					for(std::size_t e=0;e<residue.atom_ids.size();e++)
					{
						Atom atom=data_manager.atoms()[residue.atom_ids[e]];
						atom.crad.chainID=chain_info.new_chain_name;
						atom.crad.resSeq=sequence_mapping_it->second;
						fixed_atoms.push_back(atom);
					}
				}
			}
		}

		data_manager.reset_atoms_by_swapping(fixed_atoms);

		Result result;

		for(std::size_t i=0;i<chain_infos.size();i++)
		{
			const ChainInfo& chain_info=chain_infos[i];
			std::ostringstream output;
			output << chain_info.original_chain_name << " " << chain_info.best_sequence_identity << " " << chain_info.closest_sequence_id << " " << chain_info.new_chain_name;
			result.done_mappings.push_back(output.str());
		}

		return result;
	}

private:
	struct ChainInfo
	{
		std::string original_chain_name;
		std::vector<common::ChainResidueAtomDescriptor> residue_sequence_vector;
		std::string sequence_str;
		double best_sequence_identity;
		std::size_t closest_sequence_id;
		std::map<common::ChainResidueAtomDescriptor, int> sequence_mapping;
		std::string new_chain_name;

		ChainInfo() : best_sequence_identity(0.0), closest_sequence_id(0)
		{
		}
	};
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_CHAIN_NAMES_AND_RESIDUE_NUMBERS_BY_SEQUENCES_H_ */
