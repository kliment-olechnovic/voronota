#ifndef VIEWER_OPERATORS_PERSPECTIVE_H_
#define VIEWER_OPERATORS_PERSPECTIVE_H_

#include "common.h"

namespace viewer
{

namespace operators
{

class Perspective
{
public:
	struct Result
	{
		const Result& write(scripting::HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	float field_of_view;
	float near_z;
	float far_z;

	explicit Perspective(uv::ViewerApplication& app) : field_of_view(app.perspective_field_of_view()), near_z(app.perspective_near_z()), far_z(app.perspective_far_z()), app_ptr_(&app)
	{
	}

	Perspective& init(scripting::CommandInput& input)
	{
		field_of_view=input.get_value_or_default<float>("field-of-view", app_ptr_->perspective_field_of_view());
		near_z=input.get_value_or_default<float>("near-z", app_ptr_->perspective_near_z());
		far_z=input.get_value_or_default<float>("far-z", app_ptr_->perspective_far_z());
		return (*this);
	}

	Result run() const
	{
		app_ptr_->set_projection_mode_to_perspective(field_of_view, near_z, far_z);
		Result result;
		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_PERSPECTIVE_H_ */
