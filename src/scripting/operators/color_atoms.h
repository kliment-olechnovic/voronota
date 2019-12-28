#ifndef SCRIPTING_OPERATORS_COLOR_ATOMS_H_
#define SCRIPTING_OPERATORS_COLOR_ATOMS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class ColorAtoms
{
public:
	struct Result
	{
		SummaryOfAtoms atoms_summary;

		Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			return (*this);
		}
	};

	const SelectionManager::Query parameters_for_selecting;
	const std::vector<std::string> representation_names;
	const bool next_random_color;
	const auxiliaries::ColorUtilities::ColorInteger color_value;

	ColorAtoms() : next_random_color(false)
	{
	}

	ColorAtoms& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		next_random_color=input.get_flag("next-random-color");
		color_value=(next_random_color ? Utilities::get_next_random_color() : Utilities::read_color(input));
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_atoms_representations_availability();

		const std::set<std::size_t> representation_ids=data_manager.atoms_representation_descriptor().ids_by_names(representation_names);

		if(!auxiliaries::ColorUtilities::color_valid(color_value))
		{
			throw std::runtime_error(std::string("Atoms color not specified."));
		}

		const std::set<std::size_t> ids=data_manager.filter_atoms_drawable_implemented_ids(
				representation_ids,
				data_manager.selection_manager().select_atoms(parameters_for_selecting),
				false);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable atoms selected."));
		}

		data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_color(color_value), ids);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_COLOR_ATOMS_H_ */
