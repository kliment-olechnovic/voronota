#ifndef SCRIPTING_OPERATORS_ADD_FIGURES_OF_LABELS_H_
#define SCRIPTING_OPERATORS_ADD_FIGURES_OF_LABELS_H_

#include "../operators_common.h"

#include "../figure_of_text.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class AddFiguresOfLabels : public OperatorBase<AddFiguresOfLabels>
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
	std::string text;
	std::vector<std::string> figure_name_start;
	std::string color_for_text;
	std::string color_for_outline;
	double scale;
	double depth_shift;
	bool centered;
	bool no_outline;

	AddFiguresOfLabels() : scale(1.0), depth_shift(3.0), centered(false), no_outline(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		mode=input.get_value_or_default<std::string>("mode", "residue");
		text=input.get_value_or_default<std::string>("text", "${chain} ${rnum}${icode} ${rname} ${aname}");
		scale=input.get_value_or_default<double>("scale", 1.0);
		depth_shift=input.get_value_or_default<double>("depth-shift", 3.0);
		centered=input.get_flag("centered");
		no_outline=input.get_flag("no-outline");
		color_for_text=input.get_value_or_default<std::string>("color-for-text", "_of_atom");
		color_for_outline=input.get_value_or_default<std::string>("color-for-outline", "black");

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
		doc.set_option_decription(CDOD("text", CDOD::DATATYPE_STRING, "text template to draw", "${chain} ${rnum}${icode} ${rname} ${aname}"));
		doc.set_option_decription(CDOD("scale", CDOD::DATATYPE_FLOAT, "scaling factor", 1.0));
		doc.set_option_decription(CDOD("depth-shift", CDOD::DATATYPE_FLOAT, "depth shift", 3.0));
		doc.set_option_decription(CDOD("centered", CDOD::DATATYPE_BOOL, "flag to center the text"));
		doc.set_option_decription(CDOD("no-outline", CDOD::DATATYPE_BOOL, "flag to not add text outline"));
		doc.set_option_decription(CDOD("color-for-text", CDOD::DATATYPE_STRING, "color for text", "_of_atom, to take color of atoms"));
		doc.set_option_decription(CDOD("color-for-outline", CDOD::DATATYPE_STRING, "color for text outline", "black"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(mode!="atom" && mode!="residue")
		{
			throw std::runtime_error(std::string("Invalid mode, must be 'atom' or 'residue'"));
		}

		if(text.empty())
		{
			throw std::runtime_error(std::string("No text provided."));
		}

		if(figure_name_start.empty())
		{
			throw std::runtime_error(std::string("No figure name start provided."));
		}

		if(color_for_text!="_of_atom" && !auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_name(color_for_text)))
		{
			throw std::runtime_error(std::string("Invalid text color."));
		}

		if(color_for_outline!="_of_atom" && !auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_name(color_for_outline)))
		{
			throw std::runtime_error(std::string("Invalid text outline color."));
		}

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const std::map< std::string, std::vector<std::size_t> > map_of_ids=data_manager.generate_ids_for_of_labels(atom_ids, true, (mode=="atom"));

		for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=map_of_ids.begin();it!=map_of_ids.end();++it)
		{
			const std::vector<std::size_t>& atom_ids=it->second;
			if(!atom_ids.empty())
			{
				LongName figure_name(figure_name_start, it->first, "text");
				LongName figure_name_for_outline(figure_name_start, it->first, "outline");

				{
					const std::set<std::size_t> figure_ids=LongName::match(data_manager.figures(), figure_name);
					if(!figure_ids.empty())
					{
						data_manager.remove_figures(figure_ids);
					}
				}

				{
					const std::set<std::size_t> figure_ids=LongName::match(data_manager.figures(), figure_name_for_outline);
					if(!figure_ids.empty())
					{
						data_manager.remove_figures(figure_ids);
					}
				}

				std::vector<float> origin;
				{
					apollota::SimplePoint center;
					for(std::size_t i=0;i<atom_ids.size();i++)
					{
						center=center+apollota::SimplePoint(data_manager.atoms()[atom_ids[i]].value);
					}
					center=center*(1.0/static_cast<double>(atom_ids.size()));
					origin.push_back(static_cast<float>(center.x));
					origin.push_back(static_cast<float>(center.y));
					origin.push_back(static_cast<float>(center.z));
				}

				std::string text_to_use=text;
				{
					const Atom& atom=data_manager.atoms()[atom_ids.front()];
					OperatorsUtilities::replace_all_marks_in_string(text_to_use, "chain", atom.crad.chainID);
					OperatorsUtilities::replace_all_marks_in_string(text_to_use, "rnum", atom.crad.resSeq);
					OperatorsUtilities::replace_all_marks_in_string(text_to_use, "icode", atom.crad.iCode);
					OperatorsUtilities::replace_all_marks_in_string(text_to_use, "rname", atom.crad.resName);
					{
						const std::string rnameshort=auxiliaries::ResidueLettersCoding::convert_residue_code_big_to_small(atom.crad.resName);
						OperatorsUtilities::replace_all_marks_in_string(text_to_use, "rnameshort", (rnameshort!="X" ? rnameshort : atom.crad.resName));
					}
					OperatorsUtilities::replace_all_marks_in_string(text_to_use, "aname", (mode=="atom") ? atom.crad.name : std::string());
				}

				{
					Figure figure;
					if(FigureOfText::init_figure_of_text(text_to_use, false, origin, static_cast<float>(scale), centered, figure))
					{
						figure.name=figure_name;
						figure.z_shift=depth_shift;
						data_manager.add_figure(figure);

						const std::set<std::size_t> figure_ids=LongName::match(data_manager.figures(), figure_name);
						if(!figure_ids.empty())
						{
							const unsigned int figure_color=(color_for_text=="_of_atom") ? data_manager.atoms_display_states()[atom_ids.front()].visuals.front().color : auxiliaries::ColorUtilities::color_from_name(color_for_text);
							data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_color(figure_color), figure_ids);
							data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_show(true), figure_ids);
						}
					}
				}

				if(!no_outline)
				{
					Figure figure;
					if(FigureOfText::init_figure_of_text(text_to_use, true, origin, static_cast<float>(scale), centered, figure))
					{
						figure.name=figure_name_for_outline;
						figure.z_shift=depth_shift;
						data_manager.add_figure(figure);

						const std::set<std::size_t> figure_ids=LongName::match(data_manager.figures(), figure_name_for_outline);
						if(!figure_ids.empty())
						{
							const unsigned int figure_color=(color_for_outline=="_of_atom") ? data_manager.atoms_display_states()[atom_ids.front()].visuals.front().color : auxiliaries::ColorUtilities::color_from_name(color_for_outline);
							data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_color(figure_color), figure_ids);
							data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_show(true), figure_ids);
						}
					}
				}
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURES_OF_LABELS_H_ */
