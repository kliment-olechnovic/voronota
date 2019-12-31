#ifndef VIEWER_OPERATORS_SETUP_RENDERING_H_
#define VIEWER_OPERATORS_SETUP_RENDERING_H_

#include "common.h"
#include "../drawer_for_data_manager.h"

namespace viewer
{

namespace operators
{

class SetupRendering
{
public:
	struct Result
	{
		const Result& write(scripting::HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	DrawerForDataManager::RenderingParameters rendering_parameters;
	bool reset;

	SetupRendering() : rendering_parameters(DrawerForDataManager::RenderingParameters::default_rendering_parameters()), reset(false)
	{
	}

	SetupRendering& init(scripting::CommandInput& input)
	{
		rendering_parameters=DrawerForDataManager::RenderingParameters::default_rendering_parameters();
		reset=input.get_flag("reset");
		if(reset)
		{
			rendering_parameters=DrawerForDataManager::RenderingParameters();
		}
		rendering_parameters.ball_sphere_quality=input.get_value_or_default<unsigned int>("ball-sphere-quality", rendering_parameters.ball_sphere_quality);
		rendering_parameters.stick_sphere_quality=input.get_value_or_default<unsigned int>("stick-sphere-quality", rendering_parameters.stick_sphere_quality);
		rendering_parameters.stick_cylinder_quality=input.get_value_or_default<unsigned int>("stick-cylinder-quality", rendering_parameters.stick_cylinder_quality);
		rendering_parameters.trace_sphere_quality=input.get_value_or_default<unsigned int>("trace-sphere-quality", rendering_parameters.trace_sphere_quality);
		rendering_parameters.trace_cylinder_quality=input.get_value_or_default<unsigned int>("trace-cylinder-quality", rendering_parameters.trace_cylinder_quality);
		rendering_parameters.cartoon_style=input.get_value_or_default<int>("cartoon-style", rendering_parameters.cartoon_style);
		return (*this);
	}

	Result run() const
	{
		if(rendering_parameters.ball_sphere_quality>4)
		{
			throw std::runtime_error(std::string("Too high 'ball-sphere-quality'."));
		}

		if(rendering_parameters.stick_sphere_quality>4)
		{
			throw std::runtime_error(std::string("Too high 'stick-sphere-quality'."));
		}

		if(rendering_parameters.stick_cylinder_quality>360)
		{
			throw std::runtime_error(std::string("Too high 'stick-cylinder-quality'."));
		}

		if(rendering_parameters.trace_sphere_quality>4)
		{
			throw std::runtime_error(std::string("Too high 'trace-sphere-quality'."));
		}

		if(rendering_parameters.trace_cylinder_quality>360)
		{
			throw std::runtime_error(std::string("Too high 'trace-cylinder-quality'."));
		}

		if(rendering_parameters.cartoon_style<0 || rendering_parameters.cartoon_style>2)
		{
			throw std::runtime_error(std::string("Invalid cartoon style, must be in {0, 1, 2}."));
		}

		DrawerForDataManager::RenderingParameters::default_rendering_parameters()=rendering_parameters;

		Result result;

		return result;
	}
};

}

}

#endif /* VIEWER_OPERATORS_SETUP_RENDERING_H_ */
