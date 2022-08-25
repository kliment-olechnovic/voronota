#ifndef VIEWER_OPERATORS_IMPOSTORING_H_
#define VIEWER_OPERATORS_IMPOSTORING_H_

#include "../gui_configuration.h"
#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Impostoring : public scripting::OperatorBase<Impostoring>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit Impostoring(const GUIConfiguration::ImpostoringVariant impostoring_variant_value) : impostoring_variant_value_(impostoring_variant_value)
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
		GUIConfiguration::instance().impostoring_variant=impostoring_variant_value_;

		Result result;
		return result;
	}

private:
	GUIConfiguration::ImpostoringVariant impostoring_variant_value_;
};

}

}

}

#endif /* VIEWER_OPERATORS_IMPOSTORING_H_ */
