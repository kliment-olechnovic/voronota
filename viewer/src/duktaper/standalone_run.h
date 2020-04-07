#ifndef DUKTAPER_STANDALONE_RUN_H_
#define DUKTAPER_STANDALONE_RUN_H_

#include <unistd.h>
#include <stdio.h>

#include "../dependencies/linenoise/linenoise.h"

#include "binding_javascript.h"
#include "duktape_manager.h"
#include "stocked_data_resources.h"

namespace voronota
{

namespace duktaper
{

class StandaloneRun
{
public:
	static void run(const std::vector<std::string>& command_args)
	{
		if(command_args.empty())
		{
			throw std::runtime_error(std::string("No command arguments"));
		}

		operators::SetupDefaults().run(0);

		ScriptExecutionManager execution_manager;
		DuktapeManager::set_script_execution_manager(execution_manager);
		DuktapeManager::eval(BindingJavascript::generate_setup_script(execution_manager.collection_of_command_documentations()));
		DuktapeManager::eval(generate_command_args_init_script(command_args));

		const std::string file_name=command_args[0];

		if(file_name=="-")
		{
			if(is_stdin_from_terminal())
			{
				DuktapeManager::flag_to_print_result_on_eval()=true;
				bool readline_failed=false;
				while(!readline_failed && !execution_manager.exit_requested())
				{
					DuktapeManager::eval(LineReading::read_line_from_stdin(readline_failed));
				}
			}
			else
			{
				DuktapeManager::eval(read_script(std::cin));
			}
		}
		else
		{
			std::ifstream input(file_name.c_str(), std::ios::in);
			if(!input.good())
			{
				throw std::runtime_error(std::string("Invalid file '")+file_name+"'\n");
			}
			DuktapeManager::eval(read_script(input));
		}
	}

private:
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

	static std::string generate_command_args_init_script(const std::vector<std::string>& command_args)
	{
		std::ostringstream output;
		output << "CommandArgs=[];\n";
		for(std::size_t i=0;i<command_args.size();i++)
		{
			output << "CommandArgs.push('" << command_args[i] << "');\n";
		}
		return output.str();
	}

	static std::string read_script(std::istream& input)
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

	static bool is_stdin_from_terminal()
	{
		return (isatty(fileno(stdin))==1);
	}
};

}

}

#endif /* DUKTAPER_STANDALONE_RUN_H_ */
