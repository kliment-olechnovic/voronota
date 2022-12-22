#ifndef VIEWER_SCRIPT_EXECUTION_MANAGER_H_
#define VIEWER_SCRIPT_EXECUTION_MANAGER_H_

#include "../../../expansion_js/src/duktaper/script_execution_manager.h"

#include "congregations_of_drawers_for_data_managers.h"
#include "console.h"

#include "operators/animate.h"
#include "operators/antialiasing.h"
#include "operators/background.h"
#include "operators/grid.h"
#include "operators/impostoring.h"
#include "operators/mono.h"
#include "operators/multisampling.h"
#include "operators/occlusion.h"
#include "operators/ortho.h"
#include "operators/perspective.h"
#include "operators/resize_window.h"
#include "operators/rotate.h"
#include "operators/screenshot.h"
#include "operators/setup_rendering.h"
#include "operators/stereo.h"
#include "operators/sleep.h"
#include "operators/configure_gui.h"
#include "operators/import_downloaded.h"
#include "operators/export_view.h"
#include "operators/import_view.h"
#include "operators/export_session.h"
#include "operators/import_session.h"
#include "operators/hint_render_area_size.h"

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
		set_command_for_extra_actions("occlusion-none", operators::Occlusion(GUIConfiguration::OCCLUSION_VARIANT_NONE));
		set_command_for_extra_actions("occlusion-noisy", operators::Occlusion(GUIConfiguration::OCCLUSION_VARIANT_NOISY));
		set_command_for_extra_actions("occlusion-smooth", operators::Occlusion(GUIConfiguration::OCCLUSION_VARIANT_SMOOTH));
		set_command_for_extra_actions("antialiasing-none", operators::Antialiasing(GUIConfiguration::ANTIALIASING_VARIANT_NONE));
		set_command_for_extra_actions("antialiasing-fast", operators::Antialiasing(GUIConfiguration::ANTIALIASING_VARIANT_FAST));
		set_command_for_extra_actions("multisampling-none", operators::Multisampling(GUIConfiguration::MULTISAMPLING_VARIANT_NONE));
		set_command_for_extra_actions("multisampling-basic", operators::Multisampling(GUIConfiguration::MULTISAMPLING_VARIANT_BASIC));
		set_command_for_extra_actions("impostoring-none", operators::Impostoring(GUIConfiguration::IMPOSTORING_VARIANT_NONE));
		set_command_for_extra_actions("impostoring-simple", operators::Impostoring(GUIConfiguration::IMPOSTORING_VARIANT_SIMPLE));
		set_command_for_extra_actions("ortho", operators::Ortho());
		set_command_for_extra_actions("perspective", operators::Perspective());
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
		set_command_for_extra_actions("configure-gui-disable-sequence-view", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_DISABLE_SEQUENCE_VIEW));
		set_command_for_extra_actions("configure-gui-enable-sequence-view", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_ENABLE_SEQUENCE_VIEW));
		set_command_for_extra_actions("configure-gui-disable-console", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_DISABLE_CONSOLE));
		set_command_for_extra_actions("configure-gui-enable-console", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_ENABLE_CONSOLE));
		set_command_for_extra_actions("set-initial-atom-representation-to-cartoon", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_SET_INITIAL_MAIN_REPRESENTATION).set_value_of_initial_main_represenation(GUIConfiguration::INITIAL_REPRESENTATION_VARIANT_CARTOON));
		set_command_for_extra_actions("set-initial-atom-representation-to-trace", operators::ConfigureGUI(operators::ConfigureGUI::ACTION_SET_INITIAL_MAIN_REPRESENTATION).set_value_of_initial_main_represenation(GUIConfiguration::INITIAL_REPRESENTATION_VARIANT_TRACE));
		set_command_for_extra_actions("clear", scripting::operators::Mock());
		set_command_for_extra_actions("clear-last", scripting::operators::Mock());
		set_command_for_extra_actions("history", scripting::operators::Mock());
		set_command_for_extra_actions("history-all", scripting::operators::Mock());
		set_command_for_extra_actions("animate-none", operators::Animate(GUIConfiguration::ANIMATION_VARIANT_NONE));
		set_command_for_extra_actions("animate-loop-picked-objects", operators::Animate(GUIConfiguration::ANIMATION_VARIANT_LOOP_PICKED_OBJECTS));
		set_command_for_extra_actions("animate-spin-left", operators::Animate(GUIConfiguration::ANIMATION_VARIANT_SPIN_LEFT));
		set_command_for_extra_actions("animate-spin-right", operators::Animate(GUIConfiguration::ANIMATION_VARIANT_SPIN_RIGHT));
		set_command_for_extra_actions("export-view", operators::ExportView());
		set_command_for_extra_actions("import-view", operators::ImportView());
		set_command_for_extra_actions("hint-render-area-size", operators::HintRenderAreaSize());

		set_command_for_congregation_of_data_managers("fetch", duktaper::operators::Fetch(RemoteImportDownloaderAdaptive::instance()));
		set_command_for_congregation_of_data_managers("fetch-afdb", duktaper::operators::FetchAFDB(RemoteImportDownloaderAdaptive::instance()));
		set_command_for_congregation_of_data_managers("import-url", duktaper::operators::ImportUrl(RemoteImportDownloaderAdaptive::instance()));
		set_command_for_congregation_of_data_managers("import-downloaded", operators::ImportDownloaded());
		set_command_for_congregation_of_data_managers("export-session", operators::ExportSession());
		set_command_for_congregation_of_data_managers("import-session", operators::ImportSession());

