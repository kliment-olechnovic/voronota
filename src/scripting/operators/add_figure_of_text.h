#ifndef SCRIPTING_OPERATORS_ADD_FIGURE_OF_TEXT_H_
#define SCRIPTING_OPERATORS_ADD_FIGURE_OF_TEXT_H_

#include "../operators_common.h"

#include "../figure_of_text.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class AddFigureOfText : public OperatorBase<AddFigureOfText>
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
	std::vector<std::string> figure_name;
	std::string text;
	std::vector<double> fixed_pos;
	std::vector<double> offset_pos;
	double scale;
	double depth_shift;
	bool centered;
	bool as_outline;

	AddFigureOfText() : scale(1.0), depth_shift(3.0), centered(false), as_outline(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		figure_name=input.get_value_vector<std::string>("figure-name");
		text=input.get_value<std::string>("text");
		fixed_pos=input.get_value_vector_or_default<double>("fixed-pos", std::vector<double>());
		offset_pos=input.get_value_vector_or_default<double>("offset-pos", std::vector<double>());
		scale=input.get_value_or_default<double>("scale", 1.0);
		depth_shift=input.get_value_or_default<double>("z-shift", 3.0);
		centered=input.get_flag("centered");
		as_outline=input.get_flag("as-outline");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("figure-name", CDOD::DATATYPE_STRING_ARRAY, "figure name"));
		doc.set_option_decription(CDOD("text", CDOD::DATATYPE_STRING, "text to draw"));
		doc.set_option_decription(CDOD("fixed-pos", CDOD::DATATYPE_FLOAT_ARRAY, "fixed position, overrides atoms selection"));
		doc.set_option_decription(CDOD("offset-pos", CDOD::DATATYPE_FLOAT_ARRAY, "offset from computed or provided position"));
		doc.set_option_decription(CDOD("scale", CDOD::DATATYPE_FLOAT, "scaling factor", 1.0));
		doc.set_option_decription(CDOD("depth-shift", CDOD::DATATYPE_FLOAT, "depth shift", 3.0));
		doc.set_option_decription(CDOD("centered", CDOD::DATATYPE_BOOL, "flag to center the text"));
		doc.set_option_decription(CDOD("as-outline", CDOD::DATATYPE_BOOL, "flag to generate text graphics as outline"));
	}

	Result run(DataManager& data_manager) const
	{
		if(text.empty())
		{
			throw std::runtime_error(std::string("No text provided."));
		}

		apollota::SimplePoint center;

		if(fixed_pos.empty())
		{
			data_manager.assert_atoms_availability();

			const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

			if(atom_ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
			{
				center=center+apollota::SimplePoint(data_manager.atoms()[*it].value);
			}
			center=center*(1.0/static_cast<double>(atom_ids.size()));
		}
		else
		{
			if(fixed_pos.size()!=3)
			{
				throw std::runtime_error(std::string("Invalid fixed position."));
			}

			center.x=fixed_pos[0];
			center.y=fixed_pos[1];
			center.z=fixed_pos[2];
		}

		if(!offset_pos.empty())
		{
			if(offset_pos.size()!=3 && offset_pos.size()!=2)
			{
				throw std::runtime_error(std::string("Invalid offset position."));
			}

			center.x+=offset_pos[0];
			center.y+=offset_pos[1];
			center.z+=(offset_pos.size()>2 ? offset_pos[2] : 0.0);
		}

		std::vector<float> origin;
		origin.push_back(static_cast<float>(center.x));
		origin.push_back(static_cast<float>(center.y));
		origin.push_back(static_cast<float>(center.z));

		Figure figure;

		if(!FigureOfText::init_figure_of_text(text, as_outline, origin, static_cast<float>(scale), centered, figure))
		{
			throw std::runtime_error(std::string("Failed to generate text graphics."));
		}

		figure.name=LongName(figure_name);
		figure.z_shift=depth_shift;

		data_manager.add_figure(figure);

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURE_OF_TEXT_H_ */

