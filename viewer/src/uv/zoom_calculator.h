#ifndef UV_ZOOM_CALCULATOR_H_
#define UV_ZOOM_CALCULATOR_H_

#include "common.h"

namespace uv
{

class ZoomCalculator
{
public:
	ZoomCalculator() :
		modified_(false),
		low_(0.0f, 0.0f, 0.0f),
		high_(0.0f, 0.0f, 0.0f)
	{
		const float max_float=std::numeric_limits<float>::max();
		const float min_float=-max_float;
		for(int i=0;i<3;i++)
		{
			low_[i]=max_float;
			high_[i]=min_float;
		}
	}

	void update(const glm::vec3& point)
	{
		for(int i=0;i<3;i++)
		{
			low_[i]=std::min(low_[i], point[i]);
			high_[i]=std::max(high_[i], point[i]);
		}
		modified_=true;
	}

	void update(const float x, const float y, const float z)
	{
		update(glm::vec3(x, y, z));
	}

	bool modified() const
	{
		return modified_;
	}

	glm::vec3 get_center_position() const
	{
		if(modified_)
		{
			return ((low_+high_)*0.5f);
		}
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	float get_bounding_radius() const
	{
		return glm::distance(low_, high_);
	}

	double get_zoom_factor() const
	{
		if(modified_)
		{
			return (2.0/static_cast<double>(get_bounding_radius()));
		}
		return 1.0f;
	}

private:
	bool modified_;
	glm::vec3 low_;
	glm::vec3 high_;
};

}

#endif /* UV_ZOOM_CALCULATOR_H_ */
