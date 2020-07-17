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
		int residues_protein;
		int residues_nucleic;
		int residues_other;
		int ss_alpha;
		int ss_beta;
		int ss_loop;

		Result() :
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
			heterostorage.variant_object.value("residues_protein")=residues_protein;
			heterostorage.variant_object.value("residues_nucleic")=residues_nucleic;
			heterostorage.variant_object.value("residues_other")=residues_other;
			heterostorage.variant_object.value("ss_alpha")=ss_alpha;
			heterostorage.variant_object.value("ss_beta")=ss_beta;
			heterostorage.variant_object.value("ss_loop")=ss_loop;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string global_adj_prefix;

	SummarizeLinearStructure()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "");
	}

	void document(CommandDocumentation& doc) const
	{
		Utilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("ss-global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", ""));
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

		Result result;

		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), atom_ids);

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
