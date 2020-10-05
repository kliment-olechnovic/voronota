#ifndef VIEWER_SCRIPT_EXECUTION_MANAGER_H_
#define VIEWER_SCRIPT_EXECUTION_MANAGER_H_

#include "../../../expansion_js/src/duktaper/script_execution_manager.h"

#include "congregations_of_drawers_for_data_managers.h"
#include "console.h"

#include "operators/background.h"
#include "operators/fog.h"
#include "operators/grid.h"
#include "operators/mono.h"
#include "operators/ortho.h"
#include "operators/perspective.h"
#include "operators/resize_window.h"
#include "operators/rotate.h"
#include "operators/screenshot.h"
#include "operators/setup_rendering.h"
#include "operators/stereo.h"
#include "operators/sleep.h"
#include "operators/configure_gui.h"

namespace voronota
{

namespace viewer
{

class ScriptExecutionManager : public duktaper::ScriptExecutionManager
{
public:
	ScriptExecutionManager()
	{
		set_command_for_extra_actions("resize-window", operators::ResizeWindow());
		set_command_for_extra_actions("background", operators::Background());
		set_command_for_extra_actions("mono", operators::Mono());
		set_command_for_extra_actions("stereo", operators::Stereo());
		set_command_for_extra_actions("grid-by-object", operators::Grid(GUIConfiguration::GRID_VARIANT_BY_OBJECT));
		set_command_for_extra_actions("grid-by-concept", operators::Grid(GUIConfiguration::GRID_VARIANT_BY_CONCEPT));
		set_command_for_extra_actions("ortho", operators::Ortho());
		set_command_for_extra_actions("perspective", operators::Perspective());
		set_command_for_extra_actions("fog", operators::Fog());
		set_command_for_extra_actions("rotate", operators::Rotate());
		set_command_for_extra_actions("screenshot", operators::Screenshot());
		set_command_for_extra_actions("setup-rendering", operators::SetupRendering());
		set_command_for_extra_actions("sleep", operators::Sleep());
		set_command_for_extra_actions("configure-gui-push", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_PUSH));
		set_command_for_extra_actions("configure-gui-pop", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_POP));
		set_command_for_extra_actions("configure-gui-disable-widgets", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_DISABLE_WIDGETS));
		set_command_for_extra_actions("configure-gui-enable-widgets", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_ENABLE_WIDGETS));
		set_command_for_extra_actions("configure-gui-disable-waiting-indicator", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_DISABLE_WAITING_INDICATOR));
		set_command_for_extra_actions("configure-gui-enable-waiting-indicator", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_ENABLE_WAITING_INDICATOR));
		set_command_for_extra_actions("configure-gui-json-write-level-0", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_SET_JSON_WRITING_LEVEL).set_value_of_json_writing_level(0));
		set_command_for_extra_actions("configure-gui-json-write-level-1", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_SET_JSON_WRITING_LEVEL).set_value_of_json_writing_level(1));
		set_command_for_extra_actions("configure-gui-json-write-level-2", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_SET_JSON_WRITING_LEVEL).set_value_of_json_writing_level(2));
		set_command_for_extra_actions("configure-gui-json-write-level-3", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_SET_JSON_WRITING_LEVEL).set_value_of_json_writing_level(3));
		set_command_for_extra_actions("configure-gui-json-write-level-4", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_SET_JSON_WRITING_LEVEL).set_value_of_json_writing_level(4));
		set_command_for_extra_actions("configure-gui-json-write-level-5", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_SET_JSON_WRITING_LEVEL).set_value_of_json_writing_level(5));
		set_command_for_extra_actions("configure-gui-json-write-level-6", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_SET_JSON_WRITING_LEVEL).set_value_of_json_writing_level(6));
		set_command_for_extra_actions("clear", scripting::operators::Mock());
		set_command_for_extra_actions("clear-last", scripting::operators::Mock());
		set_command_for_extra_actions("history", scripting::operators::Mock());
		set_command_for_extra_actions("history-all", scripting::operators::Mock());

		set_default_aliases();

		forward_documentation();
	}

	~ScriptExecutionManager()
	{
	}

	bool generate_click_script(const uv::DrawingID drawing_id, const int button_code, const bool mod_ctrl, const bool mod_shift, std::ostringstream& output_script)
	{
		CongregationOfDrawersForDataManagers::DrawerElementID deid=congregation_of_drawers_.resolve_drawing_id(drawing_id);
		if(deid.valid())
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes dm_attributes=congregation_of_data_managers().get_object_attributes(deid.element_id.data_manager_ptr);
			if(mod_ctrl)
			{
				output_script << "ctrl-";
			}
			if(mod_shift)
			{
				output_script << "shift-";
			}
			output_script << "click-button" << button_code << "-on-";
			if(deid.element_id.valid_atom_id())
			{
				if(deid.element_id.data_manager_ptr->atoms_display_states()[deid.element_id.atom_id].marked)
				{
					output_script << "marked-";
				}
				else
				{
					output_script << "unmarked-";
				}
				output_script << "atom " << dm_attributes.name << " " << deid.element_id.atom_id << "\n";
				return true;
			}
			else if(deid.element_id.valid_contact_id())
			{
				if(deid.element_id.data_manager_ptr->contacts_display_states()[deid.element_id.contact_id].marked)
				{
					output_script << "marked-";
				}
				else
				{
					output_script << "unmarked-";
				}
				output_script << "contact " << dm_attributes.name << " " << deid.element_id.contact_id << "\n";
				return true;
			}
			else if(deid.element_id.valid_figure_id())
			{
				if(!mod_ctrl && !mod_shift)
				{
					output_script << "figure " << dm_attributes.name << " " << deid.element_id.figure_id << "\n";
					return true;
				}
			}
		}
		return false;
	}

	bool generate_click_label(const uv::DrawingID drawing_id, std::ostringstream& output_label)
	{
		CongregationOfDrawersForDataManagers::DrawerElementID deid=congregation_of_drawers_.resolve_drawing_id(drawing_id);
		if(deid.valid())
		{
			if(deid.element_id.valid_atom_id())
			{
				common::ChainResidueAtomDescriptor crad=deid.element_id.data_manager_ptr->atoms()[deid.element_id.atom_id].crad;
				output_label << crad.chainID << ",";
				if(crad.resSeq!=crad.null_num())
				{
					output_label << crad.resSeq;
				}
				else
				{
					output_label << "?";
				}
				output_label << "," << crad.resName << "," << crad.name;
				return true;
			}
			else if(deid.element_id.valid_contact_id())
			{
				common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(
						deid.element_id.data_manager_ptr->atoms(),
						deid.element_id.data_manager_ptr->contacts()[deid.element_id.contact_id]);

				output_label << crads.a.chainID << ",";
				if(crads.a.resSeq!=crads.a.null_num())
				{
					output_label << crads.a.resSeq;
				}
				else
				{
					output_label << "?";
				}
				output_label << "," << crads.a.resName << "," << crads.a.name << " | ";

				if(deid.element_id.data_manager_ptr->contacts()[deid.element_id.contact_id].solvent())
				{
					output_label << "solvent";
				}
				else
				{
					output_label << crads.b.chainID << ",";
					if(crads.b.resSeq!=crads.b.null_num())
					{
						output_label << crads.b.resSeq;
					}
					else
					{
						output_label << "?";
					}
					output_label << "," << crads.b.resName << "," << crads.b.name;
				}

				return true;
			}
			else if(deid.element_id.valid_figure_id())
			{
				const std::vector<std::string>& name=deid.element_id.data_manager_ptr->figures()[deid.element_id.figure_id].name.subnames();
				for(std::size_t i=0;i<name.size();i++)
				{
					output_label << name[i] << ((i+1)<name.size() ? "." : "");
				}
				return true;
			}
		}
		return false;
	}

	void setup_grid_parameters()
	{
		uv::ViewerApplication::instance().set_grid_size(1);
		if(uv::ViewerApplication::instance().rendering_mode_is_grid())
		{
			if(GUIConfiguration::instance().grid_variant==GUIConfiguration::GRID_VARIANT_BY_OBJECT)
			{
				uv::ViewerApplication::instance().set_grid_size(static_cast<int>(congregation_of_data_managers().count_objects(false, true)));
			}
			else if(GUIConfiguration::instance().grid_variant==GUIConfiguration::GRID_VARIANT_BY_CONCEPT)
			{
				uv::ViewerApplication::instance().set_grid_size(2);
			}
		}
	}

	void draw(const bool with_instancing, const int grid_id)
	{
		DrawerForDataManager::DrawingRequest drawing_request(true);

		scripting::CongregationOfDataManagers::ObjectQuery query;
		query.visible=true;
		std::vector<scripting::DataManager*> dms=congregation_of_data_managers().get_objects(query);

		if(uv::ViewerApplication::instance().rendering_mode_is_grid() && GUIConfiguration::instance().grid_variant==GUIConfiguration::GRID_VARIANT_BY_OBJECT)
		{
			const std::size_t grid_uid=static_cast<std::size_t>(grid_id);
			if(grid_uid<dms.size())
			{
				DrawerForDataManager* drawer=congregation_of_drawers_.get_object(dms[grid_uid]);
				if(drawer!=0)
				{
					drawer->draw(drawing_request, with_instancing);
				}
			}
		}
		else if(uv::ViewerApplication::instance().rendering_mode_is_grid() && GUIConfiguration::instance().grid_variant==GUIConfiguration::GRID_VARIANT_BY_CONCEPT)
		{
			if(grid_id==0)
			{
				drawing_request=DrawerForDataManager::DrawingRequest(true, false, false);
			}
			else if(grid_id==1)
			{
				drawing_request=DrawerForDataManager::DrawingRequest(false, true, false);
			}

			for(std::size_t i=0;i<dms.size();i++)
			{
				DrawerForDataManager* drawer=congregation_of_drawers_.get_object(dms[i]);
				if(drawer!=0)
				{
					drawer->draw(drawing_request, with_instancing);
				}
			}
		}
		else
		{
			for(std::size_t i=0;i<dms.size();i++)
			{
				DrawerForDataManager* drawer=congregation_of_drawers_.get_object(dms[i]);
				if(drawer!=0)
				{
					drawer->draw(drawing_request, with_instancing);
				}
			}
		}
	}

