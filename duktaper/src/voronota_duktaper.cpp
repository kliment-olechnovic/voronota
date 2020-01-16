#include <iostream>
#include <fstream>

#include "duktaper/terminal_utilities.h"
#include "duktaper/duktape_wrapper.h"

namespace
{

inline std::string read_script(std::istream& input)
{
	std::ostringstream output;
	int number_of_lines=0;
	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		if(!line.empty() && (number_of_lines>0 ||  line[0]!='#'))
		{
			output << line << "\n";
		}
		number_of_lines++;
	}
	return output.str();
}

}

int main(const int argc, const char** argv)
{
	std::vector<std::string> command_args;
	{
		int i=1;
		while(i<argc)
		{
			const std::string argv_i=argv[i];
			command_args.push_back(argv_i);
			i++;
		}
	}

	voronota::scripting::ScriptExecutionManagerWithVariantOutput execution_manager;
	voronota::duktaper::DuktapeContextWrapper::setup(execution_manager, std::cout, std::cerr, command_args);

	const std::string file_name=(command_args.empty() ? std::string("-") : std::string(command_args[0]));

	if(file_name=="-")
	{
		if(voronota::duktaper::TerminalUtilities::is_stdin_from_terminal())
		{
			voronota::duktaper::DuktapeContextWrapper::set_eval_autoprint(true);
			bool readline_failed=false;
			while(!readline_failed && !execution_manager.exit_requested())
			{
				voronota::duktaper::DuktapeContextWrapper::eval(voronota::duktaper::TerminalUtilities::read_line_from_stdin(readline_failed));
			}
		}
		else
		{
			voronota::duktaper::DuktapeContextWrapper::eval(read_script(std::cin));
		}
	}
	else
	{
		std::ifstream input(file_name.c_str(), std::ios::in);
		if(!input.good())
		{
			std::cerr << "Invalid file '" << file_name << "'\n";
			return 1;
		}
		voronota::duktaper::DuktapeContextWrapper::eval(read_script(input));
	}

	return 0;
}
