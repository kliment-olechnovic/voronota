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

	std::string melody;

	MusicBackground()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		melody=input.get_value<std::string>("melody");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("melody", CDOD::DATATYPE_STRING, "melody name to play, 'stop' to stop playing"));
	}

	Result run(void*) const
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

		Result result;
		return result;
	}

	static void stop()
	{
		if(was_used())
		{
			MusicBackground().init(CMDIN().set("melody", "stop")).run(0);
		}
	}

private:
	static bool& was_used()
	{
		static bool status=false;
		return status;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_MUSIC_BACKGROUND_H_ */
