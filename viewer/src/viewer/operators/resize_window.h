#ifndef VIEWER_OPERATORS_RESIZE_WINDOW_H_
#define VIEWER_OPERATORS_RESIZE_WINDOW_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class ResizeWindow : public scripting::operators::OperatorBase<ResizeWindow>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	int width;
	int height;

	ResizeWindow() : width(uv::ViewerApplication::instance().window_width()), height(uv::ViewerApplication::instance().window_height())
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		width=input.get_value_or_default<int>("width", uv::ViewerApplication::instance().window_width());
		height=input.get_value_or_default<int>("height", uv::ViewerApplication::instance().window_height());
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("width", CDOD::DATATYPE_INT, "window width", ""));
		doc.set_option_decription(CDOD("height", CDOD::DATATYPE_INT, "window height", ""));
	}

	Result run(void*) const
	{
		if(width<1 || width>9999)
		{
			throw std::runtime_error(std::string("Invalid width."));
		}

		if(height<1 || height>9999)
		{
			throw std::runtime_error(std::string("Invalid height."));
		}

		uv::ViewerApplication::instance().set_window_size(width, height);

		uv::ViewerApplication::instance_refresh_frame();

		Result result;

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_RESIZE_WINDOW_H_ */
