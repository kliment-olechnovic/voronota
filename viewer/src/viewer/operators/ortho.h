#ifndef VIEWER_OPERATORS_ORTHO_H_
#define VIEWER_OPERATORS_ORTHO_H_

#include "../operators_common.h"

namespace voronota
{

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

	Ortho()
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
		uv::ViewerApplication::instance().set_projection_mode_to_ortho();
		uv::ViewerApplication::instance_refresh_frame();
		Result result;
		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_ORTHO_H_ */
