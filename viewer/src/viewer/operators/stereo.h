#ifndef VIEWER_OPERATORS_STEREO_H_
#define VIEWER_OPERATORS_STEREO_H_

#include "common.h"

namespace viewer
{

namespace operators
{

class Stereo
{
public:
	struct Result
	{
		float angle;
		float offset;

		const Result& write(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("angle")=angle;
			heterostorage.variant_object.value("offset")=offset;
			return (*this);
		}
	};

	float angle;
	float offset;

	explicit Stereo(uv::ViewerApplication& app) : angle(app.stereo_angle()), offset(app.stereo_offset()), app_ptr_(&app)
	{
	}

	Stereo& init(scripting::CommandInput& input)
	{
		angle=input.get_value_or_default<float>("angle", app_ptr_->stereo_angle());
		offset=input.get_value_or_default<float>("offset", app_ptr_->stereo_offset());
		return (*this);
	}

	Result run() const
	{
		app_ptr_->set_rendering_mode_to_stereo(angle, offset);

		Result result;
		result.angle=angle;
		result.offset=offset;

		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_STEREO_H_ */
