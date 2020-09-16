#ifndef VIEWER_OPERATORS_PERSPECTIVE_H_
#define VIEWER_OPERATORS_PERSPECTIVE_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Perspective : public scripting::OperatorBase<Perspective>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	float field_of_view;
	float near_z;
	float far_z;

	Perspective() : field_of_view(uv::ViewerApplication::instance().perspective_field_of_view()), near_z(uv::ViewerApplication::instance().perspective_near_z()), far_z(uv::ViewerApplication::instance().perspective_far_z())
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		field_of_view=input.get_value_or_default<float>("field-of-view", uv::ViewerApplication::instance().perspective_field_of_view());
		near_z=input.get_value_or_default<float>("near-z", uv::ViewerApplication::instance().perspective_near_z());
		far_z=input.get_value_or_default<float>("far-z", uv::ViewerApplication::instance().perspective_far_z());
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("field-of-view", CDOD::DATATYPE_FLOAT, "camera field of view", ""));
		doc.set_option_decription(CDOD("near-z", CDOD::DATATYPE_FLOAT, "camera near Z coordinate", ""));
		doc.set_option_decription(CDOD("far-z", CDOD::DATATYPE_FLOAT, "camera far Z coordinate", ""));
	}

	Result run(void*) const
	{
		uv::ViewerApplication::instance().set_projection_mode_to_perspective(field_of_view, near_z, far_z);
		uv::ViewerApplication::instance_refresh_frame();
		Result result;
		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_PERSPECTIVE_H_ */
