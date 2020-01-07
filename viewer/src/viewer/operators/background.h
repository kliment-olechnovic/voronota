#ifndef VIEWER_OPERATORS_BACKGROUND_H_
#define VIEWER_OPERATORS_BACKGROUND_H_

#include "../operators_common.h"

namespace viewer
{

namespace operators
{

class Background : public scripting::operators::OperatorBase<Background>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		const Result& write(scripting::HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	auxiliaries::ColorUtilities::ColorInteger color;

	explicit Background(uv::ViewerApplication& app) : color(auxiliaries::ColorUtilities::null_color()), app_ptr_(&app)
	{
	}

	Background& init(scripting::CommandInput& input)
	{
		color=auxiliaries::ColorUtilities::color_from_name(input.get_value_or_first_unused_unnamed_value("col"));
		return (*this);
	}

	Result run(void*&) const
	{
		if(!auxiliaries::ColorUtilities::color_valid(color))
		{
			throw std::runtime_error(std::string("Invalid color."));
		}

		app_ptr_->set_background_color(color);

		Result result;

		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_BACKGROUND_H_ */
