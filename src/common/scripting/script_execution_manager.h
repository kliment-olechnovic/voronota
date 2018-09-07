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

		virtual void on_after_executing_command(const CommandInput&)
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
		set_command(new CustomCommandsForScriptPartitioner::set_alias(), "set-alias");
		set_command(new CustomCommandsForScriptPartitioner::unset_aliases(), "unset-aliases");
		set_command(new CustomCommandsForScriptPartitioner::source(), "source");
		set_command(new CustomCommandsForScriptPartitioner::exit(), "exit");

		set_command(new CustomCommandsForCongregationOfDataManagers::list_objects(), "list-objects");
		set_command(new CustomCommandsForCongregationOfDataManagers::delete_all_objects(), "delete-all-objects");
		set_command(new CustomCommandsForCongregationOfDataManagers::delete_objects(), "delete-objects");
		set_command(new CustomCommandsForCongregationOfDataManagers::rename_object(), "rename-object");
		set_command(new CustomCommandsForCongregationOfDataManagers::copy_object(), "copy-object");
		set_command(new CustomCommandsForCongregationOfDataManagers::load_atoms(), "load-atoms");
		set_command(new CustomCommandsForCongregationOfDataManagers::load_atoms_and_contacts(), "load-atoms-and-contacts");
		set_command(new CustomCommandsForCongregationOfDataManagers::with(), "with");

		set_command(new CustomCommandsForDataManager::restrict_atoms(), "restrict-atoms");
		set_command(new CustomCommandsForDataManager::save_atoms(), "save-atoms");
		set_command(new CustomCommandsForDataManager::select_atoms(), "select-atoms");
		set_command(new CustomCommandsForDataManager::tag_atoms(), "tag-atoms");
		set_command(new CustomCommandsForDataManager::untag_atoms(), "untag-atoms");
		set_command(new CustomCommandsForDataManager::mark_atoms(), "mark-atoms");
		set_command(new CustomCommandsForDataManager::unmark_atoms(), "unmark-atoms");
		set_command(new CustomCommandsForDataManager::show_atoms(), "show-atoms");
		set_command(new CustomCommandsForDataManager::hide_atoms(), "hide-atoms");
		set_command(new CustomCommandsForDataManager::color_atoms(), "color-atoms");
		set_command(new CustomCommandsForDataManager::spectrum_atoms(), "spectrum-atoms");
		set_command(new CustomCommandsForDataManager::print_atoms(), "print-atoms");
		set_command(new CustomCommandsForDataManager::print_sequence(), "print-sequence");
		set_command(new CustomCommandsForDataManager::zoom_by_atoms(), "zoom-by-atoms");
		set_command(new CustomCommandsForDataManager::write_atoms_as_pymol_cgo(), "write-atoms-as-pymol-cgo");
		set_command(new CustomCommandsForDataManager::write_cartoon_as_pymol_cgo(), "write-cartoon-as-pymol-cgo");
		set_command(new CustomCommandsForDataManager::list_selections_of_atoms(), "list-selections-of-atoms");
		set_command(new CustomCommandsForDataManager::delete_all_selections_of_atoms(), "delete-all-selections-of-atoms");
		set_command(new CustomCommandsForDataManager::delete_selections_of_atoms(), "delete-selections-of-atoms");
		set_command(new CustomCommandsForDataManager::rename_selection_of_atoms(), "rename-selection-of-atoms");
		set_command(new CustomCommandsForDataManager::construct_contacts(), "construct-contacts");
		set_command(new CustomCommandsForDataManager::save_contacts(), "save-contacts");
		set_command(new CustomCommandsForDataManager::load_contacts(), "load-contacts");
		set_command(new CustomCommandsForDataManager::select_contacts(), "select-contacts");
		set_command(new CustomCommandsForDataManager::tag_contacts(), "tag-contacts");
		set_command(new CustomCommandsForDataManager::untag_contacts(), "untag-contacts");
		set_command(new CustomCommandsForDataManager::mark_contacts(), "mark-contacts");
		set_command(new CustomCommandsForDataManager::unmark_contacts(), "unmark-contacts");
		set_command(new CustomCommandsForDataManager::show_contacts(), "show-contacts");
		set_command(new CustomCommandsForDataManager::hide_contacts(), "hide-contacts");
		set_command(new CustomCommandsForDataManager::color_contacts(), "color-contacts");
		set_command(new CustomCommandsForDataManager::spectrum_contacts(), "spectrum-contacts");
		set_command(new CustomCommandsForDataManager::print_contacts(), "print-contacts");
		set_command(new CustomCommandsForDataManager::zoom_by_contacts(), "zoom-by-contacts");
		set_command(new CustomCommandsForDataManager::write_contacts_as_pymol_cgo(), "write-contacts-as-pymol-cgo");
		set_command(new CustomCommandsForDataManager::list_selections_of_contacts(), "list-selections-of-contacts");
		set_command(new CustomCommandsForDataManager::delete_all_selections_of_contacts(), "delete-all-selections-of-contacts");
		set_command(new CustomCommandsForDataManager::delete_selections_of_contacts(), "delete-selections-of-contacts");
		set_command(new CustomCommandsForDataManager::rename_selection_of_contacts(), "rename-selection-of-contacts");
		set_command(new CustomCommandsForDataManager::save_atoms_and_contacts(), "save-atoms-and-contacts");
	}

	virtual ~ScriptExecutionManager()
	{
		delete_map_contents(commands_for_script_partitioner_);
		delete_map_contents(commands_for_congregation_of_data_managers_);
		delete_map_contents(commands_for_data_manager_);
	}

	GenericCommandForCongregationOfDataManagers* set_command(GenericCommandForCongregationOfDataManagers* command_ptr, const std::string& name)
	{
		return set_map_key_value_safely(commands_for_congregation_of_data_managers_, name, command_ptr);
	}

	GenericCommandForDataManager* set_command(GenericCommandForDataManager* command_ptr, const std::string& name)
	{
		return set_map_key_value_safely(commands_for_data_manager_, name, command_ptr);
	}

	void unset_command(const std::string& name)
	{
		unset_map_key_safely(commands_for_script_partitioner_, name);
		unset_map_key_safely(commands_for_congregation_of_data_managers_, name);
		unset_map_key_safely(commands_for_data_manager_, name);
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

	template<typename Map, typename Value>
	static Value set_map_key_value_safely(Map& map, const std::string& key, Value value)
	{
		typename Map::iterator it=map.find(key);
		if(it==map.end())
		{
			map[key]=value;
		}
		else
		{
			delete it->second;
			it->second=value;
		}
		return value;
	}

	template<typename Map>
	static void unset_map_key_safely(Map& map, const std::string& key)
	{
		typename Map::iterator it=map.find(key);
		if(it!=map.end())
		{
			delete it->second;
			map.erase(it);
		}
	}

	GenericCommandForScriptPartitioner* set_command(GenericCommandForScriptPartitioner* command_ptr, const std::string& name)
	{
		return set_map_key_value_safely(commands_for_script_partitioner_, name, command_ptr);
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

		handler.on_after_executing_command(command_record.command_input);
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
