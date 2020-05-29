#ifndef VIEWER_OPERATORS_STEREO_H_
#define VIEWER_OPERATORS_STEREO_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Stereo : public scripting::operators::OperatorBase<Stereo>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		float angle;
		float offset;

		Result() : angle(0.0), offset(0.0)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("angle")=angle;
			heterostorage.variant_object.value("offset")=offset;
		}
	};

	float angle;
	float offset;

	Stereo() : angle(uv::ViewerApplication::instance().stereo_angle()), offset(uv::ViewerApplication::instance().stereo_offset())
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		angle=input.get_value_or_default<float>("angle", uv::ViewerApplication::instance().stereo_angle());
		offset=input.get_value_or_default<float>("offset", uv::ViewerApplication::instance().stereo_offset());
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("angle", CDOD::DATATYPE_FLOAT, "stereo angle", ""));
		doc.set_option_decription(CDOD("offset", CDOD::DATATYPE_FLOAT, "stereo offset", ""));
	}

	Result run(void*) const
	{
		uv::ViewerApplication::instance().set_rendering_mode_to_stereo(angle, offset);

		uv::ViewerApplication::instance_refresh_frame();

		Result result;
		result.angle=angle;
		result.offset=offset;

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_STEREO_H_ */
