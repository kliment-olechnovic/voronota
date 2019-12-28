#ifndef SCRIPTING_OPERATORS_DELETE_SELECTIONS_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_DELETE_SELECTIONS_OF_ATOMS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class DeleteSelectionsOfAtoms
{
public:
	struct Result
	{
		std::vector<std::string> deleted_selections;

		Result& write(HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<deleted_selections.size();i++)
			{
				heterostorage.variant_object.values_array("deleted_selections").push_back(VariantValue(deleted_selections[i]));
			}
			return (*this);
		}
	};

	bool all;
	std::vector<std::string> names;

	DeleteSelectionsOfAtoms() : all(false)
	{
	}

	DeleteSelectionsOfAtoms& init(CommandInput& input)
	{
		all=input.get_flag("all");
		if(!all)
		{
			names=input.get_list_of_unnamed_values();
			input.mark_all_unnamed_values_as_used();
		}
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_selections_availability();

		if(all && !names.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of names."));
		}

		if(all)
		{
			names=data_manager.selection_manager().get_names_of_atoms_selections();
		}

		if(names.empty())
		{
			throw std::runtime_error(std::string("No atoms selections names specified."));
		}

		data_manager.assert_atoms_selections_availability(names);

		for(std::size_t i=0;i<names.size();i++)
		{
			data_manager.selection_manager().delete_atoms_selection(names[i]);
		}

		Result result;
		result.deleted_selections=names;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_SELECTIONS_OF_ATOMS_H_ */
