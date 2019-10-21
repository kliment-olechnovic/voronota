#ifndef SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_
#define SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_

#include "custom_commands_for_script_partitioner.h"
#include "custom_commands_for_congregation_of_data_managers.h"
#include "custom_commands_for_data_manager.h"
#include "custom_commands_for_extra_actions.h"

namespace scripting
{

class ScriptExecutionManager
{
public:
	struct CommandRecord
	{
		CommandInput command_input;
		bool successful;

		CommandRecord() : successful(false)
		{
		}
	};

	struct ScriptRecord
	{
		std::vector<CommandRecord> command_records;
		std::string termination_error;

		std::size_t count_successfull_commmand_records() const
		{
			std::size_t n=0;
			for(std::size_t i=0;i<command_records.size();i++)
			{
				if(command_records[i].successful)
				{
					n++;
				}
			}
			return n;
		}
	};

	ScriptExecutionManager() :
		exit_requested_(false)
	{
		set_command("set-alias", new CustomCommandsForScriptPartitioner::set_alias());
		set_command("unset-aliases", new CustomCommandsForScriptPartitioner::unset_aliases());
		set_command("source", new CustomCommandsForScriptPartitioner::source());

		set_command("list-objects", new CustomCommandsForCongregationOfDataManagers::list_objects());
		set_command("delete-objects", new CustomCommandsForCongregationOfDataManagers::delete_objects());
		set_command("rename-object", new CustomCommandsForCongregationOfDataManagers::rename_object());
		set_command("copy-object", new CustomCommandsForCongregationOfDataManagers::copy_object());
		set_command("load", new CustomCommandsForCongregationOfDataManagers::load());
		set_command("pick-objects", new CustomCommandsForCongregationOfDataManagers::pick_objects());
		set_command("pick-more-objects", new CustomCommandsForCongregationOfDataManagers::pick_more_objects());
		set_command("unpick-objects", new CustomCommandsForCongregationOfDataManagers::unpick_objects());
		set_command("show-objects", new CustomCommandsForCongregationOfDataManagers::show_objects());
		set_command("hide-objects", new CustomCommandsForCongregationOfDataManagers::hide_objects());
		set_command("zoom-by-objects", new CustomCommandsForCongregationOfDataManagers::zoom_by_objects());
		set_command("cad-score", new CustomCommandsForCongregationOfDataManagers::cad_score());

		set_command("restrict-atoms", new CustomCommandsForDataManager::restrict_atoms());
		set_command("move-atoms", new CustomCommandsForDataManager::move_atoms());
		set_command("center-atoms", new CustomCommandsForDataManager::center_atoms());
		set_command("save-atoms", new CustomCommandsForDataManager::save_atoms());
		set_command("select-atoms", new CustomCommandsForDataManager::select_atoms());
		set_command("write-selection-of-atoms", new CustomCommandsForDataManager::write_selection_of_atoms());
		set_command("read-selection-of-atoms", new CustomCommandsForDataManager::read_selection_of_atoms());
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
		set_command("construct-triangulation", new CustomCommandsForDataManager::construct_triangulation());
		set_command("write-triangulation", new CustomCommandsForDataManager::write_triangulation());
		set_command("print-triangulation", new CustomCommandsForDataManager::print_triangulation());
		set_command("construct-contacts", new CustomCommandsForDataManager::construct_contacts());
		set_command("make-drawable-contacts", new CustomCommandsForDataManager::make_drawable_contacts());
		set_command("make-undrawable-contacts", new CustomCommandsForDataManager::make_undrawable_contacts());
		set_command("save-contacts", new CustomCommandsForDataManager::save_contacts());
		set_command("load-contacts", new CustomCommandsForDataManager::load_contacts());
		set_command("select-contacts", new CustomCommandsForDataManager::select_contacts());
		set_command("write-selection-of-contacts", new CustomCommandsForDataManager::write_selection_of_contacts());
		set_command("read-selection-of-contacts", new CustomCommandsForDataManager::read_selection_of_contacts());
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
		set_command("calculate-burial-depth", new CustomCommandsForDataManager::calculate_burial_depth());
		set_command("voromqa-global", new CustomCommandsForDataManager::voromqa_global());
		set_command("voromqa-local", new CustomCommandsForDataManager::voromqa_local);
		set_command("voromqa-frustration", new CustomCommandsForDataManager::voromqa_frustration);
		set_command("voromqa-membrane-place", new CustomCommandsForDataManager::voromqa_membrane_place);
		set_command("describe-exposure", new CustomCommandsForDataManager::describe_exposure);
		set_command("find-connected-components", new CustomCommandsForDataManager::find_connected_components);
		set_command("add-figure", new CustomCommandsForDataManager::add_figure);
		set_command("print-figures", new CustomCommandsForDataManager::print_figures);
		set_command("show-figures", new CustomCommandsForDataManager::show_figures);
		set_command("hide-figures", new CustomCommandsForDataManager::hide_figures);
		set_command("color-figures", new CustomCommandsForDataManager::color_figures);
		set_command("list-figures", new CustomCommandsForDataManager::list_figures);
		set_command("delete-figures", new CustomCommandsForDataManager::delete_figures);
		set_command("add-figure-of-triangulation", new CustomCommandsForDataManager::add_figure_of_triangulation);
		set_command("calculate-betweenness", new CustomCommandsForDataManager::calculate_betweenness);

		set_command("reset-time", new CustomsCommandsForExtraActions::reset_time(elapsed_processor_time_));
		set_command("print-time", new CustomsCommandsForExtraActions::print_time(elapsed_processor_time_));
		set_command("exit", new CustomsCommandsForExtraActions::exit(exit_requested_));
		set_command("echo", new CustomsCommandsForExtraActions::echo());
		set_command("list-virtual-files", new CustomsCommandsForExtraActions::list_virtual_files());
		set_command("upload-virtual-file", new CustomsCommandsForExtraActions::upload_virtual_file());
		set_command("save-virtual-file", new CustomsCommandsForExtraActions::save_virtual_file());
		set_command("print-virtual-file", new CustomsCommandsForExtraActions::print_virtual_file());
		set_command("delete-virtual-files", new CustomsCommandsForExtraActions::delete_virtual_files());
		set_command("setup-loading", new CustomsCommandsForExtraActions::setup_loading());
		set_command("setup-voromqa", new CustomsCommandsForExtraActions::setup_voromqa());
		set_command("explain-command", new CustomsCommandsForExtraActions::explain_command(collection_of_command_documentations_));
		set_command("list-commands", new CustomsCommandsForExtraActions::list_commands(collection_of_command_documentations_));
	}

