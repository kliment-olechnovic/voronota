#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/time_utilities.h"

#include "common/scripting/script_execution_manager.h"

namespace
{

class HandlerForExecutionEvents : public common::scripting::ScriptExecutionManager::HandlerForExecutionEvents
{
public:
	bool print_time;
	bool exit_requested;

	explicit HandlerForExecutionEvents(const bool print_time=false) : print_time(print_time), exit_requested(false)
	{
	}

	void on_before_executing_command(const common::scripting::CommandInput& command_input)
	{
		std::cout << "\n> " << command_input.get_input_command_string() << std::endl;
		elapsed_processor_time_.reset();
	}

	void on_after_executing_command(const common::scripting::CommandInput&)
	{
		if(print_time)
		{
			std::cout << "Command execution time: " << elapsed_processor_time_.elapsed_miliseconds() << " ms\n";
		}
		std::cout << std::endl;
	}

	bool on_command_for_script_partitioner(const common::scripting::GenericCommandForScriptPartitioner::CommandRecord& cr)
	{
		print_command_log(cr);
		if(cr.successful && cr.command_input.get_command_name()=="exit")
		{
			exit_requested=true;
		}
		return cr.successful;
	}

	bool on_command_for_congregation_of_data_managers(const common::scripting::GenericCommandForCongregationOfDataManagers::CommandRecord& cr)
	{
		print_command_log(cr);
		for(std::vector<common::scripting::DataManager*>::const_iterator it=cr.set_of_added_objects.begin();it!=cr.set_of_added_objects.end();++it)
		{
			common::scripting::DataManager& dm=(*(*it));
			dm.add_atoms_representations(std::vector<std::string>(1, "atoms"));
			dm.set_atoms_representation_implemented_always(0, true);
			dm.add_contacts_representations(std::vector<std::string>(1, "contacts"));
			dm.set_contacts_representation_implemented_always(0, true);
		}
		return cr.successful;
	}

	bool on_command_for_data_manager(const common::scripting::GenericCommandForDataManager::CommandRecord& cr)
	{
		std::cout << cr.output_text;
		print_command_log(cr);
		return cr.successful;
	}

	bool on_command_for_extra_actions(const common::scripting::GenericCommandForExtraActions::CommandRecord& cr)
	{
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

	auxiliaries::ElapsedProcessorTime elapsed_processor_time_;
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
	HandlerForExecutionEvents handler_for_execution_events(false);

	while(!handler_for_execution_events.exit_requested && std::cin.good())
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

