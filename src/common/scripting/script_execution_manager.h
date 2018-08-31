#ifndef COMMON_SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_
#define COMMON_SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_

#include "custom_commands_for_script_partitioner.h"
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

	ScriptExecutionManager() : data_manager_(true)
	{
		map_of_commands_for_script_partitioner_["set-alias"]=new CustomCommandsForScriptPartitioner::set_alias();
		map_of_commands_for_script_partitioner_["unset-aliases"]=new CustomCommandsForScriptPartitioner::unset_aliases();

		map_of_commands_for_data_manager_["load-atoms"]=new CustomCommandsForDataManager::load_atoms();
		map_of_commands_for_data_manager_["restrict-atoms"]=new CustomCommandsForDataManager::restrict_atoms();
		map_of_commands_for_data_manager_["save-atoms"]=new CustomCommandsForDataManager::save_atoms();
		map_of_commands_for_data_manager_["select-atoms"]=new CustomCommandsForDataManager::select_atoms();
		map_of_commands_for_data_manager_["tag-atoms"]=new CustomCommandsForDataManager::tag_atoms();
		map_of_commands_for_data_manager_["untag-atoms"]=new CustomCommandsForDataManager::untag_atoms();
		map_of_commands_for_data_manager_["mark-atoms"]=new CustomCommandsForDataManager::mark_atoms();
		map_of_commands_for_data_manager_["unmark-atoms"]=new CustomCommandsForDataManager::unmark_atoms();
		map_of_commands_for_data_manager_["show-atoms"]=new CustomCommandsForDataManager::show_atoms();
		map_of_commands_for_data_manager_["hide-atoms"]=new CustomCommandsForDataManager::hide_atoms();
		map_of_commands_for_data_manager_["color-atoms"]=new CustomCommandsForDataManager::color_atoms();
		map_of_commands_for_data_manager_["spectrum-atoms"]=new CustomCommandsForDataManager::spectrum_atoms();
		map_of_commands_for_data_manager_["print-atoms"]=new CustomCommandsForDataManager::print_atoms();
		map_of_commands_for_data_manager_["zoom-by-atoms"]=new CustomCommandsForDataManager::zoom_by_atoms();
		map_of_commands_for_data_manager_["list-selections-of-atoms"]=new CustomCommandsForDataManager::list_selections_of_atoms();
		map_of_commands_for_data_manager_["delete-all-selections-of-atoms"]=new CustomCommandsForDataManager::delete_all_selections_of_atoms();
		map_of_commands_for_data_manager_["delete-selections-of-atoms"]=new CustomCommandsForDataManager::delete_selections_of_atoms();
		map_of_commands_for_data_manager_["rename-selection-of-atoms"]=new CustomCommandsForDataManager::rename_selection_of_atoms();
		map_of_commands_for_data_manager_["construct-contacts"]=new CustomCommandsForDataManager::construct_contacts();
		map_of_commands_for_data_manager_["save-contacts"]=new CustomCommandsForDataManager::save_contacts();
		map_of_commands_for_data_manager_["load-contacts"]=new CustomCommandsForDataManager::load_contacts();
		map_of_commands_for_data_manager_["select-contacts"]=new CustomCommandsForDataManager::select_contacts();
		map_of_commands_for_data_manager_["tag-contacts"]=new CustomCommandsForDataManager::tag_contacts();
		map_of_commands_for_data_manager_["untag-contacts"]=new CustomCommandsForDataManager::untag_contacts();
		map_of_commands_for_data_manager_["mark-contacts"]=new CustomCommandsForDataManager::mark_contacts();
		map_of_commands_for_data_manager_["unmark-contacts"]=new CustomCommandsForDataManager::unmark_contacts();
		map_of_commands_for_data_manager_["show-contacts"]=new CustomCommandsForDataManager::show_contacts();
		map_of_commands_for_data_manager_["hide-contacts"]=new CustomCommandsForDataManager::hide_contacts();
		map_of_commands_for_data_manager_["color-contacts"]=new CustomCommandsForDataManager::color_contacts();
		map_of_commands_for_data_manager_["spectrum-contacts"]=new CustomCommandsForDataManager::spectrum_contacts();
		map_of_commands_for_data_manager_["print-contacts"]=new CustomCommandsForDataManager::print_contacts();
		map_of_commands_for_data_manager_["zoom-by-contacts"]=new CustomCommandsForDataManager::zoom_by_contacts();
		map_of_commands_for_data_manager_["write-contacts-as-pymol-cgo"]=new CustomCommandsForDataManager::write_contacts_as_pymol_cgo();
		map_of_commands_for_data_manager_["list-selections-of-contacts"]=new CustomCommandsForDataManager::list_selections_of_contacts();
		map_of_commands_for_data_manager_["delete-all-selections-of-contacts"]=new CustomCommandsForDataManager::delete_all_selections_of_contacts();
		map_of_commands_for_data_manager_["delete-selections-of-contacts"]=new CustomCommandsForDataManager::delete_selections_of_contacts();
		map_of_commands_for_data_manager_["rename-selection-of-contacts"]=new CustomCommandsForDataManager::rename_selection_of_contacts();
		map_of_commands_for_data_manager_["save-atoms-and-contacts"]=new CustomCommandsForDataManager::save_atoms_and_contacts();
		map_of_commands_for_data_manager_["load-atoms-and-contacts"]=new CustomCommandsForDataManager::load_atoms_and_contacts();
	}

	~ScriptExecutionManager()
	{
		delete_map_contents(map_of_commands_for_script_partitioner_);
		delete_map_contents(map_of_commands_for_data_manager_);
	}

	template<typename HandlerForScriptPartitioning, typename HandlerForDataManagment, typename HandlerForUnrecognizedCommandInput>
	ScriptRecord execute_script(
			const std::string& script,
			const bool exit_on_first_failure,
			HandlerForScriptPartitioning& handler_for_script_partitioning,
			HandlerForDataManagment& handler_for_data_management,
			HandlerForUnrecognizedCommandInput& handler_for_unrecognized_command_input)
	{
		ScriptRecord script_record;

		std::list<ScriptPartitioner::Sentence> sentences;

		try
		{
			std::vector<ScriptPartitioner::Sentence> script_sentences=script_partitioner_.partition_script(script);
			sentences.insert(sentences.end(), script_sentences.begin(), script_sentences.end());
		}
		catch(const std::exception& e)
		{
			script_record.termination_error=e.what();
			return script_record;
		}

		std::list<ScriptPartitioner::Sentence>::iterator sentences_it=sentences.begin();

		while(sentences_it!=sentences.end())
		{
			std::vector<ScriptPartitioner::Sentence> subsentences;

			try
			{
				subsentences=script_partitioner_.partition_sentence(*sentences_it);
			}
			catch(const std::exception& e)
			{
				script_record.termination_error=e.what();
				return script_record;
			}

			if(!subsentences.empty())
			{
				sentences_it=sentences.erase(sentences_it);
				sentences_it=sentences.insert(sentences_it, subsentences.begin(), subsentences.end());

				const std::string& command_string=sentences_it->body;

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

				if(command_record.recognized==0 && use_map_with_handler(map_of_commands_for_script_partitioner_, handler_for_script_partitioning, script_partitioner_, command_record))
				{
					command_record.recognized=1;
				}

				if(command_record.recognized==0 && use_map_with_handler(map_of_commands_for_data_manager_, handler_for_data_management, data_manager_, command_record))
				{
					command_record.recognized=2;
				}

				if(command_record.recognized==0)
				{
					handler_for_unrecognized_command_input(command_record.command_input);
				}

				script_record.command_records.push_back(command_record);

				if(!command_record.successful && exit_on_first_failure)
				{
					script_record.termination_error="Terminated on the first failure.";
					return script_record;
				}
			}

			++sentences_it;
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

	template<typename Map, typename Handler, typename Object>
	bool use_map_with_handler(Map& map, Handler& handler, Object& object, CommandRecord& command_record)
	{
		const std::string& command_name=command_record.command_input.get_command_name();
		if(map.count(command_name)==1)
		{
			command_record.successful=handler(map[command_name]->execute(command_record.command_input, object));
			return true;
		}
		return false;
	}

	std::map<std::string, GenericCommandForScriptPartitioner*> map_of_commands_for_script_partitioner_;
	std::map<std::string, GenericCommandForDataManager*> map_of_commands_for_data_manager_;
	ScriptPartitioner script_partitioner_;
	DataManager data_manager_;
};

}

}

#endif /* COMMON_SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_ */