#ifdef FOR_WEB
		unset_command("exit");
		unset_command("call-shell");
		unset_command("run-nolb");
		unset_command("run-bff");
#endif

		set_default_aliases();

		forward_documentation();
	}

	~ScriptExecutionManager()
	{
	}

	bool generate_click_script(const uv::DrawingID drawing_id, const int button_code, const bool mod_ctrl_left, const bool mod_shift_left, const bool mod_ctrl_right, const bool mod_shift_right, std::ostringstream& output_script)
	{
		CongregationOfDrawersForDataManagers::DrawerElementID deid=congregation_of_drawers_.resolve_drawing_id(drawing_id);
		if(deid.valid())
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes dm_attributes=congregation_of_data_managers().get_object_attributes(deid.element_id.data_manager_ptr);
			if(mod_ctrl_left || mod_ctrl_right)
			{
				output_script << "ctrl-";
			}
			if(mod_shift_left || mod_shift_right)
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
				if(!mod_ctrl_left && !mod_shift_left && !mod_ctrl_right && !mod_shift_right)
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

	void setup_animation()
	{
		if(GUIConfiguration::instance().animation_variant!=GUIConfiguration::ANIMATION_VARIANT_NONE)
		{
			if(animation_timer.elapsed_miliseconds()>GUIConfiguration::instance().animation_step_miliseconds)
			{
				if(GUIConfiguration::instance().animation_variant==GUIConfiguration::ANIMATION_VARIANT_LOOP_PICKED_OBJECTS)
				{
					congregation_of_data_managers().set_next_picked_object_visible();
					update_console_object_states();
				}
				else if(GUIConfiguration::instance().animation_variant==GUIConfiguration::ANIMATION_VARIANT_SPIN_LEFT)
				{
					uv::ViewerApplication::instance().rotate(glm::vec3(0, 1, 0), 0.01);
				}
				else if(GUIConfiguration::instance().animation_variant==GUIConfiguration::ANIMATION_VARIANT_SPIN_RIGHT)
				{
					uv::ViewerApplication::instance().rotate(glm::vec3(0, 1, 0), -0.01);
				}
				animation_timer.reset();
			}
		}
	}

	void draw(const uv::ShadingMode::Mode shading_mode, const int grid_id)
	{
		const bool prefer_impostoring=(GUIConfiguration::instance().impostoring_variant==GUIConfiguration::IMPOSTORING_VARIANT_SIMPLE);

		DrawerForDataManager::DrawingRequest drawing_request(true, true, true, prefer_impostoring);

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
					drawer->draw(drawing_request, shading_mode);
				}
			}
		}
		else if(uv::ViewerApplication::instance().rendering_mode_is_grid() && GUIConfiguration::instance().grid_variant==GUIConfiguration::GRID_VARIANT_BY_CONCEPT)
		{
			if(grid_id==0)
			{
				drawing_request=DrawerForDataManager::DrawingRequest(true, false, false, prefer_impostoring);
			}
			else if(grid_id==1)
			{
				drawing_request=DrawerForDataManager::DrawingRequest(false, true, false, prefer_impostoring);
			}

			for(std::size_t i=0;i<dms.size();i++)
			{
				DrawerForDataManager* drawer=congregation_of_drawers_.get_object(dms[i]);
				if(drawer!=0)
				{
					drawer->draw(drawing_request, shading_mode);
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
					drawer->draw(drawing_request, shading_mode);
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
				if(data_manager!=0)
				{
					const bool just_added=(ci.added_objects().count(data_manager)>0);
					if(just_added || data_manager->change_indicator().changed())
					{
						DrawerForDataManager* drawer=congregation_of_drawers_.get_object(data_manager);
						if(drawer!=0)
						{
							if(just_added)
							{
								drawer->update();
							}
							else
							{
								drawer->update(data_manager->change_indicator());
							}
							need_to_refresh_frame=true;
						}
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
			uv::ViewerApplication::instance_refresh_frame(true);
		}

		insert_additional_script_if_requested(cr, congregation_of_data_managers);

		if(ci.changed())
		{
			update_console_object_states();
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

			if(ci.changed_atoms_display_states())
			{
				update_console_object_sequence_info(data_manager);
			}
		}

		if(zoom_if_requested(cr))
		{
			need_to_refresh_frame=true;
		}

		if(need_to_refresh_frame)
		{
			uv::ViewerApplication::instance_refresh_frame(false);
		}

		insert_additional_script_if_requested(cr, data_manager);
	}

	void on_after_script_with_output(const scripting::VariantObject&)
	{
		Console::instance().add_output_separator();
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
	}

private:
	void set_default_aliases()
	{
		script_partitioner().set_alias("click-button1-on-unmarked-atom", "pick-more-objects ${1} ; clear-last ; mark-atoms -on-objects ${1} -id ${2} -full-residues ; clear-last ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button1-on-marked-atom", "pick-more-objects ${1} ; clear-last ; unmark-atoms -on-objects ${1} -id ${2} -full-residues ; clear-last ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-unmarked-atom", "print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-marked-atom", "print-atoms -on-objects ${1} -id ${2}");

		script_partitioner().set_alias("click-button1-on-unmarked-contact", "pick-more-objects ${1} ; clear-last ; mark-contacts -on-objects ${1} -id ${2} -full-residues ; clear-last ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button1-on-marked-contact", "pick-more-objects ${1} ; clear-last ; unmark-contacts -on-objects ${1} -id ${2} -full-residues ; clear-last ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-unmarked-contact", "print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-marked-contact", "print-contacts -on-objects ${1} -id ${2}");

		script_partitioner().set_alias("ctrl-click-button1-on-unmarked-atom", "pick-more-objects ${1} ; clear-last ; mark-atoms -on-objects ${1} -id ${2} ; clear-last ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button1-on-marked-atom", "pick-more-objects ${1} ; clear-last ; unmark-atoms -on-objects ${1} -id ${2} ; clear-last ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-unmarked-atom", "print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-marked-atom", "print-atoms -on-objects ${1} -id ${2}");

		script_partitioner().set_alias("ctrl-click-button1-on-unmarked-contact", "pick-more-objects ${1} ; clear-last ; mark-contacts -on-objects ${1} -id ${2} ; clear-last ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button1-on-marked-contact", "pick-more-objects ${1} ; clear-last ; unmark-contacts -on-objects ${1} -id ${2} ; clear-last ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-unmarked-contact", "print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-marked-contact", "print-contacts -on-objects ${1} -id ${2}");

		script_partitioner().set_alias("shift-click-button1-on-unmarked-atom", "pick-more-objects ${1} ; clear-last ; mark-atoms -on-objects ${1} -id ${2} -full-chains ; clear-last ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("shift-click-button1-on-marked-atom", "pick-more-objects ${1} ; clear-last ; unmark-atoms -on-objects ${1} -id ${2} -full-chains ; clear-last ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("shift-click-button2-on-unmarked-atom", "print-atoms -on-objects ${1} -id ${2} -full-residues");
		script_partitioner().set_alias("shift-click-button2-on-marked-atom", "print-atoms -on-objects ${1} -id ${2} -full-residues");

		script_partitioner().set_alias("shift-click-button1-on-unmarked-contact", "pick-more-objects ${1} ; clear-last ; mark-contacts -on-objects ${1} -id ${2} -full-chains ; clear-last ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("shift-click-button1-on-marked-contact", "pick-more-objects ${1} ; clear-last ; unmark-contacts -on-objects ${1} -id ${2} -full-chains ; clear-last ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("shift-click-button2-on-unmarked-contact", "print-contacts -on-objects ${1} -id ${2} -full-residues");
		script_partitioner().set_alias("shift-click-button2-on-marked-contact", "print-contacts -on-objects ${1} -id ${2} -full-residues");

		script_partitioner().set_alias("ctrl-shift-click-button1-on-unmarked-atom", "unmark-atoms -on-objects ${1} ; clear-last ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button1-on-marked-atom", "unmark-atoms -on-objects ${1} ; clear-last ; print-atoms -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-unmarked-atom", "unpick-objects -names ${1} ; hide-objects -names ${1}");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-marked-atom", "unpick-objects -names ${1} ; hide-objects -names ${1}");

		script_partitioner().set_alias("ctrl-shift-click-button1-on-unmarked-contact", "unmark-contacts -on-objects ${1} ; clear-last ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button1-on-marked-contact", "unmark-contacts -on-objects ${1} ; clear-last ; print-contacts -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-unmarked-contact", "unpick-objects -names ${1} ; hide-objects -names ${1}");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-marked-contact", "unpick-objects -names ${1} ; hide-objects -names ${1}");

		script_partitioner().set_alias("click-button1-on-figure", "print-figures -on-objects ${1} -id ${2}");
		script_partitioner().set_alias("click-button2-on-figure", "print-figures -on-objects ${1} -id ${2}");
	}

	void insert_additional_script_if_requested(const GenericCommandRecord& cr, scripting::CongregationOfDataManagers& congregation_of_data_managers)
	{
		if(cr.successful && cr.heterostorage.summaries_of_atoms.count("loaded")==1 && cr.heterostorage.variant_object.values_arrays().count("object_names")==1)
		{
			scripting::CongregationOfDataManagers::ObjectQuery object_query;
			const std::vector<scripting::VariantValue>& object_names_values_array=cr.heterostorage.variant_object.values_arrays().find("object_names")->second;
			for(std::size_t i=0;i<object_names_values_array.size();i++)
			{
				object_query.names.insert(object_names_values_array[i].value_string());
			}
			const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects(object_query);
			if(!objects.empty())
			{
				bool available_contacts=false;
				bool available_tags_het=false;
				bool available_adjuncts_cif_cell=false;
				{
					bool checked=false;
					for(std::size_t i=0;i<objects.size() && !checked;i++)
					{
						const scripting::DataManager& object=(*objects[i]);
						available_contacts=available_contacts || (!object.contacts().empty());
						available_tags_het=available_tags_het || (object.is_any_atom_with_tag("het"));
						available_adjuncts_cif_cell=available_adjuncts_cif_cell || (object.is_any_atom_with_adjunct("cif_cell"));
						checked=available_contacts || available_tags_het || available_adjuncts_cif_cell;
					}
				}

				std::string list_of_names;
				{
					std::ostringstream list_output;
					for(std::set<std::string>::const_iterator it=object_query.names.begin();it!=object_query.names.end();++it)
					{
						list_output << " " << (*it);
					}
					list_of_names=list_output.str();
				}

				std::ostringstream script_output;

				script_output << "set-tag-of-atoms-by-secondary-structure -on-objects " << list_of_names << "\n";
				script_output << "clear-last\n";

				script_output << "zoom-by-objects  -names " << list_of_names << "\n";
				script_output << "clear-last\n";

				script_output << "hide-atoms -on-objects " << list_of_names << "\n";
				script_output << "clear-last\n";

				if(available_contacts)
				{
					script_output << "hide-contacts -on-objects " << list_of_names << "\n";
					script_output << "clear-last\n";
				}

				script_output << "show-atoms [-t! het] -rep ";
				if(GUIConfiguration::instance().initial_main_representation_variant==GUIConfiguration::INITIAL_REPRESENTATION_VARIANT_TRACE)
				{
					script_output << "trace";
				}
				else
				{
					script_output << "cartoon";
				}
				script_output << " -on-objects " << list_of_names << "\n";
				script_output << "clear-last\n";

				script_output << "spectrum-atoms [] -by chain -scheme random -on-objects " << list_of_names << "\n";
				script_output << "clear-last\n";

				if(available_tags_het)
				{
					script_output << "show-atoms [-t het] -rep sticks -on-objects " << list_of_names << "\n";
					script_output << "clear-last\n";

					script_output << "spectrum-atoms [-t het -t! el=C] -by atom-type -on-objects " << list_of_names << "\n";
					script_output << "clear-last\n";
				}

				if(available_adjuncts_cif_cell)
				{
					script_output << "show-atoms [-v cif_cell] -rep balls -on-objects " << list_of_names << "\n";
					script_output << "clear-last\n";

					script_output << "spectrum-atoms [-v cif_cell] -adjunct cif_cell -scheme bcgyr -on-objects " << list_of_names << "\n";
					script_output << "clear-last\n";

					script_output << "color-atoms [-v cif_cell=0] -col 0xFFFFFF -on-objects " << list_of_names << "\n";
					script_output << "clear-last\n";
				}

				script_partitioner().add_pending_sentences_from_string_to_front(script_output.str());
			}
		}
	}

	void insert_additional_script_if_requested(const GenericCommandRecord& cr, scripting::DataManager& data_manager)
	{
		if(cr.successful && cr.heterostorage.summaries_of_atoms.count("reloaded")==1)
		{
			scripting::CongregationOfDataManagers::ObjectAttributes object_attributes=congregation_of_data_managers().get_object_attributes(&data_manager);
			if(object_attributes.valid && !object_attributes.name.empty())
			{
				std::ostringstream script_output;

				script_output << "set-tag-of-atoms-by-secondary-structure -on-objects " << object_attributes.name << "\n";
				script_output << "clear-last\n";

				script_output << "hide-atoms -on-objects " << object_attributes.name << "\n";
				script_output << "clear-last\n";

				if(!data_manager.contacts().empty())
				{
					script_output << "hide-contacts -on-objects " << object_attributes.name << "\n";
					script_output << "clear-last\n";
				}

				script_output << "show-atoms -rep sticks -on-objects " << object_attributes.name << "\n";
				script_output << "clear-last\n";

				script_output << "color-atoms [-t! het] -next-random-color -on-objects " << object_attributes.name << "\n";
				script_output << "clear-last\n";

				if(data_manager.is_any_atom_with_tag("het"))
				{
					script_output << "color-atoms [-t het] -next-random-color -on-objects " << object_attributes.name << "\n";
					script_output << "clear-last\n";
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

	void update_console_object_states()
	{
		std::vector<Console::ObjectsInfo::ObjectState> object_states;
		const std::vector<scripting::DataManager*> data_managers=congregation_of_data_managers().get_objects();
		object_states.reserve(data_managers.size());
		for(std::size_t i=0;i<data_managers.size();i++)
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes object_attributes=congregation_of_data_managers().get_object_attributes(data_managers[i]);
			if(object_attributes.valid)
			{
				Console::ObjectsInfo::ObjectState object_state;
				object_state.name=object_attributes.name;
				object_state.picked=object_attributes.picked;
				object_state.visible=object_attributes.visible;
				object_states.push_back(object_state);
			}
		}
		Console::instance().objects_info().set_object_states(object_states, congregation_of_data_managers().change_indicator().only_changed_objects_picks_or_visibilities());
		for(std::size_t i=0;i<data_managers.size();i++)
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes object_attributes=congregation_of_data_managers().get_object_attributes(data_managers[i]);
			if(object_attributes.valid)
			{
				if(data_managers[i]->change_indicator().changed_atoms_display_states()
						|| (!congregation_of_data_managers().change_indicator().only_changed_objects_picks_or_visibilities() || !Console::instance().objects_info().object_has_details(object_attributes.name)))
				{
					update_console_object_sequence_info(*data_managers[i]);
				}
			}
		}
	}

	void update_console_object_sequence_info(scripting::DataManager& data_manager)
	{
		const scripting::CongregationOfDataManagers::ObjectAttributes object_attributes=congregation_of_data_managers().get_object_attributes(&data_manager);
		if(object_attributes.valid)
		{
			const common::ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& bops=data_manager.primary_structure_info();
			Console::ObjectsInfo::ObjectSequenceInfo sequence_info;
			sequence_info.chains.reserve(bops.chains.size());
			for(std::size_t i=0;i<bops.chains.size();i++)
			{
				Console::ObjectsInfo::ObjectSequenceInfo::ChainInfo chain;
				chain.name=bops.chains[i].name;
				chain.residues.reserve(bops.chains[i].residue_ids.size());
				for(std::size_t j=0;j<bops.chains[i].residue_ids.size();j++)
				{
					const std::size_t residue_id=bops.chains[i].residue_ids[j];
					Console::ObjectsInfo::ObjectSequenceInfo::ResidueInfo residue;
					residue.name=bops.residues[residue_id].short_name;
					if(residue.name.empty())
					{
						residue.name="x";
					}
					residue.num=bops.residues[residue_id].chain_residue_descriptor.resSeq;
					residue.marked=false;
					for(std::size_t e=0;!residue.marked && e<bops.residues[residue_id].atom_ids.size();e++)
					{
						const std::size_t atom_id=bops.residues[residue_id].atom_ids[e];
						if(data_manager.atoms_display_states()[atom_id].marked)
						{
							residue.marked=true;
						}
					}
					if(!bops.residues[residue_id].atom_ids.empty())
					{
						const std::size_t atom_id=bops.residues[residue_id].atom_ids.front();
						if(!data_manager.atoms_display_states()[atom_id].visuals.empty())
						{
							auxiliaries::ColorUtilities::color_to_components(data_manager.atoms_display_states()[atom_id].visuals.front().color, residue.rgb, true);
						}
					}
					chain.residues.push_back(residue);
				}
				{
					std::size_t j=0;
					while(j<chain.residues.size())
					{
						Console::ObjectsInfo::ObjectSequenceInfo::ResidueInfo& residue=chain.residues[j];
						if(residue.name.size()>1)
						{
							residue.num_label=std::to_string(residue.num);
							j++;
						}
						else if(residue.num>=0 && residue.num<=9999 && residue.num%5==1 && (j+3)<chain.residues.size()
								&& chain.residues[j+1].num==(residue.num+1) && chain.residues[j+2].num==(residue.num+2) && chain.residues[j+3].num==(residue.num+3)
								&& chain.residues[j+1].name.size()<2 && chain.residues[j+2].name.size()<2 && chain.residues[j+3].name.size()<2)
						{
							const std::string num_str=std::to_string(residue.num);
							for(std::size_t e=0;e<4;e++)
							{
								chain.residues[j+e].num_label=(e<num_str.size() ? num_str.substr(e, 1) : std::string("."));
							}
							j+=4;
						}
						else if(residue.num>9999 && residue.num<=999999999 && residue.num%10==1 && (j+8)<chain.residues.size()
								&& chain.residues[j+1].num==(residue.num+1) && chain.residues[j+2].num==(residue.num+2) && chain.residues[j+3].num==(residue.num+3) && chain.residues[j+4].num==(residue.num+4) && chain.residues[j+5].num==(residue.num+5) && chain.residues[j+6].num==(residue.num+6) && chain.residues[j+7].num==(residue.num+7) && chain.residues[j+8].num==(residue.num+8)
								&& chain.residues[j+1].name.size()<2 && chain.residues[j+2].name.size()<2 && chain.residues[j+3].name.size()<2 && chain.residues[j+4].name.size()<2 && chain.residues[j+5].name.size()<2 && chain.residues[j+6].name.size()<2 && chain.residues[j+7].name.size()<2 && chain.residues[j+8].name.size()<2)
						{
							const std::string num_str=std::to_string(residue.num);
							for(std::size_t e=0;e<9;e++)
							{
								chain.residues[j+e].num_label=(e<num_str.size() ? num_str.substr(e, 1) : std::string("."));
							}
							j+=5;
						}
						else
						{
							residue.num_label=".";
							j++;
						}
					}
				}
				sequence_info.chains.push_back(chain);
			}
			Console::instance().objects_info().set_object_sequence_info(object_attributes.name, sequence_info);
		}
	}

	CongregationOfDrawersForDataManagers congregation_of_drawers_;
	auxiliaries::ElapsedProcessorTime animation_timer;
};

}

}

#endif /* VIEWER_SCRIPT_EXECUTION_MANAGER_H_ */
