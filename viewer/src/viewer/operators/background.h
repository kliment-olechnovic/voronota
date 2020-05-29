#ifndef VIEWER_OPERATORS_BACKGROUND_H_
#define VIEWER_OPERATORS_BACKGROUND_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Background : public scripting::operators::OperatorBase<Background>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	auxiliaries::ColorUtilities::ColorInteger color;

	Background() : color(auxiliaries::ColorUtilities::null_color())
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		color=auxiliaries::ColorUtilities::color_from_name(input.get_value_or_first_unused_unnamed_value("col"));
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("col", CDOD::DATATYPE_STRING, "color string"));
	}

	Result run(void*) const
	{
		if(!auxiliaries::ColorUtilities::color_valid(color))
		{
			throw std::runtime_error(std::string("Invalid color."));
		}

		uv::ViewerApplication::instance().set_background_color(color);

		uv::ViewerApplication::instance_refresh_frame();

		Result result;

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_BACKGROUND_H_ */
