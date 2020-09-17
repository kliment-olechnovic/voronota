#ifndef UV_GENERIC_DRAWING_CONTROLLER_H_
#define UV_GENERIC_DRAWING_CONTROLLER_H_

#include "common.h"

namespace voronota
{

namespace uv
{

class GenericDrawingController
{
public:
	virtual ~GenericDrawingController()
	{
	}

	const std::string& get_name() const
	{
		return name_;
	}

	void set_name(const std::string& name)
	{
		name_=name;
	}

	virtual bool draw() = 0;

	virtual bool object_set_visible(unsigned int id, const bool visible) = 0;

	virtual bool object_set_enabled(unsigned int id, const bool enabled) = 0;

	virtual bool object_set_color(unsigned int id, unsigned int rgb) = 0;

	virtual bool object_set_adjunct(unsigned int id, const float a, const float b, const float c) = 0;

	virtual void objects_set_visible(const bool visible) = 0;

	virtual void objects_set_enabled(const bool enabled) = 0;

private:
	std::string name_;
};

}

}

#endif /* UV_GENERIC_DRAWING_CONTROLLER_H_ */