	virtual ~ScriptExecutionManager()
	{
		SafeUtilitiesForMapOfPointers::clear(commands_for_script_partitioner_);
		SafeUtilitiesForMapOfPointers::clear(commands_for_congregation_of_data_managers_);
		SafeUtilitiesForMapOfPointers::clear(commands_for_data_manager_);
		SafeUtilitiesForMapOfPointers::clear(commands_for_extra_actions_);
	}

	CongregationOfDataManagers& congregation_of_data_managers()
	{
		return congregation_of_data_managers_;
	}

	bool exit_requested() const
	{
		return exit_requested_;
	}

	ScriptRecord execute_script(const std::string& script, const bool exit_on_first_failure)
	{
		on_before_script(script);

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

	void unset_command(const std::string& name)
	{
		SafeUtilitiesForMapOfPointers::erase(commands_for_script_partitioner_, name);
		SafeUtilitiesForMapOfPointers::erase(commands_for_congregation_of_data_managers_, name);
		SafeUtilitiesForMapOfPointers::erase(commands_for_data_manager_, name);
		SafeUtilitiesForMapOfPointers::erase(commands_for_extra_actions_, name);
		collection_of_command_documentations_.delete_documentation(name);
	}

	void set_command(const std::string& name, GenericCommandForScriptPartitioner* command_ptr)
	{
		unset_command(name);
		if(command_ptr!=0)
		{
			SafeUtilitiesForMapOfPointers::set_key_value(commands_for_script_partitioner_, name, command_ptr);
			collection_of_command_documentations_.set_documentation(name, command_ptr->document());
		}
	}

	void set_command(const std::string& name, GenericCommandForCongregationOfDataManagers* command_ptr)
	{
		unset_command(name);
		if(command_ptr!=0)
		{
			SafeUtilitiesForMapOfPointers::set_key_value(commands_for_congregation_of_data_managers_, name, command_ptr);
			collection_of_command_documentations_.set_documentation(name, command_ptr->document());
		}
	}

	void set_command(const std::string& name, GenericCommandForDataManager* command_ptr)
	{
		unset_command(name);
		if(command_ptr!=0)
		{
			SafeUtilitiesForMapOfPointers::set_key_value(commands_for_data_manager_, name, command_ptr);
			collection_of_command_documentations_.set_documentation(name, command_ptr->document());
		}
	}

	void set_command(const std::string& name, GenericCommand* command_ptr)
	{
		unset_command(name);
		if(command_ptr!=0)
		{
			SafeUtilitiesForMapOfPointers::set_key_value(commands_for_extra_actions_, name, command_ptr);
			collection_of_command_documentations_.set_documentation(name, command_ptr->document());
		}
	}

	virtual void on_before_script(const std::string&)
	{
	}

	virtual void on_before_any_command(const CommandInput&)
	{
	}

	virtual void on_after_command_for_script_partitioner(const GenericCommandForScriptPartitioner::CommandRecord&)
	{
	}

	virtual void on_after_command_for_congregation_of_data_managers(const GenericCommandForCongregationOfDataManagers::CommandRecord&)
	{
	}

	virtual void on_after_command_for_data_manager(const GenericCommandForDataManager::CommandRecord&)
	{
	}

	virtual void on_after_command_for_extra_actions(const GenericCommand::CommandRecord&)
	{
	}

	virtual void on_command_not_allowed_for_multiple_data_managers(const CommandInput&)
	{
	}

	virtual void on_no_picked_data_manager_for_command(const CommandInput&)
	{
	}

	virtual void on_unrecognized_command(const std::string&)
	{
	}

	virtual void on_after_any_command(const CommonGenericCommandRecord&)
	{
	}

	virtual void on_after_script(const ScriptRecord&)
	{
	}

private:
	ScriptExecutionManager(const ScriptExecutionManager&);

	const ScriptExecutionManager& operator=(const ScriptExecutionManager&);

	class SafeUtilitiesForMapOfPointers
	{
	public:
		template<typename Map>
		static void clear(Map& map)
		{
			for(typename Map::iterator it=map.begin();it!=map.end();++it)
			{
				if(it->second!=0)
				{
					delete it->second;
				}
			}
			map.clear();
		}

		template<typename Map, typename PointerValue>
		static void set_key_value(Map& map, const std::string& key, PointerValue pointer_value)
		{
			typename Map::iterator it=map.find(key);
			if(it==map.end())
			{
				if(pointer_value!=0)
				{
					map[key]=pointer_value;
				}
			}
			else
			{
				delete it->second;
				if(pointer_value==0)
				{
					map.erase(it);
				}
				else
				{
					it->second=pointer_value;
				}
			}
		}

		template<typename Map>
		static void erase(Map& map, const std::string& key)
		{
			typename Map::iterator it=map.find(key);
			if(it!=map.end())
			{
				delete it->second;
				map.erase(it);
			}
		}
	};

	void execute_script(const std::string& script, const bool exit_on_first_failure, ScriptRecord& script_record)
	{
		exit_requested_=false;

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

			if(exit_requested_)
			{
				return;
			}
		}
	}

