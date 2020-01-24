#ifndef VIEWER_OPERATORS_GRID_H_
#define VIEWER_OPERATORS_GRID_H_

#include "../operators_common.h"
#include "../runtime_parameters.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Grid : public scripting::operators::OperatorBase<Grid>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit Grid(const RuntimeParameters::GridVariant grid_variant_value) : grid_variant_value_(grid_variant_value)
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
		uv::ViewerApplication::instance().set_rendering_mode_to_grid();
		RuntimeParameters::instance().grid_variant=grid_variant_value_;
		Result result;
		return result;
	}

private:
	RuntimeParameters::GridVariant grid_variant_value_;
};

}

}

}

#endif /* VIEWER_OPERATORS_GRID_H_ */
