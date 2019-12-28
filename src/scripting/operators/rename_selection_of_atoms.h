#ifndef SCRIPTING_OPERATORS_RENAME_SELECTION_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_RENAME_SELECTION_OF_ATOMS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class RenameSelectionOfAtoms
{
public:
	struct Result
	{
		Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	std::string name_a;
	std::string name_b;

	RenameSelectionOfAtoms()
	{
	}

	RenameSelectionOfAtoms& init(CommandInput& input)
	{
		const std::vector<std::string>& names=input.get_list_of_unnamed_values();
		input.mark_all_unnamed_values_as_used();
		if(names.size()!=2)
		{
			throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
		}
		name_a=names[0];
		name_b=names[1];
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_selections_availability();

		if(name_a.empty())
		{
			throw std::runtime_error(std::string("Empty first name provided for renaming."));
		}

		if(name_b.empty())
		{
			throw std::runtime_error(std::string("Empty second name provided for renaming."));
		}

		const std::set<std::size_t> ids=data_manager.selection_manager().get_atoms_selection(name_a);

		if(name_a!=name_b.empty)
		{
			data_manager.selection_manager().set_atoms_selection(name_b, ids);
			data_manager.selection_manager().delete_atoms_selection(name_a);
		}

		Result result;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_RENAME_SELECTION_OF_ATOMS_H_ */
