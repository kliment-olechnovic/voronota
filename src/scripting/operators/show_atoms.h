#ifndef SCRIPTING_OPERATORS_SHOW_ATOMS_H_
#define SCRIPTING_OPERATORS_SHOW_ATOMS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

template<bool positive>
class ShowAtomsTemplate
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
	std::vector<std::string> representation_names;

	ShowAtomsTemplate()
	{
	}

	ShowAtomsTemplate& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_atoms_representations_availability();

		std::set<std::size_t> representation_ids=data_manager.atoms_representation_descriptor().ids_by_names(representation_names);

		if(positive && representation_ids.empty())
		{
			representation_ids.insert(0);
		}

		const std::set<std::size_t> ids=data_manager.filter_atoms_drawable_implemented_ids(
				representation_ids,
				data_manager.selection_manager().select_atoms(parameters_for_selecting),
				false);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable atoms selected."));
		}

		data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_show(positive).set_hide(!positive), ids);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

typedef ShowAtomsTemplate<true> ShowAtoms;
typedef ShowAtomsTemplate<false> HideAtoms;

}

}

#endif /* SCRIPTING_OPERATORS_SHOW_ATOMS_H_ */
