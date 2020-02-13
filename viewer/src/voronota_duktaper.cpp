#include <unistd.h>
#include <stdio.h>

#include "linenoise/linenoise.h"

#include "../../src/scripting/binding_javascript.h"
#include "viewer/duktape_manager.h"

namespace
{

inline std::string generate_command_args_init_script(const std::vector<std::string>& command_args)
{
	std::ostringstream output;
	output << "CommandArgs=[];\n";
	for(std::size_t i=0;i<command_args.size();i++)
	{
		output << "CommandArgs.push('" << command_args[i] << "');\n";
	}
	return output.str();
}

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

inline bool is_stdin_from_terminal()
{
	return (isatty(fileno(stdin))==1);
}

class LineReading
{
public:
	static std::string read_line_from_stdin(bool& failed)
	{
		ManagedLineBuffer mlb(linenoise("> "));
		failed=(mlb.line==0);
		if(!failed)
		{
			std::string result;
			if(*mlb.line)
			{
				linenoiseHistoryAdd(mlb.line);
				result=mlb.line;
			}
			return result;
		}
		return std::string();
	}

private:
	struct ManagedLineBuffer
	{
		char* line;

		ManagedLineBuffer(char* line) : line(line)
		{
		}

		~ManagedLineBuffer()
		{
			if(line!=0)
			{
				free(static_cast<void*>(line));
			}
		}
	};
};

}

int main(const int argc, const char** argv)
{
	try
	{
		std::vector<std::string> command_args;
		for(int i=1;i<argc;i++)
		{
			command_args.push_back(argv[i]);
		}
		if(command_args.empty())
		{
			command_args.push_back("-");
		}
		const std::string file_name=command_args[0];

		voronota::scripting::ScriptExecutionManagerWithVariantOutput execution_manager;
		voronota::viewer::DuktapeManager::set_script_execution_manager(execution_manager);
		voronota::viewer::DuktapeManager::eval(voronota::scripting::BindingJavascript::generate_setup_script(execution_manager.collection_of_command_documentations()));
		voronota::viewer::DuktapeManager::eval(generate_command_args_init_script(command_args));

		if(file_name=="-")
		{
			if(is_stdin_from_terminal())
			{
				bool readline_failed=false;
				while(!readline_failed && !execution_manager.exit_requested())
				{
					voronota::viewer::DuktapeManager::eval(LineReading::read_line_from_stdin(readline_failed), true);
				}
			}
			else
			{
				voronota::viewer::DuktapeManager::eval(read_script(std::cin));
			}
		}
		else
		{
			std::ifstream input(file_name.c_str(), std::ios::in);
			if(!input.good())
			{
				throw std::runtime_error(std::string("Invalid file '")+file_name+"'\n");
			}
			voronota::viewer::DuktapeManager::eval(read_script(input));
		}

		return 0;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown exception caught." << std::endl;
	}

	return 1;
}

