#ifndef SCRIPTING_OPERATORS_COLOR_FIGURES_OF_LABELS_H_
#define SCRIPTING_OPERATORS_COLOR_FIGURES_OF_LABELS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ColorFiguresOfLabels : public OperatorBase<ColorFiguresOfLabels>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		Result()
		{
		}

		void store(HeterogeneousStorage&) const
		{
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	std::string mode;
	std::vector<std::string> figure_name_start;
	std::string color_for_text;
	std::string color_for_outline;

	ColorFiguresOfLabels()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		mode=input.get_value_or_default<std::string>("mode", "residue");
		color_for_text=input.get_value_or_default<std::string>("color-for-text", "");
		color_for_outline=input.get_value_or_default<std::string>("color-for-outline", "");
		std::vector<std::string> default_figure_name_start;
		default_figure_name_start.push_back("label");
		default_figure_name_start.push_back(mode);
		figure_name_start=input.get_value_vector_or_default<std::string>("figure-name-start", default_figure_name_start);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("figure-name", CDOD::DATATYPE_STRING_ARRAY, "figure name", "'label' 'mode value'"));
		doc.set_option_decription(CDOD("mode", CDOD::DATATYPE_STRING, "labeling mode, atom or residue", "atom"));
		doc.set_option_decription(CDOD("color-for-text", CDOD::DATATYPE_STRING, "color for text", ""));
		doc.set_option_decription(CDOD("color-for-outline", CDOD::DATATYPE_STRING, "color for text outline", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_figures_availability();

		if(mode!="atom" && mode!="residue")
		{
			throw std::runtime_error(std::string("Invalid mode, must be 'atom' or 'residue'"));
		}

		if(figure_name_start.empty())
		{
			throw std::runtime_error(std::string("No figure name start provided."));
		}

		if(color_for_text.empty() && color_for_outline.empty())
		{
			throw std::runtime_error(std::string("No colors provided."));
		}

		if(!color_for_text.empty() && color_for_text!="_of_atom" && !auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_name(color_for_text)))
		{
			throw std::runtime_error(std::string("Invalid text color."));
		}

		if(!color_for_outline.empty() && color_for_outline!="_of_atom" && !auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_name(color_for_outline)))
		{
			throw std::runtime_error(std::string("Invalid text outline color."));
		}

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const std::map< std::string, std::vector<std::size_t> > map_of_ids=data_manager.generate_ids_for_of_labels(atom_ids, true, (mode=="atom"));

		bool something_done=false;

		for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=map_of_ids.begin();it!=map_of_ids.end();++it)
		{
			const std::vector<std::size_t>& atom_ids=it->second;
			if(!atom_ids.empty())
			{
				if(!color_for_text.empty())
				{
					const std::set<std::size_t> figure_ids=LongName::match(data_manager.figures(), LongName (figure_name_start, it->first, "text"));
					if(!figure_ids.empty())
					{
						const unsigned int figure_color=(color_for_text=="_of_atom") ? data_manager.atoms_display_states()[atom_ids.front()].visuals.front().color : auxiliaries::ColorUtilities::color_from_name(color_for_text);
						data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_color(figure_color), figure_ids);
						something_done=true;
					}
				}

				if(!color_for_outline.empty())
				{
					const std::set<std::size_t> figure_ids=LongName::match(data_manager.figures(), LongName(figure_name_start, it->first, "outline"));
					if(!figure_ids.empty())
					{
						const unsigned int figure_color=(color_for_outline=="_of_atom") ? data_manager.atoms_display_states()[atom_ids.front()].visuals.front().color : auxiliaries::ColorUtilities::color_from_name(color_for_outline);
						data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_color(figure_color), figure_ids);
						something_done=true;
					}
				}
			}
		}

		if(!something_done)
		{
			throw std::runtime_error(std::string("No figures of labels selected."));
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_COLOR_FIGURES_OF_LABELS_H_ */
