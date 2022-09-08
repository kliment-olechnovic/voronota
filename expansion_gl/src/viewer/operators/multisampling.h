#ifndef VIEWER_OPERATORS_MULTISAMPLING_H_
#define VIEWER_OPERATORS_MULTISAMPLING_H_

#include "../gui_configuration.h"
#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Multisampling : public scripting::OperatorBase<Multisampling>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit Multisampling(const GUIConfiguration::MultisamplingVariant multisampling_variant_value) : multisampling_variant_value_(multisampling_variant_value)
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
		if(multisampling_variant_value_==GUIConfiguration::MULTISAMPLING_VARIANT_BASIC)
		{
			uv::ViewerApplication::instance().set_multisampling_mode_to_basic();
		}
		else
		{
			uv::ViewerApplication::instance().set_multisampling_mode_to_none();
		}

		GUIConfiguration::instance().multisampling_variant=multisampling_variant_value_;

		Result result;
		return result;
	}

private:
	GUIConfiguration::MultisamplingVariant multisampling_variant_value_;
};

}

}

}

#endif /* VIEWER_OPERATORS_MULTISAMPLING_H_ */

