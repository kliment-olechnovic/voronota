#include "auxiliaries/program_options_handler.h"

#include "common/scripting/script_execution_manager.h"
#include "common/scripting/json_writer.h"

namespace common
{

namespace scripting
{

class ScriptExecutionManagerWithVariantOutput : public ScriptExecutionManager
{
public:
	ScriptExecutionManagerWithVariantOutput()
	{
	}

	const VariantObject& execute_script_and_return_last_output(const std::string& script, const bool exit_on_first_failure)
	{
		execute_script(script, exit_on_first_failure);
		return output_;
	}

	const VariantObject& last_output() const
	{
		return output_;
	}

protected:
	void on_before_script(const std::string&)
	{
		output_=VariantObject();
	}

	void on_before_any_command(const CommandInput& command_input)
	{
		current_command_object(true).value("command")=command_input.get_input_command_string();
	}

	void on_after_command_for_script_partitioner(const GenericCommandForScriptPartitioner::CommandRecord&)
	{
	}

	void on_after_command_for_congregation_of_data_managers(const GenericCommandForCongregationOfDataManagers::CommandRecord& cr)
	{
		for(std::set<DataManager*>::const_iterator it=cr.change_indicator.added_objects.begin();it!=cr.change_indicator.added_objects.end();++it)
		{
			DataManager& dm=(*(*it));
			dm.add_atoms_representations(std::vector<std::string>(1, "atoms"));
			dm.set_atoms_representation_implemented_always(0, true);
			dm.add_contacts_representations(std::vector<std::string>(1, "contacts"));
			dm.set_contacts_representation_implemented_always(0, true);
		}
	}

	void on_after_command_for_data_manager(const GenericCommandForDataManager::CommandRecord& cr)
	{
		const CongregationOfDataManagers::ObjectAttributes object_attributes=congregation_of_data_managers().get_object_attributes(cr.data_manager_ptr);
		current_command_object().value("on_object")=object_attributes.name;
	}

	void on_after_command_for_extra_actions(const GenericCommand::CommandRecord&)
	{
	}

	void on_command_not_allowed_for_multiple_data_managers(const CommandInput&)
	{
		write_error_to_current_command_object("Command is not allowed to use on multiple picked objects.");
	}

	void on_no_picked_data_manager_for_command(const CommandInput&)
	{
		write_error_to_current_command_object("No objects picked.");
	}

	void on_unrecognized_command(const std::string&)
	{
		write_error_to_current_command_object("Command unrecognized.");
	}

	void on_after_any_command(const GenericCommand::CommandRecord& cr)
	{
		for(std::size_t i=0;i<cr.heterostorage.errors.size();i++)
		{
			write_error_to_current_command_object(cr.heterostorage.errors[i]);
		}

		if(!cr.heterostorage.variant_object.empty())
		{
			current_command_object().object("results")=cr.heterostorage.variant_object;
		}

		on_after_any_command_with_output(current_command_object());
	}

	void on_after_script(const ScriptRecord& script_record)
	{
		VariantObject& summary=output_.object("script_summary");

		summary.value("executed")=script_record.command_records.size();
		summary.value("executed_successfully")=script_record.count_successfull_commmand_records();

		if(!script_record.termination_error.empty())
		{
			summary.value("termination_error")=script_record.termination_error;
		}

		on_after_script_with_output(summary);
	}

	virtual void on_after_any_command_with_output(const VariantObject&)
	{
	}

	virtual void on_after_script_with_output(const VariantObject&)
	{
	}

private:
	VariantObject& current_command_object(const bool add)
	{
		std::vector<VariantObject>& command_objects=output_.objects_array("commands");
		if(add || command_objects.empty())
		{
			command_objects.push_back(VariantObject());
		}
		return command_objects.back();
	}

	VariantObject& current_command_object()
	{
		return current_command_object(false);
	}

	void write_error_to_current_command_object(const std::string& error)
	{
		current_command_object().values_array("errors").push_back(VariantValue(error));
	}

	VariantObject output_;
};

}

}

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

