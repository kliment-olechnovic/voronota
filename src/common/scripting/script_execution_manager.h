#ifndef COMMON_SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_
#define COMMON_SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_

#include "custom_commands_for_script_partitioner.h"
#include "custom_commands_for_congregation_of_data_managers.h"
#include "custom_commands_for_data_manager.h"

namespace common
{

namespace scripting
{

class ScriptExecutionManager
{
public:
	struct CommandRecord
	{
		CommandInput command_input;
		int recognized;
		bool successful;

		CommandRecord() : recognized(0), successful(false)
		{
		}
	};

	struct ScriptRecord
	{
		std::vector<CommandRecord> command_records;
		std::string termination_error;
	};

	class HandlerForExecutionEvents
	{
	public:
		HandlerForExecutionEvents()
		{
		}

		virtual ~HandlerForExecutionEvents()
		{
		}

		virtual void on_before_executing_command(const CommandInput&)
		{
		}

		virtual void on_after_executing_command()
		{
		}

		virtual bool on_command_for_script_partitioner(const GenericCommandForScriptPartitioner::CommandRecord&)=0;

		virtual bool on_command_for_congregation_of_data_managers(const GenericCommandForCongregationOfDataManagers::CommandRecord&)=0;

		virtual bool on_command_for_data_manager(const GenericCommandForDataManager::CommandRecord&)=0;

		virtual void on_no_enabled_data_manager()=0;

		virtual void on_unrecognized_command(const CommandInput&)=0;
	};

	ScriptExecutionManager()
	{
		commands_for_script_partitioner_["set-alias"]=new CustomCommandsForScriptPartitioner::set_alias();
		commands_for_script_partitioner_["unset-aliases"]=new CustomCommandsForScriptPartitioner::unset_aliases();
		commands_for_script_partitioner_["run-script"]=new CustomCommandsForScriptPartitioner::run_script();

		commands_for_congregation_of_data_managers_["list-objects"]=new CustomCommandsForCongregationOfDataManagers::list_objects();
		commands_for_congregation_of_data_managers_["delete-all-objects"]=new CustomCommandsForCongregationOfDataManagers::delete_all_objects();
		commands_for_congregation_of_data_managers_["delete-objects"]=new CustomCommandsForCongregationOfDataManagers::delete_objects();
		commands_for_congregation_of_data_managers_["rename-object"]=new CustomCommandsForCongregationOfDataManagers::rename_object();
		commands_for_congregation_of_data_managers_["copy-object"]=new CustomCommandsForCongregationOfDataManagers::copy_object();
		commands_for_congregation_of_data_managers_["load-atoms"]=new CustomCommandsForCongregationOfDataManagers::load_atoms();
		commands_for_congregation_of_data_managers_["load-atoms-and-contacts"]=new CustomCommandsForCongregationOfDataManagers::load_atoms_and_contacts();
		commands_for_congregation_of_data_managers_["with"]=new CustomCommandsForCongregationOfDataManagers::with();

		commands_for_data_manager_["restrict-atoms"]=new CustomCommandsForDataManager::restrict_atoms();
		commands_for_data_manager_["save-atoms"]=new CustomCommandsForDataManager::save_atoms();
		commands_for_data_manager_["select-atoms"]=new CustomCommandsForDataManager::select_atoms();
		commands_for_data_manager_["tag-atoms"]=new CustomCommandsForDataManager::tag_atoms();
		commands_for_data_manager_["untag-atoms"]=new CustomCommandsForDataManager::untag_atoms();
		commands_for_data_manager_["mark-atoms"]=new CustomCommandsForDataManager::mark_atoms();
		commands_for_data_manager_["unmark-atoms"]=new CustomCommandsForDataManager::unmark_atoms();
		commands_for_data_manager_["show-atoms"]=new CustomCommandsForDataManager::show_atoms();
		commands_for_data_manager_["hide-atoms"]=new CustomCommandsForDataManager::hide_atoms();
		commands_for_data_manager_["color-atoms"]=new CustomCommandsForDataManager::color_atoms();
		commands_for_data_manager_["spectrum-atoms"]=new CustomCommandsForDataManager::spectrum_atoms();
		commands_for_data_manager_["print-atoms"]=new CustomCommandsForDataManager::print_atoms();
		commands_for_data_manager_["print-sequence"]=new CustomCommandsForDataManager::print_sequence();
		commands_for_data_manager_["zoom-by-atoms"]=new CustomCommandsForDataManager::zoom_by_atoms();
		commands_for_data_manager_["write-atoms-as-pymol-cgo"]=new CustomCommandsForDataManager::write_atoms_as_pymol_cgo();
		commands_for_data_manager_["list-selections-of-atoms"]=new CustomCommandsForDataManager::list_selections_of_atoms();
		commands_for_data_manager_["delete-all-selections-of-atoms"]=new CustomCommandsForDataManager::delete_all_selections_of_atoms();
		commands_for_data_manager_["delete-selections-of-atoms"]=new CustomCommandsForDataManager::delete_selections_of_atoms();
		commands_for_data_manager_["rename-selection-of-atoms"]=new CustomCommandsForDataManager::rename_selection_of_atoms();
		commands_for_data_manager_["construct-contacts"]=new CustomCommandsForDataManager::construct_contacts();
		commands_for_data_manager_["save-contacts"]=new CustomCommandsForDataManager::save_contacts();
		commands_for_data_manager_["load-contacts"]=new CustomCommandsForDataManager::load_contacts();
		commands_for_data_manager_["select-contacts"]=new CustomCommandsForDataManager::select_contacts();
		commands_for_data_manager_["tag-contacts"]=new CustomCommandsForDataManager::tag_contacts();
		commands_for_data_manager_["untag-contacts"]=new CustomCommandsForDataManager::untag_contacts();
		commands_for_data_manager_["mark-contacts"]=new CustomCommandsForDataManager::mark_contacts();
		commands_for_data_manager_["unmark-contacts"]=new CustomCommandsForDataManager::unmark_contacts();
		commands_for_data_manager_["show-contacts"]=new CustomCommandsForDataManager::show_contacts();
		commands_for_data_manager_["hide-contacts"]=new CustomCommandsForDataManager::hide_contacts();
		commands_for_data_manager_["color-contacts"]=new CustomCommandsForDataManager::color_contacts();
		commands_for_data_manager_["spectrum-contacts"]=new CustomCommandsForDataManager::spectrum_contacts();
		commands_for_data_manager_["print-contacts"]=new CustomCommandsForDataManager::print_contacts();
		commands_for_data_manager_["zoom-by-contacts"]=new CustomCommandsForDataManager::zoom_by_contacts();
		commands_for_data_manager_["write-contacts-as-pymol-cgo"]=new CustomCommandsForDataManager::write_contacts_as_pymol_cgo();
		commands_for_data_manager_["list-selections-of-contacts"]=new CustomCommandsForDataManager::list_selections_of_contacts();
		commands_for_data_manager_["delete-all-selections-of-contacts"]=new CustomCommandsForDataManager::delete_all_selections_of_contacts();
		commands_for_data_manager_["delete-selections-of-contacts"]=new CustomCommandsForDataManager::delete_selections_of_contacts();
		commands_for_data_manager_["rename-selection-of-contacts"]=new CustomCommandsForDataManager::rename_selection_of_contacts();
		commands_for_data_manager_["save-atoms-and-contacts"]=new CustomCommandsForDataManager::save_atoms_and_contacts();
	}

