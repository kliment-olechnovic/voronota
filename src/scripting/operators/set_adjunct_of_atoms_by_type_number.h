#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_TYPE_NUMBER_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_TYPE_NUMBER_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_using_voromqa.h"
#include "../primitive_chemistry_annotation.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctOfAtomsByTypeNumber : public OperatorBase<SetAdjunctOfAtomsByTypeNumber>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string name;
	std::string typing_mode;

	SetAdjunctOfAtomsByTypeNumber()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		name=input.get_value<std::string>("name");
		typing_mode=input.get_value<std::string>("typing-mode");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "adjunct name"));
		doc.set_option_decription(CDOD("typing-mode", CDOD::DATATYPE_STRING, "typing mode, possible values: CNOSP, protein_atom, protein_residue"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_adjunct_name_input(name, false);

		if(typing_mode!="CNOSP" && typing_mode!="protein_atom" && typing_mode!="protein_residue")
		{
			throw std::runtime_error(std::string("Invalid typing mode, valid options are: 'CNOSP', 'protein_atom', 'protein_residue'."));
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const Atom& atom=data_manager.atoms()[*it];
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			atom_adjuncts.erase(name);
			int value=-1;
			if(typing_mode=="CNOSP")
			{
				value=PrimitiveChemistryAnnotation::get_CNOSP_atom_type_number(atom);
			}
			else if(typing_mode=="protein_atom")
			{
				value=get_protein_atom_type_number(atom.crad, false);
			}
			else if(typing_mode=="protein_residue")
			{
				value=get_protein_atom_type_number(atom.crad, true);
			}
			if(value>=0)
			{
				atom_adjuncts[name]=value;
			}
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}

private:
	static int get_protein_atom_type_number(const common::ChainResidueAtomDescriptor& crad, const bool residue_level)
	{
		static std::map<common::ChainResidueAtomDescriptor, int> map_of_atom_type_numbers;
		static std::map<common::ChainResidueAtomDescriptor, int> map_of_residue_type_numbers;

		const std::map<common::ChainResidueAtomDescriptor, common::NormalDistributionParameters>& voromqa_means_and_sds=ScoringOfDataManagerUsingVoroMQA::Configuration::get_default_configuration().means_and_sds;

		if(residue_level)
		{
			if(map_of_residue_type_numbers.empty() && !voromqa_means_and_sds.empty())
			{
				for(std::map<common::ChainResidueAtomDescriptor, common::NormalDistributionParameters>::const_iterator it=voromqa_means_and_sds.begin();it!=voromqa_means_and_sds.end();++it)
				{
					map_of_residue_type_numbers[generalize_crad(it->first).without_atom()]=0;
				}
				int i=0;
				for(std::map<common::ChainResidueAtomDescriptor, int>::iterator it=map_of_residue_type_numbers.begin();it!=map_of_residue_type_numbers.end();++it)
				{
					it->second=i;
					i++;
				}
			}

			std::map<common::ChainResidueAtomDescriptor, int>::const_iterator it=map_of_residue_type_numbers.find(generalize_crad(crad).without_atom());
			if(it!=map_of_residue_type_numbers.end())
			{
				return it->second;
			}
		}
		else
		{
			if(map_of_atom_type_numbers.empty() && !voromqa_means_and_sds.empty())
			{
				int i=0;
				for(std::map<common::ChainResidueAtomDescriptor, common::NormalDistributionParameters>::const_iterator it=voromqa_means_and_sds.begin();it!=voromqa_means_and_sds.end();++it)
				{
					map_of_atom_type_numbers[generalize_crad(it->first)]=i;
					i++;
				}
			}

			std::map<common::ChainResidueAtomDescriptor, int>::const_iterator it=map_of_atom_type_numbers.find(generalize_crad(crad));
			if(it!=map_of_atom_type_numbers.end())
			{
				return it->second;
			}
		}

		return -1;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_TYPE_NUMBER_H_ */
