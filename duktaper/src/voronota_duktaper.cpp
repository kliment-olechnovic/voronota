#include <iostream>

#include "duktaper/terminal_utilities.h"
#include "duktaper/duktape_wrapper.h"

int main(const int /*argc*/, const char** /*argv*/)
{
	voronota::scripting::ScriptExecutionManagerWithVariantOutput execution_manager;
	voronota::duktaper::DuktapeContextWrapper::setup_utility_functions(execution_manager, std::cout, std::cerr);
	if(voronota::duktaper::TerminalUtilities::is_stdin_from_terminal())
	{
		bool readline_failed=false;
		while(!readline_failed && !execution_manager.exit_requested())
		{
			voronota::duktaper::DuktapeContextWrapper::eval(execution_manager, voronota::duktaper::TerminalUtilities::read_line_from_stdin(readline_failed), std::cout, std::cerr, true);
		}
	}
	else
	{
		std::istreambuf_iterator<char> eos;
		std::string script(std::istreambuf_iterator<char>(std::cin), eos);
		voronota::duktaper::DuktapeContextWrapper::eval(execution_manager, script, std::cout, std::cerr, false);
	}
	return 0;
}
