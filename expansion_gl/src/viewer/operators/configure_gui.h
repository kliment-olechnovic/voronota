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
		ACTION_ENABLE_SEQUENCE_VIEW,
		ACTION_TOGGLE_SEQUENCE_VIEW,
		ACTION_DISABLE_CONSOLE,
		ACTION_ENABLE_CONSOLE,
		ACTION_TOGGLE_CONSOLE,
		ACTION_SET_INITIAL_MAIN_REPRESENTATION
	};

	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	explicit ConfigureGUI(const Action action) :
			action_(action),
			value_of_json_writing_level_(2),
			value_of_initial_main_represenation_(GUIConfiguration::INITIAL_REPRESENTATION_VARIANT_CARTOON)
	{
	}

	ConfigureGUI& set_value_of_json_writing_level(const int value_of_json_writing_level)
	{
		value_of_json_writing_level_=value_of_json_writing_level;
		return (*this);
	}

	ConfigureGUI& set_value_of_initial_main_represenation(const GUIConfiguration::InitialRepresentationVariant value_of_initial_main_represenation)
	{
		value_of_initial_main_represenation_=value_of_initial_main_represenation;
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
		else if(action_==ACTION_TOGGLE_SEQUENCE_VIEW)
		{
			GUIConfiguration::instance().enabled_sequence_view=!GUIConfiguration::instance().enabled_sequence_view;
		}
		else if(action_==ACTION_DISABLE_CONSOLE)
		{
			GUIConfiguration::instance().enabled_console=false;
		}
		else if(action_==ACTION_ENABLE_CONSOLE)
		{
			GUIConfiguration::instance().enabled_console=true;
		}
		else if(action_==ACTION_TOGGLE_CONSOLE)
		{
			GUIConfiguration::instance().enabled_console=!GUIConfiguration::instance().enabled_console;
		}
		else if(action_==ACTION_SET_INITIAL_MAIN_REPRESENTATION)
		{
			GUIConfiguration::instance().initial_main_representation_variant=value_of_initial_main_represenation_;
		}

		if(need_refresh)
		{
			uv::ViewerApplication::instance_refresh_frame(false);
		}

		Result result;
		return result;
	}

private:
	Action action_;
	int value_of_json_writing_level_;
	GUIConfiguration::InitialRepresentationVariant value_of_initial_main_represenation_;
};

}

}

}

#endif /* VIEWER_OPERATORS_CONFIGURE_GUI_H_ */
