#ifndef VIEWER_OPERATORS_ORTHO_H_
#define VIEWER_OPERATORS_ORTHO_H_

#include "../operators_common.h"

namespace viewer
{

namespace operators
{

class Ortho : public scripting::operators::OperatorBase<Ortho>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit Ortho(uv::ViewerApplication& app) : app_ptr_(&app)
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
		app_ptr_->set_projection_mode_to_ortho();
		Result result;
		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_ORTHO_H_ */
