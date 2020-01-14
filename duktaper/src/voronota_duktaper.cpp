#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "duktaper/duktape_wrapper.h"

namespace
{

inline bool is_stdin_from_terminal()
{
	return (isatty(fileno(stdin))==1);
}

inline std::string read_line_from_stdin(bool& failed)
{
	char* line=readline("> ");
	failed=(line==0);
	if(!failed)
	{
		std::string result;
		if(*line)
		{
			add_history(line);
			result=line;
		}
		free(static_cast<void*>(line));
		return result;
	}
	return std::string();
}

}

int main(const int /*argc*/, const char** /*argv*/)
{
	voronota::scripting::ScriptExecutionManagerWithVariantOutput execution_manager;
	if(is_stdin_from_terminal())
	{
		bool readline_failed=false;
		while(!readline_failed && !execution_manager.exit_requested())
		{
			voronota::duktaper::DuktapeContextWrapper::eval(execution_manager, read_line_from_stdin(readline_failed), std::cout, std::cerr, true);
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
