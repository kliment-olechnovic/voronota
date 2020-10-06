#ifndef VIEWER_OPERATORS_ANIMATE_H_
#define VIEWER_OPERATORS_ANIMATE_H_

#include "../gui_configuration.h"
#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Animate : public scripting::OperatorBase<Animate>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	double time_step;

	explicit Animate(const GUIConfiguration::AnimationVariant animation_variant_value) : time_step(25.0), animation_variant_value_(animation_variant_value)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		time_step=input.get_value_or_default<double>("time-step", 25.0);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("time-step", CDOD::DATATYPE_FLOAT, "animation time step", 25.0));
	}

	Result run(void*) const
	{
		if(time_step<0.0)
		{
			throw std::runtime_error(std::string("Invalid time step."));
		}

		GUIConfiguration::instance().animation_step_miliseconds=time_step;

		GUIConfiguration::instance().animation_variant=animation_variant_value_;

		Result result;
		return result;
	}

private:
	GUIConfiguration::AnimationVariant animation_variant_value_;
};

}

}

}

#endif /* VIEWER_OPERATORS_ANIMATE_H_ */
