#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/time_utilities.h"

#include "common/scripting/script_execution_manager.h"

namespace
{

class CustomsCommandsForExtraActions
{
public:
	class reset_time : public common::scripting::GenericCommandForExtraActions
	{
	public:
		reset_time(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) :
			elapsed_processor_time_(elapsed_processor_time)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			elapsed_processor_time_.reset();
		}

	private:
		auxiliaries::ElapsedProcessorTime& elapsed_processor_time_;
	};

	class print_time : public common::scripting::GenericCommandForExtraActions
	{
	public:
		print_time(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) :
			elapsed_processor_time_(elapsed_processor_time)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			const bool reset=cargs.input.get_flag("reset");
			cargs.input.assert_nothing_unusable();
			cargs.output_for_log << "Elapsed time: " << elapsed_processor_time_.elapsed_miliseconds() << " ms\n";
			if(reset)
			{
				elapsed_processor_time_.reset();
			}
		}

	private:
		auxiliaries::ElapsedProcessorTime& elapsed_processor_time_;
	};
};

class HandlerForExecutionEvents : public common::scripting::ScriptExecutionManager::HandlerForExecutionEvents
{
public:
	bool exit_requested;

	explicit HandlerForExecutionEvents(common::scripting::ScriptExecutionManager& esecution_manager) : exit_requested(false)
	{
		esecution_manager.set_command("reset-time", new CustomsCommandsForExtraActions::reset_time(elapsed_processor_time_));
		esecution_manager.set_command("print-time", new CustomsCommandsForExtraActions::print_time(elapsed_processor_time_));
	}

	void on_before_executing_command(const common::scripting::CommandInput& command_input)
	{
		std::cout << "\n> " << command_input.get_input_command_string() << std::endl;
	}

	void on_after_executing_command(const common::scripting::CommandInput&)
	{
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

	common::scripting::ScriptExecutionManager esecution_manager;
	HandlerForExecutionEvents handler_for_execution_events(esecution_manager);

	while(!handler_for_execution_events.exit_requested && std::cin.good())
	{
		std::string line;
		std::getline(std::cin, line);
		if(!line.empty())
		{
			common::scripting::ScriptExecutionManager::ScriptRecord script_record=esecution_manager.execute_script(line, false, handler_for_execution_events);
			if(!script_record.termination_error.empty())
			{
				std::cout << "Script termnation error: " << script_record.termination_error << std::endl;
			}
		}
	}
}

