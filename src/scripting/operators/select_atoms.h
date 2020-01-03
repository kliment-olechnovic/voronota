#ifndef SCRIPTING_OPERATORS_SELECT_ATOMS_H_
#define SCRIPTING_OPERATORS_SELECT_ATOMS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class SelectAtoms
{
public:
	struct Result
	{
		std::set<std::size_t> ids;
		SummaryOfAtoms atoms_summary;
		std::string selection_name;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("selection_name")=selection_name;
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string name;
	bool mark;

	SelectAtoms() : mark(false)
	{
	}

	SelectAtoms& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		name=(input.is_any_unnamed_value_unused() ? input.get_value_or_first_unused_unnamed_value("name") : input.get_value_or_default<std::string>("name", ""));
		mark=input.get_flag("mark");
		return (*this);
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

#endif /* SCRIPTING_OPERATORS_SELECT_ATOMS_H_ */
