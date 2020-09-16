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
	std::string name;
	bool mark;

	SelectAtoms() : mark(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		name=input.get_value_or_first_unused_unnamed_value_or_default("name", "");
		mark=input.get_flag("mark");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "atom selection name", ""));
		doc.set_option_decription(CDOD("mark", CDOD::DATATYPE_BOOL, "flag to mark selected atoms"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_selection_name_input(name, true);

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
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
