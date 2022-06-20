#ifndef DUKTAPER_OPERATORS_MUSIC_BACKGROUND_H_
#define DUKTAPER_OPERATORS_MUSIC_BACKGROUND_H_

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class MusicBackground : public scripting::OperatorBase<MusicBackground>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	std::string action;

	MusicBackground()
	{
	}

	MusicBackground(const std::string& action) : action(action)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		action=input.get_value_or_first_unused_unnamed_value("action");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("action", CDOD::DATATYPE_STRING, "melody name to play, 'stop', 'disable', or 'enable'"));
	}

	Result run(void*) const
	{
		if(action=="disable")
		{
			stop();
			set_enabled(false);
		}
		else if(action=="enable")
		{
			set_enabled(true);
		}
		else if(action=="stop")
		{
			stop();
		}
		else
		{
			if(is_enabled())
			{
				play(action);
			}
		}

		Result result;
		return result;
	}

	static void set_enabled(const bool enabled)
	{
		is_enabled()=enabled;
	}

	static void stop_if_was_used()
	{
		if(was_used())
		{
			stop();
		}
	}

private:
	static bool& was_used()
	{
		static bool usage_status=false;
		return usage_status;
	}

	static bool& is_enabled()
	{
		static bool enabling_status=false;
		return enabling_status;
	}

	static void play(const std::string& melody)
	{
		if(CallShellUtilities::test_if_shell_command_available("background-music-for-voronota"))
		{
			std::ostringstream command_output;
			command_output << "background-music-for-voronota " << melody;
			operators::CallShell::Result call_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(call_result.exit_status==0 && melody!="stop")
			{
				was_used()=true;
			}
		}
	}

	static void stop()
	{
		play("stop");
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_MUSIC_BACKGROUND_H_ */
