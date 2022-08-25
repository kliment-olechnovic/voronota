#ifndef VIEWER_OPERATORS_OCCLUSION_H_
#define VIEWER_OPERATORS_OCCLUSION_H_

#include "../gui_configuration.h"
#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Occlusion : public scripting::OperatorBase<Occlusion>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit Occlusion(const GUIConfiguration::OcclusionVariant occlusion_variant_value) : occlusion_variant_value_(occlusion_variant_value)
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
		if(occlusion_variant_value_==GUIConfiguration::OCCLUSION_VARIANT_NOISY)
		{
			uv::ViewerApplication::instance().set_occlusion_mode_to_noisy();
		}
		else if(occlusion_variant_value_==GUIConfiguration::OCCLUSION_VARIANT_SMOOTH)
		{
			uv::ViewerApplication::instance().set_occlusion_mode_to_smooth();
		}
		else
		{
			uv::ViewerApplication::instance().set_occlusion_mode_to_none();
		}

		GUIConfiguration::instance().occlusion_variant=occlusion_variant_value_;

		Result result;
		return result;
	}

private:
	GUIConfiguration::OcclusionVariant occlusion_variant_value_;
};

}

}

}

#endif /* VIEWER_OPERATORS_OCCLUSION_H_ */
