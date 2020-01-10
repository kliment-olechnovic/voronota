#include "../auxiliaries/program_options_handler.h"

#include "../scripting/script_execution_manager_with_variant_output.h"
#include "../scripting/json_writer.h"

void run_script(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "script as plain text");
	pohw.describe_io("stdout", false, true, "output of script execution");

	const bool interactive=poh.contains_option(pohw.describe_option("--interactive", "", "flag for interactive mode"));
	const bool exit_on_first_failure=poh.contains_option(pohw.describe_option("--exit-on-first-failure", "", "flag to terminate script when a command fails"));
	const int max_unfolding=poh.argument<int>(pohw.describe_option("--max-unfolding", "number", "maximum level of output unfolding, default is 6"), 6);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	voronota::scripting::JSONWriter::Configuration::setup_default_configuration(voronota::scripting::JSONWriter::Configuration(max_unfolding));

	voronota::scripting::ScriptExecutionManagerWithVariantOutput execution_manager;

	if(interactive)
	{
		while(!execution_manager.exit_requested() && std::cin.good())
		{
			std::string line;
			std::getline(std::cin, line);
			if(!line.empty())
			{
				voronota::scripting::JSONWriter::write(execution_manager.execute_script_and_return_last_output(line, false), std::cout);
				std::cout << std::endl;
			}
		}
	}
	else
	{
		std::istreambuf_iterator<char> eos;
		std::string script(std::istreambuf_iterator<char>(std::cin), eos);
		voronota::scripting::JSONWriter::write(execution_manager.execute_script_and_return_last_output(script, exit_on_first_failure), std::cout);
		std::cout << std::endl;
	}
}