	void execute_command(CommandRecord& command_record)
	{
		const std::string& command_name=command_record.command_input.get_command_name();

		if(commands_for_script_partitioner_.count(command_name)==1)
		{
			on_before_any_command(command_record.command_input);
			const GenericCommandForScriptPartitioner::CommandRecord cr=commands_for_script_partitioner_[command_name]->execute(command_record.command_input, script_partitioner_);
			command_record.successful=cr.successful;
			on_after_command_for_script_partitioner(cr);
			on_after_any_command(cr);
		}
		else if(commands_for_congregation_of_data_managers_.count(command_name)==1)
		{
			on_before_any_command(command_record.command_input);
			const GenericCommandForCongregationOfDataManagers::CommandRecord cr=commands_for_congregation_of_data_managers_[command_name]->execute(command_record.command_input, congregation_of_data_managers_);
			command_record.successful=cr.successful;
			on_after_command_for_congregation_of_data_managers(cr);
			on_after_any_command(cr);
		}
		else if(commands_for_data_manager_.count(command_name)==1)
		{
			CongregationOfDataManagers::ObjectQuery query;
			query.picked=true;
			{
				const std::vector<std::string> on_objects=command_record.command_input.get_value_vector_or_default<std::string>("on-objects", std::vector<std::string>());
				if(!on_objects.empty())
				{
					query.names.insert(on_objects.begin(), on_objects.end());
					query.picked=false;
				}
			}
			std::vector<DataManager*> picked_data_managers=congregation_of_data_managers_.get_objects(query);
			if(!picked_data_managers.empty())
			{
				GenericCommandForDataManager* command_for_data_manager=commands_for_data_manager_[command_name];
				if(picked_data_managers.size()==1 || command_for_data_manager->allowed_to_work_on_multiple_data_managers(command_record.command_input))
				{
					for(std::size_t i=0;i<picked_data_managers.size();i++)
					{
						on_before_any_command(command_record.command_input);
						const GenericCommandForDataManager::CommandRecord cr=commands_for_data_manager_[command_name]->execute(command_record.command_input, *picked_data_managers[i]);
						command_record.successful=cr.successful;
						on_after_command_for_data_manager(cr);
						on_after_any_command(cr);
					}
				}
				else
				{
					on_before_any_command(command_record.command_input);
					on_command_not_allowed_for_multiple_data_managers(command_record.command_input);
					on_after_any_command(GenericCommand::CommandRecord(command_record.command_input));
				}
			}
			else
			{
				on_before_any_command(command_record.command_input);
				on_no_picked_data_manager_for_command(command_record.command_input);
				on_after_any_command(GenericCommand::CommandRecord(command_record.command_input));
			}
		}
		else if(commands_for_extra_actions_.count(command_name)==1)
		{
			on_before_any_command(command_record.command_input);
			const GenericCommand::CommandRecord cr=commands_for_extra_actions_[command_name]->execute(command_record.command_input);
			command_record.successful=cr.successful;
			on_after_command_for_extra_actions(cr);
			on_after_any_command(cr);
		}
		else
		{
			on_before_any_command(command_record.command_input);
			on_unrecognized_command(command_name);
			on_after_any_command(GenericCommand::CommandRecord(command_record.command_input));
		}
	}

	std::map<std::string, GenericCommandForScriptPartitioner*> commands_for_script_partitioner_;
	std::map<std::string, GenericCommandForCongregationOfDataManagers*> commands_for_congregation_of_data_managers_;
	std::map<std::string, GenericCommandForDataManager*> commands_for_data_manager_;
	std::map<std::string, GenericCommand*> commands_for_extra_actions_;
	ScriptPartitioner script_partitioner_;
	CongregationOfDataManagers congregation_of_data_managers_;
	CollectionOfCommandDocumentations collection_of_command_documentations_;
	auxiliaries::ElapsedProcessorTime elapsed_processor_time_;
	bool exit_requested_;
};

}

#endif /* SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_ */
