#ifndef SCRIPTING_SCRIPT_EXECUTION_MANAGER_WITH_VARIANT_OUTPUT_H_
#define SCRIPTING_SCRIPT_EXECUTION_MANAGER_WITH_VARIANT_OUTPUT_H_

#include "script_execution_manager.h"
#include "variant_types.h"
#include "json_writer.h"

namespace voronota
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
		return last_output();
	}

	const std::string& execute_script_and_return_last_output_string(const std::string& script, const bool exit_on_first_failure)
	{
		execute_script(script, exit_on_first_failure);
		return last_output_string();
	}

	const VariantObject& last_output() const
	{
		return output_;
	}

	const std::string& last_output_string()
	{
		if(output_string_.empty())
		{
			output_string_=JSONWriter::write(JSONWriter::Configuration(0), last_output());
		}
		return output_string_;
	}

protected:
	void on_before_script(const std::string&)
	{
		output_=VariantObject();
		output_string_.clear();
	}

	void on_before_any_command(const CommandInput& command_input)
	{
		current_command_object(true).value("command_line")=command_input.get_input_command_string();
		current_command_object().value("command_name")=command_input.get_command_name();
	}

	void on_after_command_for_script_partitioner(const GenericCommandRecord&, ScriptPartitioner&)
	{
	}

	void on_after_command_for_congregation_of_data_managers(const GenericCommandRecord&, CongregationOfDataManagers&)
	{
	}

	void on_after_command_for_data_manager(const GenericCommandRecord&, DataManager& data_manager)
	{
		const CongregationOfDataManagers::ObjectAttributes object_attributes=congregation_of_data_managers().get_object_attributes(&data_manager);
		current_command_object().value("data_object")=object_attributes.name;
	}

	void on_after_command_for_extra_actions(const GenericCommandRecord&)
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

	void on_after_any_command(const GenericCommandRecord& cr)
	{
		for(std::size_t i=0;i<cr.heterostorage.errors.size();i++)
		{
			write_error_to_current_command_object(cr.heterostorage.errors[i]);
		}

		if(!cr.heterostorage.variant_object.empty())
		{
			current_command_object().object("output")=cr.heterostorage.variant_object;
		}

		current_command_object().value("success")=cr.successful;

		on_after_any_command_with_output(current_command_object());
	}

	void on_after_script(const ScriptRecord& script_record)
	{
		VariantObject& summary=output_.object("results_summary");

		const std::size_t num_of_successfull_commmand_records=script_record.count_successfull_commmand_records();

		summary.value("count_all")=script_record.command_records.size();
		summary.value("count_successful")=num_of_successfull_commmand_records;

		if(!script_record.termination_error.empty())
		{
			summary.value("termination_error")=script_record.termination_error;
		}

		summary.value("full_success")=(script_record.command_records.size()==num_of_successfull_commmand_records && script_record.termination_error.empty());
		summary.value("partial_success")=(num_of_successfull_commmand_records>0);

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
		std::vector<VariantObject>& command_objects=output_.objects_array("results");
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
	std::string output_string_;
};

}

}

#endif /* SCRIPTING_SCRIPT_EXECUTION_MANAGER_WITH_VARIANT_OUTPUT_H_ */
