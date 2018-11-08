#include "auxiliaries/program_options_handler.h"

#include "common/scripting/script_execution_manager.h"
#include "common/scripting/json_writer.h"

namespace
{

class ScriptExecutionManager : public common::scripting::ScriptExecutionManager
{
public:
	class Observer
	{
	public:
		Observer()
		{
		}

		virtual ~Observer()
		{
		}

		virtual void observe(common::scripting::VariantObject&)
		{
		}
	};

	ScriptExecutionManager() : observer_(0)
	{
	}

	const common::scripting::VariantObject& execute_script_and_return_output(const std::string& script, const bool exit_on_first_failure, Observer* observer)
	{
		observer_=observer;
		execute_script(script, exit_on_first_failure);
		return output_;
	}

protected:
	virtual void on_before_script(const std::string&)
	{
		output_=common::scripting::VariantObject();
	}

	void on_before_any_command(const common::scripting::CommandInput& command_input)
	{
		current_command_object(true).value("command")=command_input.get_input_command_string();
	}

	void on_after_command_for_script_partitioner(const common::scripting::GenericCommandForScriptPartitioner::CommandRecord&)
	{
	}

	void on_after_command_for_congregation_of_data_managers(const common::scripting::GenericCommandForCongregationOfDataManagers::CommandRecord& cr)
	{
		for(std::set<common::scripting::DataManager*>::const_iterator it=cr.change_indicator.added_objects.begin();it!=cr.change_indicator.added_objects.end();++it)
		{
			common::scripting::DataManager& dm=(*(*it));
			dm.add_atoms_representations(std::vector<std::string>(1, "atoms"));
			dm.set_atoms_representation_implemented_always(0, true);
			dm.add_contacts_representations(std::vector<std::string>(1, "contacts"));
			dm.set_contacts_representation_implemented_always(0, true);
		}
	}

	void on_after_command_for_data_manager(const common::scripting::GenericCommandForDataManager::CommandRecord& cr)
	{
		const common::scripting::CongregationOfDataManagers::ObjectAttributes object_attributes=congregation_of_data_managers().get_object_attributes(cr.data_manager_ptr);
		current_command_object().value("on_object")=object_attributes.name;
	}

	void on_after_command_for_extra_actions(const common::scripting::GenericCommand::CommandRecord&)
	{
	}

	void on_command_not_allowed_for_multiple_data_managers(const common::scripting::CommandInput&)
	{
		write_error_to_current_command_object("Command is not allowed to use on multiple picked objects.");
	}

	void on_no_picked_data_manager_for_command(const common::scripting::CommandInput&)
	{
		write_error_to_current_command_object("No objects picked.");
	}

	void on_unrecognized_command(const std::string&)
	{
		write_error_to_current_command_object("Command unrecognized.");
	}

	void on_after_any_command(const common::scripting::GenericCommand::CommandRecord& cr)
	{
		for(std::size_t i=0;i<cr.heterostorage.errors.size();i++)
		{
			write_error_to_current_command_object(cr.heterostorage.errors[i]);
		}

		if(!cr.heterostorage.variant_object.empty())
		{
			current_command_object().object("results")=cr.heterostorage.variant_object;
		}

		if(observer_!=0)
		{
			observer_->observe(current_command_object());
		}
	}

	void on_after_script(const ScriptRecord& script_record)
	{
		if(!script_record.termination_error.empty())
		{
			output_.value("termination_error")=script_record.termination_error;

			if(observer_!=0)
			{
				common::scripting::VariantObject report;
				report.value("termination_error")=script_record.termination_error;
				observer_->observe(report);
			}
		}
	}

private:
	common::scripting::VariantObject& current_command_object(const bool add)
	{
		std::vector<common::scripting::VariantObject>& command_objects=output_.objects_array("commands");
		if(add || command_objects.empty())
		{
			command_objects.push_back(common::scripting::VariantObject());
		}
		return command_objects.back();
	}

	common::scripting::VariantObject& current_command_object()
	{
		return current_command_object(false);
	}

	void write_error_to_current_command_object(const std::string& error)
	{
		current_command_object().values_array("errors").push_back(common::scripting::VariantValue(error));
	}

	Observer* observer_;
	common::scripting::VariantObject output_;
};

class PrintingObserver : public ScriptExecutionManager::Observer
{
public:
	void observe(common::scripting::VariantObject& object)
	{
		common::scripting::JSONWriter::write(object, std::cout);
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

	common::scripting::JSONWriter::Configuration::setup_default_configuration(common::scripting::JSONWriter::Configuration(4));

	ScriptExecutionManager execution_manager;
	PrintingObserver observer;

	while(!execution_manager.exit_requested() && std::cin.good())
	{
		std::string line;
		std::getline(std::cin, line);
		if(!line.empty())
		{
			execution_manager.execute_script_and_return_output(line, false, &observer);
		}
	}
}

