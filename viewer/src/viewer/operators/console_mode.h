#ifndef VIEWER_OPERATORS_CONSOLE_MODE_H_
#define VIEWER_OPERATORS_CONSOLE_MODE_H_

#include "../operators_common.h"
#include "../runtime_parameters.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class ConsoleMode : public scripting::operators::OperatorBase<ConsoleMode>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit ConsoleMode(const RuntimeParameters::ConsoleModeVariant console_mode_variant_value) : console_mode_variant_value_(console_mode_variant_value)
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
		RuntimeParameters::instance().console_mode_variant=console_mode_variant_value_;
		Result result;
		return result;
	}

private:
	RuntimeParameters::ConsoleModeVariant console_mode_variant_value_;
};

}

}

}

#endif /* VIEWER_OPERATORS_CONSOLE_MODE_H_ */
