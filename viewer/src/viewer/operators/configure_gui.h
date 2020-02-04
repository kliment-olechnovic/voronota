#ifndef VIEWER_OPERATORS_CONFIGURE_GUI_H_
#define VIEWER_OPERATORS_CONFIGURE_GUI_H_

#include "../gui_configuration.h"
#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class ConfigureGUI : public scripting::operators::OperatorBase<ConfigureGUI>
{
public:
	enum Action
	{
		ACTION_UNDEFINED,
		ACTION_PUSH,
		ACTION_POP,
		ACTION_DISABLE_WIDGETS,
		ACTION_ENABLE_WIDGETS
	};

	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit ConfigureGUI(const Action action) : action_(action)
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
		if(action_==ACTION_PUSH)
		{
			GUIConfiguration::push();
		}
		else if(action_==ACTION_POP)
		{
			GUIConfiguration::pop();
			uv::ViewerApplication::instance_refresh_frame();
		}
		else if(action_==ACTION_DISABLE_WIDGETS)
		{
			GUIConfiguration::instance().set_enabled_widgets(false);
			uv::ViewerApplication::instance_refresh_frame();
		}
		else if(action_==ACTION_ENABLE_WIDGETS)
		{
			GUIConfiguration::instance().set_enabled_widgets(true);
			uv::ViewerApplication::instance_refresh_frame();
		}

		Result result;
		return result;
	}

private:
	Action action_;
};

}

}

}

#endif /* VIEWER_OPERATORS_CONFIGURE_GUI_H_ */
