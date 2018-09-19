#ifndef COMMON_SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_
#define COMMON_SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_

#include "custom_commands_for_script_partitioner.h"
#include "custom_commands_for_congregation_of_data_managers.h"
#include "custom_commands_for_data_manager.h"
#include "custom_commands_for_extra_actions.h"

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

	ScriptExecutionManager() :
		exit_requested_(false)
	{
		set_command("set-alias", new CustomCommandsForScriptPartitioner::set_alias());
		set_command("unset-aliases", new CustomCommandsForScriptPartitioner::unset_aliases());
		set_command("source", new CustomCommandsForScriptPartitioner::source());

		set_command("list-objects", new CustomCommandsForCongregationOfDataManagers::list_objects());
		set_command("delete-all-objects", new CustomCommandsForCongregationOfDataManagers::delete_all_objects());
		set_command("delete-objects", new CustomCommandsForCongregationOfDataManagers::delete_objects());
		set_command("rename-object", new CustomCommandsForCongregationOfDataManagers::rename_object());
		set_command("copy-object", new CustomCommandsForCongregationOfDataManagers::copy_object());
		set_command("load-atoms", new CustomCommandsForCongregationOfDataManagers::load_atoms());
		set_command("load-atoms-and-contacts", new CustomCommandsForCongregationOfDataManagers::load_atoms_and_contacts());
		set_command("pick-all-objects", new CustomCommandsForCongregationOfDataManagers::pick_all_objects());
		set_command("unpick-all-objects", new CustomCommandsForCongregationOfDataManagers::unpick_all_objects());
		set_command("pick-objects", new CustomCommandsForCongregationOfDataManagers::pick_objects());
		set_command("pick-more-objects", new CustomCommandsForCongregationOfDataManagers::pick_more_objects());
		set_command("unpick-objects", new CustomCommandsForCongregationOfDataManagers::unpick_objects());
		set_command("show-all-objects", new CustomCommandsForCongregationOfDataManagers::show_all_objects());
		set_command("hide-all-objects", new CustomCommandsForCongregationOfDataManagers::hide_all_objects());
		set_command("show-objects", new CustomCommandsForCongregationOfDataManagers::show_objects());
		set_command("hide-objects", new CustomCommandsForCongregationOfDataManagers::hide_objects());
		set_command("zoom-by-objects", new CustomCommandsForCongregationOfDataManagers::zoom_by_objects());

		set_command("restrict-atoms", new CustomCommandsForDataManager::restrict_atoms());
		set_command("save-atoms", new CustomCommandsForDataManager::save_atoms());
		set_command("select-atoms", new CustomCommandsForDataManager::select_atoms());
		set_command("tag-atoms", new CustomCommandsForDataManager::tag_atoms());
		set_command("tag-atoms-by-secondary-structure", new CustomCommandsForDataManager::tag_atoms_by_secondary_structure());
		set_command("delete-tags-of-atoms", new CustomCommandsForDataManager::delete_tags_of_atoms());
		set_command("adjunct-atoms", new CustomCommandsForDataManager::adjunct_atoms());
		set_command("delete-adjuncts-of-atoms", new CustomCommandsForDataManager::delete_adjuncts_of_atoms());
		set_command("mark-atoms", new CustomCommandsForDataManager::mark_atoms());
		set_command("unmark-atoms", new CustomCommandsForDataManager::unmark_atoms());
		set_command("show-atoms", new CustomCommandsForDataManager::show_atoms());
		set_command("hide-atoms", new CustomCommandsForDataManager::hide_atoms());
		set_command("color-atoms", new CustomCommandsForDataManager::color_atoms());
		set_command("spectrum-atoms", new CustomCommandsForDataManager::spectrum_atoms());
		set_command("print-atoms", new CustomCommandsForDataManager::print_atoms());
		set_command("print-sequence", new CustomCommandsForDataManager::print_sequence());
		set_command("zoom-by-atoms", new CustomCommandsForDataManager::zoom_by_atoms());
		set_command("write-atoms-as-pymol-cgo", new CustomCommandsForDataManager::write_atoms_as_pymol_cgo());
		set_command("write-cartoon-as-pymol-cgo", new CustomCommandsForDataManager::write_cartoon_as_pymol_cgo());
		set_command("list-selections-of-atoms", new CustomCommandsForDataManager::list_selections_of_atoms());
		set_command("delete-selections-of-atoms", new CustomCommandsForDataManager::delete_selections_of_atoms());
		set_command("rename-selection-of-atoms", new CustomCommandsForDataManager::rename_selection_of_atoms());
		set_command("construct-contacts", new CustomCommandsForDataManager::construct_contacts());
		set_command("save-contacts", new CustomCommandsForDataManager::save_contacts());
		set_command("load-contacts", new CustomCommandsForDataManager::load_contacts());
		set_command("select-contacts", new CustomCommandsForDataManager::select_contacts());
		set_command("tag-contacts", new CustomCommandsForDataManager::tag_contacts());
		set_command("delete-tags-of-contacts", new CustomCommandsForDataManager::delete_tags_of_contacts());
		set_command("adjunct-contacts", new CustomCommandsForDataManager::adjunct_contacts());
		set_command("delete-adjuncts-of-contacts", new CustomCommandsForDataManager::delete_adjuncts_of_contacts());
		set_command("mark-contacts", new CustomCommandsForDataManager::mark_contacts());
		set_command("unmark-contacts", new CustomCommandsForDataManager::unmark_contacts());
		set_command("show-contacts", new CustomCommandsForDataManager::show_contacts());
		set_command("hide-contacts", new CustomCommandsForDataManager::hide_contacts());
		set_command("color-contacts", new CustomCommandsForDataManager::color_contacts());
		set_command("spectrum-contacts", new CustomCommandsForDataManager::spectrum_contacts());
		set_command("print-contacts", new CustomCommandsForDataManager::print_contacts());
		set_command("zoom-by-contacts", new CustomCommandsForDataManager::zoom_by_contacts());
		set_command("write-contacts-as-pymol-cgo", new CustomCommandsForDataManager::write_contacts_as_pymol_cgo());
		set_command("list-selections-of-contacts", new CustomCommandsForDataManager::list_selections_of_contacts());
		set_command("delete-selections-of-contacts", new CustomCommandsForDataManager::delete_selections_of_contacts());
		set_command("rename-selection-of-contacts", new CustomCommandsForDataManager::rename_selection_of_contacts());
		set_command("save-atoms-and-contacts", new CustomCommandsForDataManager::save_atoms_and_contacts());

		set_command("reset-time", new CustomsCommandsForExtraActions::reset_time(elapsed_processor_time_));
		set_command("print-time", new CustomsCommandsForExtraActions::print_time(elapsed_processor_time_));
		set_command("exit", new CustomsCommandsForExtraActions::exit(exit_requested_));
		set_command("echo", new CustomsCommandsForExtraActions::echo());
	}

	virtual ~ScriptExecutionManager()
	{
		delete_map_contents(commands_for_script_partitioner_);
		delete_map_contents(commands_for_congregation_of_data_managers_);
		delete_map_contents(commands_for_data_manager_);
		delete_map_contents(commands_for_extra_actions_);
	}

	bool exit_requested() const
	{
		return exit_requested_;
	}

	ScriptRecord execute_script(const std::string& script, const bool exit_on_first_failure)
	{
		ScriptRecord script_record;
		execute_script(script, exit_on_first_failure, script_record);
		on_after_script(script_record);
		return script_record;
	}

