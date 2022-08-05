#ifndef SCRIPTING_OPERATORS_SELECT_ATOMS_H_
#define SCRIPTING_OPERATORS_SELECT_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SelectAtoms : public OperatorBase<SelectAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::set<std::size_t> ids;
		SummaryOfAtoms atoms_summary;
		std::string selection_name;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("selection_name")=selection_name;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string motif;
	std::string name;
	bool mark;

	SelectAtoms() : mark(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		motif=input.get_value_or_default<std::string>("motif", "");
		name=input.get_value_or_first_unused_unnamed_value_or_default("name", "");
		mark=input.get_flag("mark");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("motif", CDOD::DATATYPE_STRING, "sequence motif", ""));
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "atom selection name", ""));
		doc.set_option_decription(CDOD("mark", CDOD::DATATYPE_BOOL, "flag to mark selected atoms"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_selection_name_input(name, true);

		std::set<std::size_t> ids;

		if(motif.empty())
		{
			ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		}
		else
		{
			SelectionManager::Query restricted_parameters_for_selecting=parameters_for_selecting;
			if(restricted_parameters_for_selecting.restrict_from_ids(common::ConstructionOfPrimaryStructure::collect_atom_ids_from_residue_ids(data_manager.primary_structure_info(), common::ConstructionOfPrimaryStructure::find_residue_ids_of_motif(data_manager.primary_structure_info(), motif))))
			{
				ids=data_manager.selection_manager().select_atoms(restricted_parameters_for_selecting);
			}
		}

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		if(!name.empty())
		{
			data_manager.selection_manager().set_atoms_selection(name, ids);
		}

		if(mark)
		{
			data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_unmark(true));
			data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_mark(true), ids);
		}

		Result result;
		result.ids=ids;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);
		result.selection_name=name;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SELECT_ATOMS_H_ */
