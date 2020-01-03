#ifndef SCRIPTING_OPERATORS_MARK_ATOMS_H_
#define SCRIPTING_OPERATORS_MARK_ATOMS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

template<bool positive>
class MarkAtomsTemplate
{
public:
	struct Result
	{
		SummaryOfAtoms atoms_summary;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;

	MarkAtomsTemplate()
	{
	}

	MarkAtomsTemplate& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_mark(positive).set_unmark(!positive), ids);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

typedef MarkAtomsTemplate<true> MarkAtoms;
typedef MarkAtomsTemplate<false> UnmarkAtoms;

}

}

#endif /* SCRIPTING_OPERATORS_MARK_ATOMS_H_ */

