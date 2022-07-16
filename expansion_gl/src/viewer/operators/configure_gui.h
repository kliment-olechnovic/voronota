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

class ConfigureGUI : public scripting::OperatorBase<ConfigureGUI>
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
		ACTION_ENABLE_WAITING_INDICATOR,
		ACTION_SET_JSON_WRITING_LEVEL,
		ACTION_DISABLE_SEQUENCE_VIEW,
		ACTION_ENABLE_SEQUENCE_VIEW
	};

	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit ConfigureGUI(const Action action) : action_(action), value_of_json_writing_level_(2)
	{
	}

	ConfigureGUI& set_value_of_json_writing_level(const int value_of_json_writing_level)
	{
		value_of_json_writing_level_=value_of_json_writing_level;
		return (*this);
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
		else if(action_==ACTION_SET_JSON_WRITING_LEVEL)
		{
			GUIConfiguration::instance().json_writing_level=value_of_json_writing_level_;
		}
		else if(action_==ACTION_DISABLE_SEQUENCE_VIEW)
		{
			GUIConfiguration::instance().enabled_sequence_view=false;
		}
		else if(action_==ACTION_ENABLE_SEQUENCE_VIEW)
		{
			GUIConfiguration::instance().enabled_sequence_view=true;
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
	int value_of_json_writing_level_;
};

}

}

}

#endif /* VIEWER_OPERATORS_CONFIGURE_GUI_H_ */
