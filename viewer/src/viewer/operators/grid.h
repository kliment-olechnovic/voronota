#ifndef VIEWER_OPERATORS_GRID_H_
#define VIEWER_OPERATORS_GRID_H_

#include "common.h"

namespace viewer
{

namespace operators
{

template<int grid_variant_value>
class GridTemplate
{
public:
	struct Result
	{
		const Result& write(scripting::HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	GridTemplate(uv::ViewerApplication& app, int& grid_variant) : app_ptr_(&app), grid_variant_ptr_(&grid_variant)
	{
	}

	GridTemplate& init(scripting::CommandInput& input)
	{
		return (*this);
	}

	Result run() const
	{
		app_ptr_->set_rendering_mode_to_grid();
		(*grid_variant_ptr_)=grid_variant_value;
		Result result;
		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
	int* grid_variant_ptr_;
};

typedef GridTemplate<0> GridByObject;
typedef GridTemplate<1> GridByConcept;

}

}

#endif /* VIEWER_OPERATORS_GRID_H_ */
