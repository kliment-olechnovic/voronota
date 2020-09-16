#ifndef SCRIPTING_OPERATORS_MARK_ATOMS_H_
#define SCRIPTING_OPERATORS_MARK_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class MarkAtoms : public OperatorBase<MarkAtoms>
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

	MarkAtoms()
	{
	}

	virtual ~MarkAtoms()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_mark(positive()).set_unmark(!positive()), ids);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}

protected:
	virtual bool positive() const
	{
		return true;
	}
};

class UnmarkAtoms : public MarkAtoms
{
protected:
	bool positive() const
	{
		return false;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_MARK_ATOMS_H_ */

