#ifndef VIEWER_OPERATORS_FOG_H_
#define VIEWER_OPERATORS_FOG_H_

#include "../operators_common.h"

namespace viewer
{

namespace operators
{

class Fog
{
public:
	struct Result
	{
		const Result& write(scripting::HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	bool off;

	explicit Fog(uv::ViewerApplication& app) : off(false), app_ptr_(&app)
	{
	}

	Fog& init(scripting::CommandInput& input)
	{
		off=input.get_flag("off");
		return (*this);
	}

	Result run(void*&) const
	{
		app_ptr_->set_fog_enabled(!off);
		Result result;
		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_FOG_H_ */
