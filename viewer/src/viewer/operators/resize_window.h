#ifndef VIEWER_OPERATORS_RESIZE_WINDOW_H_
#define VIEWER_OPERATORS_RESIZE_WINDOW_H_

#include "../operators_common.h"

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

	explicit ResizeWindow(uv::ViewerApplication& app) : width(app.window_width()), height(app.window_height()), app_ptr_(&app)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		width=input.get_value_or_default<int>("width", app_ptr_->window_width());
		height=input.get_value_or_default<int>("height", app_ptr_->window_height());
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("width", scripting::CDOD::DATATYPE_INT, "window width", ""));
		doc.set_option_decription(scripting::CDOD("height", scripting::CDOD::DATATYPE_INT, "window height", ""));
	}

	Result run(void*&) const
	{
		if(width<1 || width>9999)
		{
			throw std::runtime_error(std::string("Invalid width."));
		}

		if(height<1 || height>9999)
		{
			throw std::runtime_error(std::string("Invalid height."));
		}

		app_ptr_->set_window_size(width, height);

		Result result;

		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_RESIZE_WINDOW_H_ */
