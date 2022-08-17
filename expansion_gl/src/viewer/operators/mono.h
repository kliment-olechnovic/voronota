#ifndef VIEWER_OPERATORS_MONO_H_
#define VIEWER_OPERATORS_MONO_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Mono : public scripting::OperatorBase<Mono>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	Mono()
	{
	}

	void initialize(scripting::CommandInput&)
	{
	}

	void document(scripting::CommandDocumentation&) const
	{
	}

	Result run(void*) const
	{
		uv::ViewerApplication::instance().set_rendering_mode_to_simple();
		uv::ViewerApplication::instance_refresh_frame(false);
		Result result;
		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_MONO_H_ */