protected:
	void on_after_command_for_congregation_of_data_managers(const GenericCommandRecord& cr, scripting::CongregationOfDataManagers& congregation_of_data_managers)
	{
		const scripting::CongregationOfDataManagers::ChangeIndicator& ci=congregation_of_data_managers.change_indicator();

		for(std::set<scripting::DataManager*>::const_iterator it=ci.added_objects().begin();it!=ci.added_objects().end();++it)
		{
			congregation_of_drawers_.add_object(*(*it));
		}

		for(std::set<scripting::DataManager*>::const_iterator it=ci.deleted_objects().begin();it!=ci.deleted_objects().end();++it)
		{
			congregation_of_drawers_.delete_object(*it);
		}

		bool need_to_refresh_frame=false;

		{
			const std::vector<scripting::DataManager*> data_managers=congregation_of_data_managers.get_objects();
			for(std::size_t i=0;i<data_managers.size();i++)
			{
				scripting::DataManager* data_manager=data_managers[i];
				if(data_manager!=0 && data_manager->change_indicator().changed())
				{
					DrawerForDataManager* drawer=congregation_of_drawers_.get_object(data_manager);
					if(drawer!=0)
					{
						drawer->update(data_manager->change_indicator());
						need_to_refresh_frame=true;
					}
				}
			}
		}

		if(zoom_if_requested(cr))
		{
			need_to_refresh_frame=true;
		}

		if(need_to_refresh_frame)
		{
			uv::ViewerApplication::instance_refresh_frame();
		}

		insert_additional_script_if_requested(cr);

		if(ci.changed())
		{
			std::vector<Console::ObjectState> object_states;
			const std::vector<scripting::DataManager*> data_managers=congregation_of_data_managers.get_objects();
			object_states.reserve(data_managers.size());
			for(std::size_t i=0;i<data_managers.size();i++)
			{
				const scripting::CongregationOfDataManagers::ObjectAttributes object_attributes=congregation_of_data_managers.get_object_attributes(data_managers[i]);
				if(object_attributes.valid)
				{
					Console::ObjectState object_state;
					object_state.name=object_attributes.name;
					object_state.picked=object_attributes.picked;
					object_state.visible=object_attributes.visible;
					object_states.push_back(object_state);
				}
			}
			Console::instance().set_object_states(object_states);
		}
	}

	void on_after_command_for_data_manager(const GenericCommandRecord& cr, scripting::DataManager& data_manager)
	{
		scripting::ScriptExecutionManagerWithVariantOutput::on_after_command_for_data_manager(cr, data_manager);

		const scripting::DataManager::ChangeIndicator ci=data_manager.change_indicator();

		bool need_to_refresh_frame=false;

		if(ci.changed())
		{
			DrawerForDataManager* drawer=congregation_of_drawers_.get_object(&data_manager);
			if(drawer!=0)
			{
				drawer->update(ci);
				need_to_refresh_frame=true;
			}
		}

		if(zoom_if_requested(cr))
		{
			need_to_refresh_frame=true;
		}

		if(need_to_refresh_frame)
		{
			uv::ViewerApplication::instance_refresh_frame();
		}
	}

	void on_after_script_with_output(const scripting::VariantObject&)
	{
		scripting::JSONWriter::Configuration json_writing_configuration(GUIConfiguration::instance().json_writing_level);
		if(last_output().objects_arrays().count("results")>0)
		{
			const std::vector<scripting::VariantObject>& results=last_output().objects_arrays().find("results")->second;
			for(std::size_t i=0;i<results.size();i++)
			{
				if(results[i].values().count("command_name")>0)
				{
					scripting::VariantObject result=results[i];
					const std::string command_name=result.value("command_name").value_as_string();
					if(command_name=="clear")
					{
						Console::instance().clear_outputs();
					}
					else if(command_name=="clear-last")
					{
						Console::instance().clear_last_output();
					}
					else if(command_name=="history")
					{
						Console::instance().add_history_output(20);
					}
					else if(command_name=="history-all")
					{
						Console::instance().add_history_output(0);
					}
					else
					{
						const bool success=result.value("success").value_as_string()=="true";
						result.erase("command_name");
						result.erase("success");
						if(success)
						{
							Console::instance().add_output(scripting::JSONWriter::write(json_writing_configuration, result), 0.5f, 1.0f, 1.0f);
						}
						else
						{
							Console::instance().add_output(scripting::JSONWriter::write(json_writing_configuration, result), 1.0f, 0.5f, 0.5f);
						}
					}
				}
				else
				{
					Console::instance().add_output(scripting::JSONWriter::write(json_writing_configuration, results[i]), 1.0f, 1.0f, 0.0f);
				}
			}
		}
		else
		{
			Console::instance().add_output(scripting::JSONWriter::write(json_writing_configuration, last_output()), 1.0f, 1.0f, 1.0f);
		}
		Console::instance().add_output_separator();
	}

