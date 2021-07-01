#ifndef SCRIPTING_OPERATORS_SUMMARIZE_LINEAR_STRUCTURE_H_
#define SCRIPTING_OPERATORS_SUMMARIZE_LINEAR_STRUCTURE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SummarizeLinearStructure : public OperatorBase<SummarizeLinearStructure>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		int chains_all;
		int chains_protein;
		int chains_nucleic;
		int chains_other;
		int unique_chains_protein;
		int unique_chains_nucleic;
		int residues_protein;
		int residues_nucleic;
		int residues_other;
		int ss_alpha;
		int ss_beta;
		int ss_loop;
		std::vector<std::string> chain_names_all;
		std::vector<std::string> chain_names_protein;
		std::vector<std::string> chain_names_nucleic;
		std::vector<std::string> chain_names_other;

		Result() :
			chains_all(0),
			chains_protein(0),
			chains_nucleic(0),
			chains_other(0),
			unique_chains_protein(0),
			unique_chains_nucleic(0),
			residues_protein(0),
			residues_nucleic(0),
			residues_other(0),
			ss_alpha(0),
			ss_beta(0),
			ss_loop(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("chains_all")=chains_all;
			heterostorage.variant_object.value("chains_protein")=chains_protein;
			heterostorage.variant_object.value("chains_nucleic")=chains_nucleic;
			heterostorage.variant_object.value("chains_other")=chains_other;
			heterostorage.variant_object.value("unique_chains_protein")=unique_chains_protein;
			heterostorage.variant_object.value("unique_chains_nucleic")=unique_chains_nucleic;
			heterostorage.variant_object.value("residues_protein")=residues_protein;
			heterostorage.variant_object.value("residues_nucleic")=residues_nucleic;
			heterostorage.variant_object.value("residues_other")=residues_other;
			heterostorage.variant_object.value("ss_alpha")=ss_alpha;
			heterostorage.variant_object.value("ss_beta")=ss_beta;
			heterostorage.variant_object.value("ss_loop")=ss_loop;
			{
				std::vector<VariantValue>& va=heterostorage.variant_object.values_array("chain_names_all");
				for(std::size_t i=0;i<chain_names_all.size();i++)
				{
					va.push_back(VariantValue(chain_names_all[i]));
				}
			}
			{
				std::vector<VariantValue>& va=heterostorage.variant_object.values_array("chain_names_protein");
				for(std::size_t i=0;i<chain_names_protein.size();i++)
				{
					va.push_back(VariantValue(chain_names_protein[i]));
				}
			}
			{
				std::vector<VariantValue>& va=heterostorage.variant_object.values_array("chain_names_nucleic");
				for(std::size_t i=0;i<chain_names_nucleic.size();i++)
				{
					va.push_back(VariantValue(chain_names_nucleic[i]));
				}
			}
			{
				std::vector<VariantValue>& va=heterostorage.variant_object.values_array("chain_names_other");
				for(std::size_t i=0;i<chain_names_other.size();i++)
				{
					va.push_back(VariantValue(chain_names_other[i]));
				}
			}
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string global_adj_prefix;
	double min_seq_identity;

	SummarizeLinearStructure() : min_seq_identity(1.0)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "");
		min_seq_identity=input.get_value_or_default<double>("min-seq-identity", 1.0);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("ss-global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", ""));
		doc.set_option_decription(CDOD("min-seq-identity", CDOD::DATATYPE_FLOAT, "minimum sequence identity threshold", 1.0));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::set<std::size_t> residue_ids;
		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			residue_ids.insert(data_manager.primary_structure_info().map_of_atoms_to_residues[*it]);
		}

		std::set<std::size_t> chain_ids;
		for(std::set<std::size_t>::const_iterator it=residue_ids.begin();it!=residue_ids.end();++it)
		{
			chain_ids.insert(data_manager.primary_structure_info().map_of_residues_to_chains[*it]);
		}

		Result result;

		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), atom_ids);

		for(std::set<std::size_t>::const_iterator it=chain_ids.begin();it!=chain_ids.end();++it)
		{
			const common::ConstructionOfPrimaryStructure::Chain& ps_chain=data_manager.primary_structure_info().chains[*it];
			if(ps_chain.prevalent_residue_type==common::ConstructionOfPrimaryStructure::RESIDUE_TYPE_AMINO_ACID)
			{
				result.chains_protein++;
				result.chain_names_protein.push_back(ps_chain.name);
			}
			else if(ps_chain.prevalent_residue_type==common::ConstructionOfPrimaryStructure::RESIDUE_TYPE_NUCLEOTIDE)
			{
				result.chains_nucleic++;
				result.chain_names_nucleic.push_back(ps_chain.name);
			}
			else
			{
				result.chains_other++;
				result.chain_names_other.push_back(ps_chain.name);
			}
			result.chains_all++;
			result.chain_names_all.push_back(ps_chain.name);
		}

		{
			std::map<std::size_t, std::string> map_of_chains_sequences;
			for(std::set<std::size_t>::const_iterator chain_ids_it=chain_ids.begin();chain_ids_it!=chain_ids.end();++chain_ids_it)
			{
				const common::ConstructionOfPrimaryStructure::Chain& ps_chain=data_manager.primary_structure_info().chains[*chain_ids_it];
				std::ostringstream output_for_residue_sequence;
				for(std::vector<std::size_t>::const_iterator residue_ids_it=ps_chain.residue_ids.begin();residue_ids_it!=ps_chain.residue_ids.end();++residue_ids_it)
				{
					const common::ConstructionOfPrimaryStructure::Residue& r=data_manager.primary_structure_info().residues[*residue_ids_it];
					output_for_residue_sequence << auxiliaries::ResidueLettersCoding::convert_residue_code_big_to_small(r.chain_residue_descriptor.resName);
				}
				map_of_chains_sequences[*chain_ids_it]=output_for_residue_sequence.str();
			}
			std::set<std::size_t> representative_chains_protein;
			std::set<std::size_t> representative_chains_nucleic;
			std::set<std::size_t> repeated_chains;
			for(std::map<std::size_t, std::string>::const_iterator it1=map_of_chains_sequences.begin();it1!=map_of_chains_sequences.end();++it1)
			{
				if(repeated_chains.count(it1->first)==0)
				{
					const common::ConstructionOfPrimaryStructure::Chain& ps_chain1=data_manager.primary_structure_info().chains[it1->first];
					if(ps_chain1.prevalent_residue_type==common::ConstructionOfPrimaryStructure::RESIDUE_TYPE_AMINO_ACID)
					{
						representative_chains_protein.insert(it1->first);
					}
					else if(ps_chain1.prevalent_residue_type==common::ConstructionOfPrimaryStructure::RESIDUE_TYPE_NUCLEOTIDE)
					{
						representative_chains_nucleic.insert(it1->first);
					}
					std::map<std::size_t, std::string>::const_iterator it2=it1;
					++it2;
					for(;it2!=map_of_chains_sequences.end();++it2)
					{
						const common::ConstructionOfPrimaryStructure::Chain& ps_chain2=data_manager.primary_structure_info().chains[it2->first];
						if(ps_chain1.prevalent_residue_type==ps_chain2.prevalent_residue_type && voronota::common::SequenceUtilities::calculate_sequence_identity(it1->second, it2->second)>=min_seq_identity)
						{
							repeated_chains.insert(it2->first);
						}
					}
				}
			}
			result.unique_chains_protein=static_cast<int>(representative_chains_protein.size());
			result.unique_chains_nucleic=static_cast<int>(representative_chains_nucleic.size());
		}

		for(std::set<std::size_t>::const_iterator it=residue_ids.begin();it!=residue_ids.end();++it)
		{
			const common::ConstructionOfPrimaryStructure::Residue& ps_residue=data_manager.primary_structure_info().residues[*it];
			if(ps_residue.residue_type==common::ConstructionOfPrimaryStructure::RESIDUE_TYPE_AMINO_ACID)
			{
				result.residues_protein++;
				const common::ConstructionOfSecondaryStructure::ResidueDescriptor& ss_residue_descriptor=data_manager.secondary_structure_info().residue_descriptors[*it];
				if(ss_residue_descriptor.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX)
				{
					result.ss_alpha++;
				}
				else if(ss_residue_descriptor.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND)
				{
					result.ss_beta++;
				}
				else
				{
					result.ss_loop++;
				}
			}
			else if(ps_residue.residue_type==common::ConstructionOfPrimaryStructure::RESIDUE_TYPE_NUCLEOTIDE)
			{
				result.residues_nucleic++;
			}
			else
			{
				result.residues_other++;
			}
		}

		if(!global_adj_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_chains_all"]=result.chains_all;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_chains_protein"]=result.chains_protein;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_chains_nucleic"]=result.chains_nucleic;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_chains_other"]=result.chains_other;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_unique_chains_protein"]=result.unique_chains_protein;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_unique_chains_nucleic"]=result.unique_chains_nucleic;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_residues_protein"]=result.residues_protein;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_residues_nucleic"]=result.residues_nucleic;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_residues_other"]=result.residues_other;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_ss_alpha"]=result.ss_alpha;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_ss_beta"]=result.ss_beta;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_ss_loop"]=result.ss_loop;
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SUMMARIZE_LINEAR_STRUCTURE_H_ */
