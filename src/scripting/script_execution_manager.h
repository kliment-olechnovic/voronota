#ifndef SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_
#define SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_

#include "generic_command_for_data_manager.h"

#include "operators/all.h"

#include "custom_commands_for_script_partitioner.h"
#include "custom_commands_for_congregation_of_data_managers.h"
#include "custom_commands_for_extra_actions.h"

namespace scripting
{

class ScriptExecutionManager
{
public:
	struct ScriptRecord
	{
		struct CommandRecord
		{
			CommandInput command_input;
			bool successful;

			CommandRecord() : successful(false)
			{
			}
		};

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
		set_command("import", new CustomCommandsForCongregationOfDataManagers::import());
		set_command("pick-objects", new CustomCommandsForCongregationOfDataManagers::pick_objects());
		set_command("pick-more-objects", new CustomCommandsForCongregationOfDataManagers::pick_more_objects());
		set_command("unpick-objects", new CustomCommandsForCongregationOfDataManagers::unpick_objects());
		set_command("show-objects", new CustomCommandsForCongregationOfDataManagers::show_objects());
		set_command("hide-objects", new CustomCommandsForCongregationOfDataManagers::hide_objects());
		set_command("zoom-by-objects", new CustomCommandsForCongregationOfDataManagers::zoom_by_objects());
		set_command("cad-score", new CustomCommandsForCongregationOfDataManagers::cad_score());