protected:
	ScriptPartitioner& script_partitioner()
	{
		return script_partitioner_;
	}

	CongregationOfDataManagers& congregation_of_data_managers()
	{
		return congregation_of_data_managers_;
	}

	GenericCommandForScriptPartitioner* set_command(const std::string& name, GenericCommandForScriptPartitioner* command_ptr)
	{
		return set_map_key_value_safely(commands_for_script_partitioner_, name, command_ptr);
	}

	GenericCommandForCongregationOfDataManagers* set_command(const std::string& name, GenericCommandForCongregationOfDataManagers* command_ptr)
	{
		return set_map_key_value_safely(commands_for_congregation_of_data_managers_, name, command_ptr);
	}

	GenericCommandForDataManager* set_command(const std::string& name, GenericCommandForDataManager* command_ptr)
	{
		return set_map_key_value_safely(commands_for_data_manager_, name, command_ptr);
	}

	GenericCommandForExtraActions* set_command(const std::string& name, GenericCommandForExtraActions* command_ptr)
	{
		return set_map_key_value_safely(commands_for_extra_actions_, name, command_ptr);
	}

	void unset_command(const std::string& name)
	{
		unset_map_key_safely(commands_for_script_partitioner_, name);
		unset_map_key_safely(commands_for_congregation_of_data_managers_, name);
		unset_map_key_safely(commands_for_data_manager_, name);
		unset_map_key_safely(commands_for_extra_actions_, name);
	}

	virtual void on_before_any_command(const CommandInput&)
	{
	}

	virtual void on_after_any_command(const CommandInput&)
	{
	}

	virtual bool on_after_command_for_script_partitioner(const GenericCommandForScriptPartitioner::CommandRecord& cr)
	{
		return cr.successful;
	}

	virtual bool on_after_command_for_congregation_of_data_managers(const GenericCommandForCongregationOfDataManagers::CommandRecord& cr)
	{
		return cr.successful;
	}

	virtual bool on_after_command_for_data_manager(const GenericCommandForDataManager::CommandRecord& cr)
	{
		return cr.successful;
	}

	virtual bool on_after_command_for_extra_actions(const GenericCommandForExtraActions::CommandRecord& cr)
	{
		return cr.successful;
	}

	virtual void on_disallowed_command_for_data_manager()
	{
	}

	virtual void on_no_picked_data_manager()
	{
	}

	virtual void on_unrecognized_command(const CommandInput&)
	{
	}

	virtual void on_after_script(ScriptRecord&)
	{
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
			if(value!=0)
			{
				map[key]=value;
			}
		}
		else
		{
			delete it->second;
			if(value==0)
			{
				map.erase(it);
			}
			else
			{
				it->second=value;
			}
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

	void execute_script(const std::string& script, const bool exit_on_first_failure, ScriptRecord& script_record)
	{
		if(exit_requested())
		{
			script_record.termination_error="Cannot execute anything because exit was requested.";
			return;
		}

		try
		{
			script_partitioner_.add_pending_sentences_from_string_to_front(script);
		}
		catch(const std::exception& e)
		{
			script_record.termination_error=e.what();
			return;
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
				return;
			}

			CommandRecord command_record;

			try
			{
				command_record.command_input=CommandInput(command_string);
			}
			catch(const std::exception& e)
			{
				script_record.termination_error=e.what();
				return;
			}

			execute_command(command_record);

			script_record.command_records.push_back(command_record);

			if(!command_record.successful && exit_on_first_failure)
			{
				script_record.termination_error="Terminated on the first failure.";
				return;
			}

			if(exit_requested())
			{
				return;
			}
		}

		return;
	}

	void execute_command(CommandRecord& command_record)
	{
		on_before_any_command(command_record.command_input);

		const std::string& command_name=command_record.command_input.get_command_name();

		if(commands_for_script_partitioner_.count(command_name)==1)
		{
			command_record.recognized=1;
			command_record.successful=on_after_command_for_script_partitioner(commands_for_script_partitioner_[command_name]->execute(command_record.command_input, script_partitioner_));
		}
		else if(commands_for_congregation_of_data_managers_.count(command_name)==1)
		{
			command_record.recognized=2;
			command_record.successful=on_after_command_for_congregation_of_data_managers(commands_for_congregation_of_data_managers_[command_name]->execute(command_record.command_input, congregation_of_data_managers_));
		}
		else if(commands_for_data_manager_.count(command_name)==1)
		{
			command_record.recognized=3;
			std::vector<DataManager*> picked_data_managers=congregation_of_data_managers_.get_objects(true, false);
			if(!picked_data_managers.empty())
			{
				GenericCommandForDataManager* command_for_data_manager=commands_for_data_manager_[command_name];
				if(picked_data_managers.size()==1 || command_for_data_manager->allowed_to_work_on_multiple_data_managers(command_record.command_input))
				{
					for(std::size_t i=0;i<picked_data_managers.size();i++)
					{
						command_record.successful=on_after_command_for_data_manager(commands_for_data_manager_[command_name]->execute(command_record.command_input, *picked_data_managers[i]));
					}
				}
				else
				{
					on_disallowed_command_for_data_manager();
				}
			}
			else
			{
				on_no_picked_data_manager();
			}
		}
		else if(commands_for_extra_actions_.count(command_name)==1)
		{
			command_record.recognized=4;
			command_record.successful=on_after_command_for_extra_actions(commands_for_extra_actions_[command_name]->execute(command_record.command_input));
		}
		else
		{
			on_unrecognized_command(command_record.command_input);
		}

		on_after_any_command(command_record.command_input);
	}

	std::map<std::string, GenericCommandForScriptPartitioner*> commands_for_script_partitioner_;
	std::map<std::string, GenericCommandForCongregationOfDataManagers*> commands_for_congregation_of_data_managers_;
	std::map<std::string, GenericCommandForDataManager*> commands_for_data_manager_;
	std::map<std::string, GenericCommandForExtraActions*> commands_for_extra_actions_;
	ScriptPartitioner script_partitioner_;
	CongregationOfDataManagers congregation_of_data_managers_;
	auxiliaries::ElapsedProcessorTime elapsed_processor_time_;
	bool exit_requested_;
};

}

}

#endif /* COMMON_SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_ */