	~ScriptExecutionManager()
	{
		delete_map_contents(commands_for_script_partitioner_);
		delete_map_contents(commands_for_data_manager_);
	}

	ScriptRecord execute_script(
			const std::string& script,
			const bool exit_on_first_failure,
			HandlerForExecutionEvents& handler)
	{
		ScriptRecord script_record;

		try
		{
			script_partitioner_.add_pending_sentences_from_string_to_front(script);
		}
		catch(const std::exception& e)
		{
			script_record.termination_error=e.what();
			return script_record;
		}

		while(!script_partitioner_.pending_sentences().empty())
		{
			std::string command_string;

			try
			{
				command_string=script_partitioner_.extract_pending_sentence();
			}
			catch(const std::exception& e)
			{
				script_record.termination_error=e.what();
				return script_record;
			}

			CommandRecord command_record;

			try
			{
				command_record.command_input=CommandInput(command_string);
			}
			catch(const std::exception& e)
			{
				script_record.termination_error=e.what();
				return script_record;
			}

			execute_command(handler, command_record);

			script_record.command_records.push_back(command_record);

			if(!command_record.successful && exit_on_first_failure)
			{
				script_record.termination_error="Terminated on the first failure.";
				return script_record;
			}
		}

		return script_record;
	}

private:
	template<typename Map>
	static void delete_map_contents(Map& map)
	{
		for(typename Map::iterator it=map.begin();it!=map.end();++it)
		{
			if(it->second!=0)
			{
				delete it->second;
			}
		}
	}

	void execute_command(HandlerForExecutionEvents& handler, CommandRecord& command_record)
	{
		handler.on_before_executing_command(command_record.command_input);

		const std::string& command_name=command_record.command_input.get_command_name();

		if(commands_for_script_partitioner_.count(command_name)==1)
		{
			command_record.recognized=1;
			command_record.successful=handler.on_command_for_script_partitioner(commands_for_script_partitioner_[command_name]->execute(command_record.command_input, script_partitioner_));
		}
		else if(commands_for_congregation_of_data_managers_.count(command_name)==1)
		{
			command_record.recognized=2;
			command_record.successful=handler.on_command_for_congregation_of_data_managers(commands_for_congregation_of_data_managers_[command_name]->execute(command_record.command_input, congregation_of_data_managers_));
		}
		else if(commands_for_data_manager_.count(command_name)==1)
		{
			command_record.recognized=3;
			std::vector<CongregationOfDataManagers::ObjectDescriptor> descriptors_of_enabled_data_managers=congregation_of_data_managers_.get_descriptors(true);
			if(!descriptors_of_enabled_data_managers.empty())
			{
				for(std::size_t i=0;i<descriptors_of_enabled_data_managers.size();i++)
				{
					command_record.successful=handler.on_command_for_data_manager(commands_for_data_manager_[command_name]->execute(command_record.command_input, descriptors_of_enabled_data_managers[i].data_manager()));
				}
			}
			else
			{
				handler.on_no_enabled_data_manager();
			}
		}
		else
		{
			handler.on_unrecognized_command(command_record.command_input);
		}

		handler.on_after_executing_command();
	}

	std::map<std::string, GenericCommandForScriptPartitioner*> commands_for_script_partitioner_;
	std::map<std::string, GenericCommandForCongregationOfDataManagers*> commands_for_congregation_of_data_managers_;
	std::map<std::string, GenericCommandForDataManager*> commands_for_data_manager_;
	ScriptPartitioner script_partitioner_;
	CongregationOfDataManagers congregation_of_data_managers_;
};

}

}

#endif /* COMMON_SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_ */
