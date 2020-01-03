#ifndef SCRIPTING_OPERATORS_COLOR_FIGURES_H_
#define SCRIPTING_OPERATORS_COLOR_FIGURES_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class ColorFigures
{
public:
	struct Result
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	std::vector<std::string> name;
	std::vector<std::string> representation_names;
	bool next_random_color;
	auxiliaries::ColorUtilities::ColorInteger color_value;

	ColorFigures() : next_random_color(false), color_value(auxiliaries::ColorUtilities::null_color())
	{
	}

	ColorFigures& init(CommandInput& input)
	{
		name=input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		next_random_color=input.get_flag("next-random-color");
		color_value=(next_random_color ? Utilities::get_next_random_color() : Utilities::read_color(input));
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_figures_availability();
		data_manager.assert_figures_representations_availability();

		const std::set<std::size_t> representation_ids=data_manager.figures_representation_descriptor().ids_by_names(representation_names);

		if(!auxiliaries::ColorUtilities::color_valid(color_value))
		{
			throw std::runtime_error(std::string("Figure color not specified."));
		}

		const std::set<std::size_t> ids=data_manager.filter_figures_drawable_implemented_ids(
				representation_ids,
				LongName::match(data_manager.figures(), LongName(name)),
				false);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable figures selected."));
		}

		data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_color(color_value), ids);

		Result result;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_COLOR_FIGURES_H_ */
