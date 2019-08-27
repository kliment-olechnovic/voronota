#ifndef UV_TRANSFORMATION_MATRIX_CONTROLLER_H_
#define UV_TRANSFORMATION_MATRIX_CONTROLLER_H_

#include "common.h"

namespace uv
{

class TransformationMatrixController
{
public:
	TransformationMatrixController() : matrix_(1.0f)
	{
	}

	TransformationMatrixController(const int width, const int height, const float z_near, const float z_far) : matrix_(1.0f)
	{
		float side=static_cast<float>(std::min(width, height));
		float width_range=static_cast<float>(width)/side;
		float height_range=static_cast<float>(height)/side;
		matrix_=glm::ortho(-width_range, +width_range, -height_range, +height_range, z_near, z_far);
	}

	const glm::mat4& matrix() const
	{
		return matrix_;
	}

	const GLfloat* matrix_data() const
	{
		return (&matrix_[0][0]);
	}

	void add_rotation(const float angle_in_radians, const glm::vec3& axis)
	{
		matrix_=glm::rotate(glm::mat4(1.0f), angle_in_radians, axis)*matrix_;
	}

	void add_translation(const glm::vec3& offset)
	{
		matrix_=glm::translate(glm::mat4(1.0f), offset)*matrix_;
	}

	void add_scaling(const float factor)
	{
		matrix_=glm::scale(glm::mat4(1.0f), glm::vec3(factor, factor, factor))*matrix_;
	}

	void add_scaling(const float factor_x, const float factor_y, const float factor_z)
	{
		matrix_=glm::scale(glm::mat4(1.0f), glm::vec3(factor_x, factor_y, factor_z))*matrix_;
	}

	void reset()
	{
		matrix_=glm::mat4(1.0f);
	}

	void center(const glm::vec3& pos)
	{
		add_translation(glm::vec3(matrix_*glm::vec4(pos, 1.0f))*(-1.0f));
	}

private:
	glm::mat4 matrix_;
};

}

#endif /* UV_TRANSFORMATION_MATRIX_CONTROLLER_H_ */
