#include "auxiliaries/program_options_handler.h"

#include "common/scripting/script_execution_manager.h"

namespace
{

class HandlerForExecutionEvents : public common::scripting::ScriptExecutionManager::HandlerForExecutionEvents
{
public:
	void on_before_executing_command(const common::scripting::CommandInput& command_input)
	{
		std::cout << "\n> " << command_input.get_input_command_string() << std::endl;
	}

	void on_after_executing_command()
	{
		std::cout << std::endl;
	}

	bool on_command_for_script_partitioner(const common::scripting::GenericCommandForScriptPartitioner::CommandRecord& cr)
	{
		print_command_log(cr);
		return cr.successful;
	}

	bool on_command_for_congregation_of_data_managers(const common::scripting::GenericCommandForCongregationOfDataManagers::CommandRecord& cr)
	{
		print_command_log(cr);
		return cr.successful;
	}

	bool on_command_for_data_manager(const common::scripting::GenericCommandForDataManager::CommandRecord& cr)
	{
		std::cout << cr.output_text;
		print_command_log(cr);
		return cr.successful;
	}

	void on_no_enabled_data_manager()
	{
		std::cout << "Error: no object enabled";
	}

	void  on_unrecognized_command(const common::scripting::CommandInput& command_input)
	{
		std::cout << "Error: unrecognized command '" << command_input.get_command_name() << "'";
	}

private:
	template<typename CommandRecord>
	void print_command_log(const CommandRecord& cr)
	{
		std::cout << cr.output_log;
		if(!cr.output_error.empty())
		{
			std::cout << "Error: " << cr.output_error << "\n";
		}
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

	common::scripting::ScriptExecutionManager manager;
	HandlerForExecutionEvents handler_for_execution_events;

	while(std::cin.good())
	{
		std::string line;
		std::getline(std::cin, line);
		if(!line.empty())
		{
			common::scripting::ScriptExecutionManager::ScriptRecord script_record=manager.execute_script(line, false, handler_for_execution_events);
			if(!script_record.termination_error.empty())
			{
				std::cout << "Script termnation error: " << script_record.termination_error << std::endl;
			}
		}
	}
}

