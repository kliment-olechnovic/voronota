#ifndef SCRIPTING_OPERATORS_SET_CHAIN_NAMES_AND_RESIDUE_NUMBERS_BY_SEQUENCES_H_
#define SCRIPTING_OPERATORS_SET_CHAIN_NAMES_AND_RESIDUE_NUMBERS_BY_SEQUENCES_H_

#include "../operators_common.h"

#include "../../common/sequence_utilities.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetChainNamesAndResidueNumberBySequences : public OperatorBase<SetChainNamesAndResidueNumberBySequences>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string sequences_file;

	SetChainNamesAndResidueNumberBySequences()
	{
	}

	void initialize(CommandInput& input)
	{
		sequences_file=input.get_value<std::string>("sequences-file");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("sequences-file", CDOD::DATATYPE_STRING, "sequences input file"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::vector<std::string> sequences=common::SequenceUtilities::read_sequences_from_file(sequences_file);

		if(sequences.empty())
		{
			throw std::runtime_error(std::string("No sequences from file '")+sequences_file+"'");
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
			chain_info.closest_sequence_id=sequence_identities.begin()->second;
			chain_info.sequence_mapping=common::SequenceUtilities::construct_sequence_mapping(chains_info[i].residue_sequence_vector, sequences[chain_info.closest_sequence_id], "");
		}

		std::vector<std::size_t> order_of_chains;
		{
			std::vector< std::deque<std::size_t> > mapping_of_sequences_to_chains(sequences.size());
			for(std::size_t i=0;i<chain_infos.size();i++)
			{
				mapping_of_sequences_to_chains[chain_infos[i].closest_sequence_id].push_back(i);
			}

			order_of_chains.reserve(chain_infos.size());
			std::size_t j=0;
			bool something_left=true;
			while(order_of_chains.size()<chain_infos.size() && something_left)
			{
				if(!mapping_of_sequences_to_chains[j].empty())
				{
					order_of_chains.push_back(mapping_of_sequences_to_chains[j].front());
					mapping_of_sequences_to_chains[j].pop_front();
				}
				j++;
				if(j>=mapping_of_sequences_to_chains.size())
				{
					j=0;
					something_left=false;
					for(std::size_t i=0;i<mapping_of_sequences_to_chains.size() && !something_left;j++)
					{
						if(!mapping_of_sequences_to_chains[i].empty())
						{
							something_left=true;
						}
					}
				}
			}
		}

		Result result;
		return result;
	}

private:
	struct ChainInfo
	{
		std::vector<common::ChainResidueAtomDescriptor> residue_sequence_vector;
		std::string sequence_str;
		std::size_t closest_sequence_id;
		std::map<common::ChainResidueAtomDescriptor, int> sequence_mapping;

		ChainInfo() : closest_sequence_id(0)
		{
		}
	};
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_CHAIN_NAMES_AND_RESIDUE_NUMBERS_BY_SEQUENCES_H_ */
