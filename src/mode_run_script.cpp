#include "auxiliaries/program_options_handler.h"

#include "common/scripting/script_execution_manager_with_variant_output.h"
#include "common/scripting/json_writer.h"

namespace
{

class CustomScriptExecutionManager : public common::scripting::ScriptExecutionManagerWithVariantOutput
{
public:
	CustomScriptExecutionManager(const bool print_progress) : print_progress_(print_progress)
	{
	}

protected:
	void on_after_any_command_with_output(const common::scripting::VariantObject& variant_object)
	{
		if(print_progress_)
		{
			common::scripting::JSONWriter::write(variant_object, std::cout);
			std::cout << std::endl;
		}
	}

	void on_after_script_with_output(const common::scripting::VariantObject& variant_object)
	{
		if(print_progress_ && variant_object.names().count("termination_error")>0)
		{
			common::scripting::JSONWriter::write(variant_object, std::cout);
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
	pohw.describe_io("stdout", false, true, "output on script execution");

	const bool not_interactive=poh.contains_option(pohw.describe_option("--not-interactive", "", "flag for noninteractive mode"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	common::scripting::JSONWriter::Configuration::setup_default_configuration(common::scripting::JSONWriter::Configuration(not_interactive ? 6 : 4));

	CustomScriptExecutionManager execution_manager(!not_interactive);

	if(not_interactive)
	{
		std::istreambuf_iterator<char> eos;
		std::string script(std::istreambuf_iterator<char>(std::cin), eos);
		common::scripting::JSONWriter::write(execution_manager.execute_script_and_return_last_output(script, false), std::cout);
	}
	else
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
}

