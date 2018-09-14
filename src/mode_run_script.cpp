#include "auxiliaries/program_options_handler.h"

#include "common/scripting/script_execution_manager.h"

namespace
{

class ScriptExecutionManager : public common::scripting::ScriptExecutionManager
{
public:
	ScriptExecutionManager()
	{
	}

protected:
	void on_before_any_command(const common::scripting::CommandInput& command_input)
	{
		std::cout << "\n> " << command_input.get_input_command_string() << std::endl;
	}

	void on_after_any_command(const common::scripting::CommandInput&)
	{
		std::cout << std::endl;
	}

	bool on_after_command_for_script_partitioner(const common::scripting::GenericCommandForScriptPartitioner::CommandRecord& cr)
	{
		print_command_log(cr);
		return cr.successful;
	}

	bool on_after_command_for_congregation_of_data_managers(const common::scripting::GenericCommandForCongregationOfDataManagers::CommandRecord& cr)
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

	bool on_after_command_for_data_manager(const common::scripting::GenericCommandForDataManager::CommandRecord& cr)
	{
		std::cout << cr.output_text;
		print_command_log(cr);
		return cr.successful;
	}

	bool on_after_command_for_extra_actions(const common::scripting::GenericCommandForExtraActions::CommandRecord& cr)
	{
		print_command_log(cr);
		return cr.successful;
	}

	void on_disallowed_command_for_data_manager()
	{
		std::cout << "Error: command is not allowed to use on multiple picked objects.";
	}

	void on_no_picked_data_manager()
	{
		std::cout << "Error: no object picked";
	}

	void  on_unrecognized_command(const common::scripting::CommandInput& command_input)
	{
		std::cout << "Error: unrecognized command '" << command_input.get_command_name() << "'";
	}

	void on_after_script(ScriptRecord& script_record)
	{
		if(!script_record.termination_error.empty())
		{
			std::cout << "Script termnation error: " << script_record.termination_error << std::endl;
		}
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

	ScriptExecutionManager execution_manager;

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

