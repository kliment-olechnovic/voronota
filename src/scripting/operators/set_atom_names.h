#ifndef SCRIPTING_OPERATORS_SET_ATOM_NAMES_H_
#define SCRIPTING_OPERATORS_SET_ATOM_NAMES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAtomNames : public OperatorBase<SetAtomNames>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		std::string atom_name;
		std::string residue_name;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("atom_name")=atom_name;
			heterostorage.variant_object.value("residue_name")=residue_name;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string atom_name;
	std::string residue_name;

	SetAtomNames()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		atom_name=input.get_value_or_default<std::string>("atom-name", "");
		residue_name=input.get_value_or_default<std::string>("residue-name", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("atom-name", CDOD::DATATYPE_STRING, "atom name"));
		doc.set_option_decription(CDOD("residue-name", CDOD::DATATYPE_STRING, "residue name"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(atom_name.empty() && residue_name.empty())
		{
			throw std::runtime_error(std::string("No atom name or residue name provided."));
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::vector<Atom> atoms=data_manager.atoms();

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			Atom& atom=atoms[*it];
			if(!atom_name.empty())
			{
				if(atom_name=="_autofix_CA")
				{
					if(atom.crad.name==atoms[*it].crad.resName)
					{
						atom.crad.name="CA";
					}
				}
				else
				{
					atom.crad.name=atom_name;
				}
			}
			if(!residue_name.empty())
			{
				atom.crad.resName=residue_name;
			}
		}

		data_manager.reset_atoms_by_swapping(atoms);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);
		result.atom_name=atom_name;
		result.residue_name=residue_name;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ATOM_NAMES_H_ */
