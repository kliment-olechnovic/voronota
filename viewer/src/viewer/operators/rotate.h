#ifndef VIEWER_OPERATORS_ROTATE_H_
#define VIEWER_OPERATORS_ROTATE_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Rotate : public scripting::operators::OperatorBase<Rotate>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	double angle_in_degrees;
	std::vector<double> axis;

	Rotate() : angle_in_degrees(10.0f), axis(3, 0.0)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		std::vector<double> default_axis(3, 0.0);
		default_axis[1]=1.0;
		angle_in_degrees=input.get_value_or_default<double>("angle", 10.0f);
		axis=input.get_value_vector_or_default<double>("axis", default_axis);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("angle", scripting::CDOD::DATATYPE_FLOAT, "rotation angle in degrees", 10.0));
		doc.set_option_decription(scripting::CDOD("axis", scripting::CDOD::DATATYPE_FLOAT_ARRAY, "rotation axis", "0 1 0"));
	}

	Result run(void*&) const
	{
		if(angle_in_degrees>360.0 || angle_in_degrees<(-360.0))
		{
			throw std::runtime_error(std::string("Invalid angle."));
		}

		if(axis.size()!=3 || (axis[0]==0.0 && axis[1]==0.0 && axis[2]==0.0))
		{
			throw std::runtime_error(std::string("Invalid axis."));
		}

		const double angle_in_radians=(angle_in_degrees*glm::pi<double>()/180.0);

		uv::ViewerApplication::instance().rotate(glm::vec3(axis[0], axis[1], axis[2]), angle_in_radians);

		uv::ViewerApplication::instance_refresh_frame();

		Result result;

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_ROTATE_H_ */
