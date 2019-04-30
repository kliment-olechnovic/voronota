#include "auxiliaries/program_options_handler.h"

#include "scripting/script_execution_manager_with_variant_output.h"
#include "scripting/json_writer.h"

namespace
{

class CustomScriptExecutionManager : public scripting::ScriptExecutionManagerWithVariantOutput
{
public:
	explicit CustomScriptExecutionManager(const bool print_progress) : print_progress_(print_progress)
	{
	}

protected:
	void on_after_any_command_with_output(const scripting::VariantObject& variant_object)
	{
		if(print_progress_)
		{
			scripting::JSONWriter::write(variant_object, std::cout);
			std::cout << std::endl;
		}
	}

	void on_after_script_with_output(const scripting::VariantObject& variant_object)
	{
		if(print_progress_ && variant_object.names().count("termination_error")>0)
		{
			scripting::JSONWriter::write(variant_object, std::cout);
			std::cout << std::endl;
		}
	}

private:
	bool print_progress_;
};

}

void run_script(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "script as plain text");
	pohw.describe_io("stdout", false, true, "output of script execution");

	const bool interactive=poh.contains_option(pohw.describe_option("--interactive", "", "flag for interactive mode"));
	const int max_unfolding=poh.argument<int>(pohw.describe_option("--max-unfolding", "number", "maximum level of output unfolding, default is 6"), 6);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	scripting::JSONWriter::Configuration::setup_default_configuration(scripting::JSONWriter::Configuration(max_unfolding));

	CustomScriptExecutionManager execution_manager(interactive);

	if(interactive)
	{
		while(!execution_manager.exit_requested() && std::cin.good())
		{
			std::string line;
			std::getline(std::cin, line);
			if(!line.empty())
			{
				execution_manager.execute_script(line, false);
			}
		}
	}
	else
	{
		std::istreambuf_iterator<char> eos;
		std::string script(std::istreambuf_iterator<char>(std::cin), eos);
		scripting::JSONWriter::write(execution_manager.execute_script_and_return_last_output(script, false), std::cout);
		std::cout << std::endl;
	}
}

