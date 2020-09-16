#ifndef VIEWER_OPERATORS_FOG_H_
#define VIEWER_OPERATORS_FOG_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Fog : public scripting::OperatorBase<Fog>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	bool off;

	Fog() : off(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		off=input.get_flag("off");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("off", CDOD::DATATYPE_BOOL, "flag to turn fog off"));
	}

	Result run(void*) const
	{
		uv::ViewerApplication::instance().set_fog_enabled(!off);
		uv::ViewerApplication::instance_refresh_frame();
		Result result;
		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_FOG_H_ */
