#include <iostream>
#include <unistd.h>
#include <stdio.h>

#include "duktaper/duktape_wrapper.h"

namespace
{

inline bool is_stdin_from_terminal()
{
	return (isatty(fileno(stdin))==1);
}

}

int main(const int /*argc*/, const char** /*argv*/)
{
	voronota::scripting::ScriptExecutionManagerWithVariantOutput execution_manager;
	if(is_stdin_from_terminal())
	{
		while(!execution_manager.exit_requested() && std::cin.good())
		{
			std::string line;
			std::getline(std::cin, line);
			if(!line.empty())
			{
				voronota::duktaper::DuktapeContextWrapper::eval(execution_manager, line, std::cout, std::cerr, true);
			}
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
