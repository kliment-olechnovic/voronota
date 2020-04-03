#ifndef SCRIPTING_OPERATORS_DELETE_SELECTIONS_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_DELETE_SELECTIONS_OF_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class DeleteSelectionsOfAtoms : public OperatorBase<DeleteSelectionsOfAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<std::string> deleted_selections;

		void store(HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<deleted_selections.size();i++)
			{
				heterostorage.variant_object.values_array("deleted_selections").push_back(VariantValue(deleted_selections[i]));
			}
		}
	};

	bool all;
	std::vector<std::string> names;

	DeleteSelectionsOfAtoms() : all(false)
	{
	}

	void initialize(CommandInput& input)
	{
		all=input.get_flag("all");
		names=input.get_value_vector_or_all_unused_unnamed_values("names");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("all", CDOD::DATATYPE_BOOL, "flag to delete all selections"));
		doc.set_option_decription(CDOD("names", CDOD::DATATYPE_STRING_ARRAY, "selection names", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_selections_availability();

		if(all && !names.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of names."));
		}

		std::vector<std::string> names_filled;

		if(all)
		{
			names_filled=data_manager.selection_manager().get_names_of_atoms_selections();
		}
		else
		{
			names_filled=names;
		}

		if(names_filled.empty())
		{
			throw std::runtime_error(std::string("No atoms selections names specified."));
		}

		data_manager.assert_atoms_selections_availability(names_filled);

		for(std::size_t i=0;i<names_filled.size();i++)
		{
			data_manager.selection_manager().delete_atoms_selection(names_filled[i]);
		}

		Result result;
		result.deleted_selections=names_filled;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_SELECTIONS_OF_ATOMS_H_ */
