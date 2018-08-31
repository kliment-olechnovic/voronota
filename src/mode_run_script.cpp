#include "auxiliaries/program_options_handler.h"

#include "common/scripting/script_execution_manager.h"

namespace
{

struct HandlerForScriptPartitioning
{
	bool operator()(const common::scripting::GenericCommandForScriptPartitioner::CommandRecord& cr) const
	{
		std::cout << "\n> " << cr.command_input.get_input_command_string() << std::endl;
		std::cout << cr.output_log;
		if(!cr.output_error.empty())
		{
			std::cout << "Error: " << cr.output_error << "\n";
		}
		std::cout << std::endl;
		return cr.successful;
	}
};

struct HandlerForDataManagment
{
	bool operator()(const common::scripting::GenericCommandForDataManager::CommandRecord& cr) const
	{
		std::cout << "\n> " << cr.command_input.get_input_command_string() << std::endl;
		std::cout << cr.output_text;
		std::cout << cr.output_log;
		if(!cr.output_error.empty())
		{
			std::cout << "Error: " << cr.output_error << "\n";
		}
		std::cout << std::endl;
		return cr.successful;
	}
};

struct HandlerForUnrecognizedCommandInput
{
	void operator()(const common::scripting::CommandInput& command_input) const
	{
		std::cout << "\n> " << command_input.get_input_command_string() << "\n";
		std::cout << "Error: unrecognized command";
		std::cout << std::endl;
	}
};

}

void run_script(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "script as plain text");
	pohw.describe_io("stdout", false, true, "output on script execution");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::istreambuf_iterator<char> eos;
	std::string script(std::istreambuf_iterator<char>(std::cin), eos);

	common::scripting::ScriptExecutionManager manager;

	HandlerForScriptPartitioning handler_for_script_partitioning;
	HandlerForDataManagment handler_for_data_management;
	HandlerForUnrecognizedCommandInput handler_for_unrecognized_command_input;

	common::scripting::ScriptExecutionManager::ScriptRecord script_record=
			manager.execute_script(
					script,
					false,
					handler_for_script_partitioning,
					handler_for_data_management,
					handler_for_unrecognized_command_input);

	if(!script_record.termination_error.empty())
	{
		std::cout << "Script termnation errot: " << script_record.termination_error << std::endl;
	}
}

