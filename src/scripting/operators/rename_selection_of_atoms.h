#ifndef SCRIPTING_OPERATORS_RENAME_SELECTION_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_RENAME_SELECTION_OF_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class RenameSelectionOfAtoms : public OperatorBase<RenameSelectionOfAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string name_original;
	std::string name_new;

	RenameSelectionOfAtoms()
	{
	}

	void initialize(CommandInput& input)
	{
		name_original=input.get_value_or_first_unused_unnamed_value("original");
		name_new=input.get_value_or_first_unused_unnamed_value("new");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("original", CDOD::DATATYPE_STRING, "original selection name"));
		doc.set_option_decription(CDOD("new", CDOD::DATATYPE_STRING, "new selection name"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_selections_availability();

		if(name_original.empty())
		{
			throw std::runtime_error(std::string("Empty first name provided for renaming."));
		}

		if(name_new.empty())
		{
			throw std::runtime_error(std::string("Empty second name provided for renaming."));
		}

		const std::set<std::size_t> ids=data_manager.selection_manager().get_atoms_selection(name_original);

		if(name_original!=name_new)
		{
			data_manager.selection_manager().set_atoms_selection(name_new, ids);
			data_manager.selection_manager().delete_atoms_selection(name_original);
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_RENAME_SELECTION_OF_ATOMS_H_ */
