#ifndef VIEWER_OPERATORS_ANTIALIASING_H_
#define VIEWER_OPERATORS_ANTIALIASING_H_

#include "../gui_configuration.h"
#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Antialiasing : public scripting::OperatorBase<Antialiasing>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit Antialiasing(const GUIConfiguration::AntialiasingVariant antialiasing_variant_value) : antialiasing_variant_value_(antialiasing_variant_value)
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
		if(antialiasing_variant_value_==GUIConfiguration::ANTIALIASING_VARIANT_FAST)
		{
			uv::ViewerApplication::instance().set_antialiasing_mode_to_fast();
		}
		else
		{
			uv::ViewerApplication::instance().set_antialiasing_mode_to_none();
		}

		GUIConfiguration::instance().antialiasing_variant=antialiasing_variant_value_;

		Result result;
		return result;
	}

private:
	GUIConfiguration::AntialiasingVariant antialiasing_variant_value_;
};

}

}

}

#endif /* VIEWER_OPERATORS_ANTIALIASING_H_ */
