#ifndef VIEWER_OPERATORS_MONO_H_
#define VIEWER_OPERATORS_MONO_H_

#include "common.h"

namespace viewer
{

namespace operators
{

class Mono
{
public:
	struct Result
	{
		const Result& write(scripting::HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	explicit Mono(uv::ViewerApplication& app) : app_ptr_(&app)
	{
	}

	Mono& init(scripting::CommandInput&)
	{
		return (*this);
	}

	Result run() const
	{
		app_ptr_->set_rendering_mode_to_simple();
		Result result;
		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_MONO_H_ */
