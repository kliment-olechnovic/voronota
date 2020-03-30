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
		ACTION_ENABLE_WIDGETS,
		ACTION_DISABLE_WAITING_INDICATOR,
		ACTION_ENABLE_WAITING_INDICATOR
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

	Result run(void*) const
	{
		bool need_refresh=true;

		if(action_==ACTION_PUSH)
		{
			need_refresh=false;
			GUIConfiguration::push();
		}
		else if(action_==ACTION_POP)
		{
			GUIConfiguration::pop();
		}
		else if(action_==ACTION_DISABLE_WIDGETS)
		{
			GUIConfiguration::instance().set_enabled_widgets(false);
		}
		else if(action_==ACTION_ENABLE_WIDGETS)
		{
			GUIConfiguration::instance().set_enabled_widgets(true);
		}
		else if(action_==ACTION_DISABLE_WAITING_INDICATOR)
		{
			GUIConfiguration::instance().enabled_waiting_indicator=false;
		}
		else if(action_==ACTION_ENABLE_WAITING_INDICATOR)
		{
			GUIConfiguration::instance().enabled_waiting_indicator=true;
		}

		if(need_refresh)
		{
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
