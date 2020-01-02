#ifndef VIEWER_OPERATORS_ROTATE_H_
#define VIEWER_OPERATORS_ROTATE_H_

#include "common.h"

namespace viewer
{

namespace operators
{

class Rotate
{
public:
	struct Result
	{
		const Result& write(scripting::HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	double angle_in_degrees;
	std::vector<double> axis;

	explicit Rotate(uv::ViewerApplication& app) : angle_in_degrees(10.0f), axis(3, 0.0), app_ptr_(&app)
	{
	}

	Rotate& init(scripting::CommandInput& input)
	{
		std::vector<double> default_axis(3, 0.0);
		default_axis[1]=1.0;
		angle_in_degrees=input.get_value_or_default<double>("angle", 10.0f);
		axis=input.get_value_vector_or_default<double>("axis", default_axis);
		return (*this);
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

		app_ptr_->rotate(glm::vec3(axis[0], axis[1], axis[2]), angle_in_radians);

		Result result;

		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_ROTATE_H_ */