		set_command_for_data_manager("add-figure", operators::AddFigure(), true);
		set_command_for_data_manager("add-figure-of-triangulation", operators::AddFigureOfTriangulation(), true);
		set_command_for_data_manager("calculate-betweenness", operators::CalculateBetweenness(), true);
		set_command_for_data_manager("calculate-burial-depth", operators::CalculateBurialDepth(), true);
		set_command_for_data_manager("center-atoms", operators::CenterAtoms(), true);
		set_command_for_data_manager("color-atoms", operators::ColorAtoms(), true);
		set_command_for_data_manager("color-contacts", operators::ColorContacts(), true);
		set_command_for_data_manager("color-figures", operators::ColorFigures(), true);
		set_command_for_data_manager("construct-contacts", operators::ConstructContacts(), true);
		set_command_for_data_manager("construct-triangulation", operators::ConstructTriangulation(), true);
		set_command_for_data_manager("delete-adjuncts-of-atoms", operators::DeleteAdjunctsOfAtoms(), true);
		set_command_for_data_manager("delete-adjuncts-of-contacts", operators::DeleteAdjunctsOfContacts(), true);
		set_command_for_data_manager("delete-figures", operators::DeleteFigures(), true);
		set_command_for_data_manager("delete-selections-of-atoms", operators::DeleteSelectionsOfAtoms(), true);
		set_command_for_data_manager("delete-selections-of-contacts", operators::DeleteSelectionsOfContacts(), true);
		set_command_for_data_manager("delete-tags-of-atoms", operators::DeleteTagsOfAtoms(), true);
		set_command_for_data_manager("delete-tags-of-contacts", operators::DeleteTagsOfContacts(), true);
		set_command_for_data_manager("describe-exposure", operators::DescribeExposure(), true);
		set_command_for_data_manager("export-adjuncts-of-atoms", operators::ExportAdjunctsOfAtoms(), false);
		set_command_for_data_manager("export-atoms-and-contacts", operators::ExportAtomsAndContacts(), false);
		set_command_for_data_manager("export-atoms-as-pymol-cgo", operators::ExportAtomsAsPymolCGO(), false);
		set_command_for_data_manager("export-atoms", operators::ExportAtoms(), false);
		set_command_for_data_manager("export-cartoon-as-pymol-cgo", operators::ExportCartoonAsPymolCGO(), false);
		set_command_for_data_manager("export-contacts-as-pymol-cgo", operators::ExportContactsAsPymolCGO(), false);
		set_command_for_data_manager("export-contacts", operators::ExportContacts(), false);
		set_command_for_data_manager("export-figures-as-pymol-cgo", operators::ExportFiguresAsPymolCGO(), false);
		set_command_for_data_manager("export-selection-of-atoms", operators::ExportSelectionOfAtoms(), false);
		set_command_for_data_manager("export-selection-of-contacts", operators::ExportSelectionOfContacts(), false);
		set_command_for_data_manager("export-triangulation", operators::ExportTriangulation(), false);
		set_command_for_data_manager("export-triangulation-voxels", operators::ExportTriangulationVoxels(), false);
		set_command_for_data_manager("find-connected-components", operators::FindConnectedComponents(), true);
		set_command_for_data_manager("hide-atoms", operators::HideAtoms(), true);
		set_command_for_data_manager("hide-contacts", operators::HideContacts(), true);
		set_command_for_data_manager("hide-figures", operators::HideFigures(), true);
		set_command_for_data_manager("import-adjuncts-of-atoms", operators::ImportAdjunctsOfAtoms(), true);
		set_command_for_data_manager("import-contacts", operators::ImportContacts(), false);
		set_command_for_data_manager("import-selection-of-atoms", operators::ImportSelectionOfAtoms(), true);
		set_command_for_data_manager("import-selection-of-contacts", operators::ImportSelectionOfContacts(), true);
		set_command_for_data_manager("list-figures", operators::ListFigures(), true);
		set_command_for_data_manager("list-selections-of-atoms", operators::ListSelectionsOfAtoms(), true);
		set_command_for_data_manager("list-selections-of-contacts", operators::ListSelectionsOfContacts(), true);
		set_command_for_data_manager("make-drawable-contacts", operators::MakeDrawableContacts(), true);
		set_command_for_data_manager("make-undrawable-contacts", operators::MakeUndrawableContacts(), true);
		set_command_for_data_manager("mark-atoms", operators::MarkAtoms(), true);
		set_command_for_data_manager("mark-contacts", operators::MarkContacts(), true);
		set_command_for_data_manager("move-atoms", operators::MoveAtoms(), true);
		set_command_for_data_manager("print-atoms", operators::PrintAtoms(), true);
		set_command_for_data_manager("print-contacts", operators::PrintContacts(), true);
		set_command_for_data_manager("print-figures", operators::PrintFigures(), true);
		set_command_for_data_manager("print-sequence", operators::PrintSequence(), true);
		set_command_for_data_manager("print-text-description", operators::PrintTextDescription(), true);
		set_command_for_data_manager("print-triangulation", operators::PrintTriangulation(), true);
		set_command_for_data_manager("rename-selection-of-atoms", operators::RenameSelectionOfAtoms(), true);
		set_command_for_data_manager("rename-selection-of-contacts", operators::RenameSelectionOfContacts(), true);
		set_command_for_data_manager("restrict-atoms", operators::RestrictAtoms(), true);
		set_command_for_data_manager("select-atoms-by-triangulation-query", operators::SelectAtomsByTriangulationQuery(), true);
		set_command_for_data_manager("select-atoms", operators::SelectAtoms(), true);
		set_command_for_data_manager("select-contacts", operators::SelectContacts(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-contact-adjuncts", operators::SetAdjunctOfAtomsByContactAdjuncts(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-contact-areas", operators::SetAdjunctOfAtomsByContactAreas(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-residue-pooling", operators::SetAdjunctOfAtomsByResiduePooling(), true);
		set_command_for_data_manager("set-adjunct-of-atoms", operators::SetAdjunctOfAtoms(), true);
		set_command_for_data_manager("set-adjunct-of-contacts", operators::SetAdjunctOfContacts(), true);
		set_command_for_data_manager("set-tag-of-atoms-by-secondary-structure", operators::SetTagOfAtomsBySecondaryStructure(), true);
		set_command_for_data_manager("set-tag-of-atoms", operators::SetTagOfAtoms(), true);
		set_command_for_data_manager("set-tag-of-contacts", operators::SetTagOfContacts(), true);
		set_command_for_data_manager("set-text-description", operators::SetTextDescription(), true);
		set_command_for_data_manager("show-atoms", operators::ShowAtoms(), true);
		set_command_for_data_manager("show-contacts", operators::ShowContacts(), true);
		set_command_for_data_manager("show-figures", operators::ShowFigures(), true);
		set_command_for_data_manager("spectrum-atoms", operators::SpectrumAtoms(), true);
		set_command_for_data_manager("spectrum-contacts", operators::SpectrumContacts(), true);
		set_command_for_data_manager("unmark-atoms", operators::UnmarkAtoms(), true);
		set_command_for_data_manager("unmark-contacts", operators::UnmarkContacts(), true);
		set_command_for_data_manager("voromqa-frustration", operators::VoroMQAFrustration(), true);
		set_command_for_data_manager("voromqa-global", operators::VoroMQAGlobal(), true);
		set_command_for_data_manager("voromqa-local", operators::VoroMQALocal(), true);
		set_command_for_data_manager("voromqa-membrane-place", operators::VoroMQAMembranePlace(), true);
		set_command_for_data_manager("zoom-by-atoms", operators::ZoomByAtoms(), false);
		set_command_for_data_manager("zoom-by-contacts", operators::ZoomByContacts(), false);


		set_command("reset-time", new CustomsCommandsForExtraActions::reset_time(elapsed_processor_time_));
		set_command("print-time", new CustomsCommandsForExtraActions::print_time(elapsed_processor_time_));
		set_command("exit", new CustomsCommandsForExtraActions::exit(exit_requested_));
		set_command("echo", new CustomsCommandsForExtraActions::echo());
		set_command("list-virtual-files", new CustomsCommandsForExtraActions::list_virtual_files());
		set_command("upload-virtual-file", new CustomsCommandsForExtraActions::upload_virtual_file());
		set_command("download-virtual-file", new CustomsCommandsForExtraActions::download_virtual_file());
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

	template<class Operator>
	void set_command_for_data_manager(const std::string& name, const Operator& op, const bool on_multiple)
	{
		unset_command(name);
		GenericCommandForDataManager* command_ptr=new GenericCommandForDataManagerFromOperator<Operator>(op, on_multiple);
		SafeUtilitiesForMapOfPointers::set_key_value(commands_for_data_manager_, name, command_ptr);
		collection_of_command_documentations_.set_documentation(name, command_ptr->document());
	}

	void set_command(const std::string& name, GenericCommandForExtraActions* command_ptr)
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

	virtual void on_after_command_for_script_partitioner(const GenericCommandRecord&, ScriptPartitioner&)
	{
	}

	virtual void on_after_command_for_congregation_of_data_managers(const GenericCommandRecord&, CongregationOfDataManagers&)
	{
	}

	virtual void on_after_command_for_data_manager(const GenericCommandRecord&, DataManager&)
	{
	}

	virtual void on_after_command_for_extra_actions(const GenericCommandRecord&)
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

	virtual void on_after_any_command(const GenericCommandRecord&)
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

			ScriptRecord::CommandRecord script_command_record;

			try
			{
				script_command_record.command_input=CommandInput(command_string);
			}
			catch(const std::exception& e)
			{
				script_record.termination_error=e.what();
				return;
			}

			execute_command(script_command_record);

			script_record.command_records.push_back(script_command_record);

			if(!script_command_record.successful && exit_on_first_failure)
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

	void execute_command(ScriptRecord::CommandRecord& script_command_record)
	{
		const std::string& command_name=script_command_record.command_input.get_command_name();

		if(commands_for_script_partitioner_.count(command_name)==1)
		{
			on_before_any_command(script_command_record.command_input);
			GenericCommandRecord cr(script_command_record.command_input);
			script_command_record.successful=commands_for_script_partitioner_[command_name]->execute(cr, script_partitioner_);
			on_after_command_for_script_partitioner(cr, script_partitioner_);
			on_after_any_command(cr);
		}
		else if(commands_for_congregation_of_data_managers_.count(command_name)==1)
		{
			on_before_any_command(script_command_record.command_input);
			GenericCommandRecord cr(script_command_record.command_input);
			script_command_record.successful=commands_for_congregation_of_data_managers_[command_name]->execute(cr, congregation_of_data_managers_);
			on_after_command_for_congregation_of_data_managers(cr, congregation_of_data_managers_);
			on_after_any_command(cr);
		}
		else if(commands_for_data_manager_.count(command_name)==1)
		{
			CongregationOfDataManagers::ObjectQuery query;
			query.picked=true;
			{
				const std::vector<std::string> on_objects=script_command_record.command_input.get_value_vector_or_default<std::string>("on-objects", std::vector<std::string>());
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
				if(picked_data_managers.size()==1 || command_for_data_manager->allowed_to_work_on_multiple_data_managers(script_command_record.command_input))
				{
					for(std::size_t i=0;i<picked_data_managers.size();i++)
					{
						on_before_any_command(script_command_record.command_input);
						GenericCommandRecord cr(script_command_record.command_input);
						script_command_record.successful=commands_for_data_manager_[command_name]->execute(cr, *picked_data_managers[i]);
						on_after_command_for_data_manager(cr, *picked_data_managers[i]);
						on_after_any_command(cr);
					}
				}
				else
				{
					on_before_any_command(script_command_record.command_input);
					on_command_not_allowed_for_multiple_data_managers(script_command_record.command_input);
					on_after_any_command(GenericCommandRecord(script_command_record.command_input));
				}
			}
			else
			{
				on_before_any_command(script_command_record.command_input);
				on_no_picked_data_manager_for_command(script_command_record.command_input);
				on_after_any_command(GenericCommandRecord(script_command_record.command_input));
			}
		}
		else if(commands_for_extra_actions_.count(command_name)==1)
		{
			on_before_any_command(script_command_record.command_input);
			GenericCommandRecord cr(script_command_record.command_input);
			script_command_record.successful=commands_for_extra_actions_[command_name]->execute(cr);
			on_after_command_for_extra_actions(cr);
			on_after_any_command(cr);
		}
		else
		{
			on_before_any_command(script_command_record.command_input);
			on_unrecognized_command(command_name);
			on_after_any_command(GenericCommandRecord(script_command_record.command_input));
		}
	}

	std::map<std::string, GenericCommandForScriptPartitioner*> commands_for_script_partitioner_;
	std::map<std::string, GenericCommandForCongregationOfDataManagers*> commands_for_congregation_of_data_managers_;
	std::map<std::string, GenericCommandForDataManager*> commands_for_data_manager_;
	std::map<std::string, GenericCommandForExtraActions*> commands_for_extra_actions_;
	ScriptPartitioner script_partitioner_;
	CongregationOfDataManagers congregation_of_data_managers_;
	CollectionOfCommandDocumentations collection_of_command_documentations_;
	auxiliaries::ElapsedProcessorTime elapsed_processor_time_;
	bool exit_requested_;
};

}

#endif /* SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_ */
