#ifndef VIEWER_OPERATORS_HINT_RENDER_AREA_SIZE_H_
#define VIEWER_OPERATORS_HINT_RENDER_AREA_SIZE_H_

#include "../gui_configuration.h"
#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class HintRenderAreaSize : public scripting::OperatorBase<HintRenderAreaSize>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	int width;
	int height;

	HintRenderAreaSize() : width(-1), height(-1)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		width=input.get_value_or_default<int>("width", -1);
		height=input.get_value_or_default<int>("height", -1);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("width", CDOD::DATATYPE_INT, "render area width hint", ""));
		doc.set_option_decription(CDOD("height", CDOD::DATATYPE_INT, "render area height hint", ""));
	}

	Result run(void*) const
	{
		GUIConfiguration::instance().hint_render_area_width=width;
		GUIConfiguration::instance().hint_render_area_height=height;

		Result result;

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_HINT_RENDER_AREA_SIZE_H_ */
