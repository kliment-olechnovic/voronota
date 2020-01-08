#ifndef VIEWER_OPERATORS_FOG_H_
#define VIEWER_OPERATORS_FOG_H_

#include "../operators_common.h"

namespace viewer
{

namespace operators
{

class Fog : public scripting::operators::OperatorBase<Fog>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		const Result& write(scripting::HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	bool off;

	explicit Fog(uv::ViewerApplication& app) : off(false), app_ptr_(&app)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		off=input.get_flag("off");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("off", scripting::CDOD::DATATYPE_BOOL, "flag to turn fog off"));
	}

	Result run(void*&) const
	{
		app_ptr_->set_fog_enabled(!off);
		Result result;
		return result;
	}

private:
	uv::ViewerApplication* app_ptr_;
};

}

}

#endif /* VIEWER_OPERATORS_FOG_H_ */