private:
	void set_default_aliases()
	{
		script_partitioner().set_alias("click-button1-on-unmarked-atom", "pick-more-objects ${1} ; mark-atoms -on-objects ${1} -id ${2} ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button1-on-marked-atom", "pick-more-objects ${1} ; unmark-atoms -on-objects ${1} -id ${2} ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-unmarked-atom", "print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-marked-atom", "print-atoms -on-objects ${1} -id ${2}");

		script_partitioner().set_alias("click-button1-on-unmarked-contact", "pick-more-objects ${1} ; mark-contacts -on-objects ${1} -id ${2} ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button1-on-marked-contact", "pick-more-objects ${1} ; unmark-contacts -on-objects ${1} -id ${2} ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-unmarked-contact", "print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-marked-contact", "print-contacts -on-objects ${1} -id ${2}");

		script_partitioner().set_alias("ctrl-click-button1-on-unmarked-atom", "pick-more-objects ${1} ; unmark-atoms -on-objects ${1} ; mark-atoms -on-objects ${1} -id ${2} ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button1-on-marked-atom", "pick-more-objects ${1} ; unmark-atoms -on-objects ${1} -id ${2} ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-unmarked-atom", "print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-marked-atom", "print-atoms -on-objects ${1} -id ${2}");

		script_partitioner().set_alias("ctrl-click-button1-on-unmarked-contact", "pick-more-objects ${1} ; unmark-contacts -on-objects ${1} ; mark-contacts -on-objects ${1} -id ${2} ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button1-on-marked-contact", "pick-more-objects ${1} ; unmark-contacts -on-objects ${1} -id ${2} ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-unmarked-contact", "print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-marked-contact", "print-contacts -on-objects ${1} -id ${2}");

		script_partitioner().set_alias("shift-click-button1-on-unmarked-atom", "pick-more-objects ${1} ; mark-atoms -on-objects ${1} -id ${2} -full-residues ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("shift-click-button1-on-marked-atom", "pick-more-objects ${1} ; unmark-atoms -on-objects ${1} -id ${2} -full-residues ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("shift-click-button2-on-unmarked-atom", "print-atoms -on-objects ${1} -id ${2} -full-residues");
		script_partitioner().set_alias("shift-click-button2-on-marked-atom", "print-atoms -on-objects ${1} -id ${2} -full-residues");

		script_partitioner().set_alias("shift-click-button1-on-unmarked-contact", "pick-more-objects ${1} ; mark-contacts -on-objects ${1} -id ${2} -full-residues ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("shift-click-button1-on-marked-contact", "pick-more-objects ${1} ; unmark-contacts -on-objects ${1} -id ${2} -full-residues ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("shift-click-button2-on-unmarked-contact", "print-contacts -on-objects ${1} -id ${2} -full-residues");
		script_partitioner().set_alias("shift-click-button2-on-marked-contact", "print-contacts -on-objects ${1} -id ${2} -full-residues");

		script_partitioner().set_alias("ctrl-shift-click-button1-on-unmarked-atom", "pick-more-objects ${1} ; unmark-atoms -on-objects ${1} ; mark-atoms -on-objects ${1} -id ${2} -full-residues ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button1-on-marked-atom", "pick-more-objects ${1} ; unmark-atoms -on-objects ${1} -id ${2} -full-residues ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-unmarked-atom", "print-atoms -on-objects ${1} -id ${2} -full-residues");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-marked-atom", "print-atoms -on-objects ${1} -id ${2} -full-residues");

		script_partitioner().set_alias("ctrl-shift-click-button1-on-unmarked-contact", "pick-more-objects ${1} ; unmark-contacts -on-objects ${1} ; mark-contacts -on-objects ${1} -id ${2} -full-residues ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button1-on-marked-contact", "pick-more-objects ${1} ; unmark-contacts -on-objects ${1} -id ${2} -full-residues ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-unmarked-contact", "print-contacts -on-objects ${1} -id ${2} -full-residues");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-marked-contact", "print-contacts -on-objects ${1} -id ${2} -full-residues");

		script_partitioner().set_alias("click-button1-on-figure", "print-figures -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-figure", "print-figures -on-objects ${1} -id ${2}");
	}

	template<typename CommandRecord>
	void insert_additional_script_if_requested(const CommandRecord& cr)
	{
		if(cr.successful && cr.heterostorage.summaries_of_atoms.count("loaded")==1)
		{
			scripting::CongregationOfDataManagers::ObjectQuery object_query;
			object_query.picked=true;
			const std::vector<scripting::DataManager*> objects=congregation_of_data_managers().get_objects(object_query);
			if(!objects.empty())
			{
				bool available_contacts=false;
				bool available_tags_het=false;
				{
					bool checked=false;
					for(std::size_t i=0;i<objects.size() && !checked;i++)
					{
						const scripting::DataManager& object=(*objects[i]);
						available_contacts=available_contacts || (!object.contacts().empty());
						available_tags_het=available_tags_het || (object.is_any_atom_with_tag("het"));
						checked=available_contacts || available_tags_het;
					}
				}
				std::ostringstream script_output;
				script_output << "set-tag-of-atoms-by-secondary-structure\n";
				script_output << "zoom-by-objects -picked\n";
				script_output << "hide-atoms\n";
				if(available_contacts)
				{
					script_output << "hide-contacts\n";
				}
				script_output << "show-atoms [-t! het] -rep cartoon\n";
				script_output << "color-atoms [-t! het] -next-random-color\n";
				if(available_tags_het)
				{
					script_output << "show-atoms [-t het] -rep sticks\n";
					script_output << "color-atoms [-t het] -next-random-color\n";
				}
				script_partitioner().add_pending_sentences_from_string_to_front(script_output.str());
			}
		}
	}

	template<typename CommandRecord>
	bool zoom_if_requested(const CommandRecord& cr)
	{
		if(cr.successful && cr.heterostorage.summaries_of_atoms.count("zoomed")==1)
		{
			const scripting::BoundingBox& box=cr.heterostorage.summaries_of_atoms.find("zoomed")->second.bounding_box;
			if(box.filled)
			{
				uv::ZoomCalculator zoom_calculator;
				zoom_calculator.update(box.p_min.x, box.p_min.y, box.p_min.z);
				zoom_calculator.update(box.p_max.x, box.p_max.y, box.p_max.z);
				uv::ViewerApplication::instance().zoom(zoom_calculator);
				return true;
			}
		}
		return false;
	}

	void forward_documentation() const
	{
		std::map<std::string, std::string> reference;

		const std::vector<std::string> names=collection_of_command_documentations().get_all_names();

		for(std::size_t i=0;i<names.size();i++)
		{
			const scripting::CommandDocumentation doc=collection_of_command_documentations().get_documentation(names[i]);

			std::size_t max_option_name_length=1;
			std::size_t max_option_data_type_length=1;
			std::size_t max_option_description_length=1;
			for(std::size_t j=0;j<doc.get_option_descriptions().size();j++)
			{
				const scripting::CommandDocumentation::OptionDescription& od=doc.get_option_descriptions()[j];
				max_option_name_length=std::max(max_option_name_length, od.name.size());
				max_option_data_type_length=std::max(max_option_data_type_length, od.data_type_string().size());
				max_option_description_length=std::max(max_option_description_length, od.description.size());
			}
			max_option_name_length+=4;
			max_option_data_type_length+=4;
			max_option_description_length+=4;

			std::ostringstream output;
			for(std::size_t j=0;j<doc.get_option_descriptions().size();j++)
			{
				const scripting::CommandDocumentation::OptionDescription& od=doc.get_option_descriptions()[j];
				output << "    --";
				output << od.name;
				output << std::string(max_option_name_length-od.name.size(), ' ');
				output << (od.required ? " * " : "   ");
				output << od.data_type_string();
				output << std::string(max_option_data_type_length-od.data_type_string().size(), ' ');
				output << od.description;
				if(!od.default_value.empty())
				{
					output << std::string(max_option_description_length-od.description.size(), ' ');
					output << od.default_value;
				}
				output << "\n";
			}

			reference[names[i]]=output.str();
		}

		Console::instance().set_documentation(reference);
	}

	CongregationOfDrawersForDataManagers congregation_of_drawers_;
};

}

}

#endif /* VIEWER_SCRIPT_EXECUTION_MANAGER_H_ */
