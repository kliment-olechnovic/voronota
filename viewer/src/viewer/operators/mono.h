#ifndef VIEWER_OPERATORS_MONO_H_
#define VIEWER_OPERATORS_MONO_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Mono : public scripting::operators::OperatorBase<Mono>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit Mono(uv::ViewerApplication& app) : app_ptr_(&app)
	{
	}

	void initialize(scripting::CommandInput&)
	{
	}

	void document(scripting::CommandDocumentation&) const
	{
	}

	Result run(void*&) const
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

}

#endif /* VIEWER_OPERATORS_MONO_H_ */
