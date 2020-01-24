#ifndef VIEWER_SCRIPT_EXECUTION_MANAGER_H_
#define VIEWER_SCRIPT_EXECUTION_MANAGER_H_

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"

#include "congregations_of_drawers_for_data_managers.h"
#include "operators_all.h"
#include "environment.h"
#include "runtime_parameters.h"

namespace voronota
{

namespace viewer
{

class ScriptExecutionManager : public scripting::ScriptExecutionManagerWithVariantOutput
{
public:
	ScriptExecutionManager()
	{
		set_command_for_extra_actions("resize-window", operators::ResizeWindow());
		set_command_for_extra_actions("background", operators::Background());
		set_command_for_extra_actions("mono", operators::Mono());
		set_command_for_extra_actions("stereo", operators::Stereo());
		set_command_for_extra_actions("grid-by-object", operators::Grid(RuntimeParameters::GRID_VARIANT_BY_OBJECT));
		set_command_for_extra_actions("grid-by-concept", operators::Grid(RuntimeParameters::GRID_VARIANT_BY_CONCEPT));
		set_command_for_extra_actions("ortho", operators::Ortho());
		set_command_for_extra_actions("perspective", operators::Perspective());
		set_command_for_extra_actions("fog", operators::Fog());
		set_command_for_extra_actions("rotate", operators::Rotate());
		set_command_for_extra_actions("screenshot", operators::Screenshot());
		set_command_for_extra_actions("setup-rendering", operators::SetupRendering());

		set_default_aliases();
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
			if(RuntimeParameters::instance().grid_variant==RuntimeParameters::GRID_VARIANT_BY_OBJECT)
			{
				uv::ViewerApplication::instance().set_grid_size(static_cast<int>(congregation_of_data_managers().count_objects(false, true)));
			}
			else if(RuntimeParameters::instance().grid_variant==RuntimeParameters::GRID_VARIANT_BY_CONCEPT)
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

		if(uv::ViewerApplication::instance().rendering_mode_is_grid() && RuntimeParameters::instance().grid_variant==RuntimeParameters::GRID_VARIANT_BY_OBJECT)
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
		else if(uv::ViewerApplication::instance().rendering_mode_is_grid() && RuntimeParameters::instance().grid_variant==RuntimeParameters::GRID_VARIANT_BY_CONCEPT)
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

	bool generate_text_description(const int grid_id, std::ostringstream& output_text)
	{
		scripting::CongregationOfDataManagers::ObjectQuery query;
		query.visible=true;
		std::vector<scripting::DataManager*> dms=congregation_of_data_managers().get_objects(query);

		bool generated=false;

		if(uv::ViewerApplication::instance().rendering_mode_is_grid() && RuntimeParameters::instance().grid_variant==RuntimeParameters::GRID_VARIANT_BY_OBJECT)
		{
			const std::size_t grid_uid=static_cast<std::size_t>(grid_id);
			if(grid_uid<dms.size())
			{
				if(!dms[grid_uid]->text_description().empty())
				{
					output_text << congregation_of_data_managers().get_object_attributes(dms[grid_uid]).name << ": ";
					output_text << dms[grid_uid]->text_description();
					generated=true;
				}
			}
		}
		else
		{
			for(std::size_t i=0;i<dms.size();i++)
			{
				if(!dms[i]->text_description().empty())
				{
					output_text << congregation_of_data_managers().get_object_attributes(dms[i]).name << ": ";
					output_text << dms[i]->text_description() << ((i+1<dms.size()) ? "\n" : "");
					generated=true;
				}
			}
		}

		return generated;
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
					}
				}
			}
		}

		insert_additional_script_if_requested(cr);

		zoom_if_requested(cr);
	}

	void on_after_command_for_data_manager(const GenericCommandRecord& cr, scripting::DataManager& data_manager)
	{
		scripting::ScriptExecutionManagerWithVariantOutput::on_after_command_for_data_manager(cr, data_manager);

		const scripting::DataManager::ChangeIndicator ci=data_manager.change_indicator();

		if(ci.changed())
		{
			DrawerForDataManager* drawer=congregation_of_drawers_.get_object(&data_manager);
			if(drawer!=0)
			{
				drawer->update(ci);
			}
		}

		zoom_if_requested(cr);
	}

	void on_after_script_with_output(const scripting::VariantObject&)
	{
		Environment::print_log(last_output());
	}

private:
	void set_default_aliases()
	{
		script_partitioner().set_alias("click-button1-on-unmarked-atom", "pick-objects ${1} ; mark-atoms -id ${2} ; print-atoms -id ${2}");
		script_partitioner().set_alias("click-button1-on-marked-atom", "pick-objects ${1} ; unmark-atoms -id ${2} ; print-atoms -id ${2}");
		script_partitioner().set_alias("click-button2-on-unmarked-atom", "pick-objects ${1} ; print-atoms -id ${2}");
		script_partitioner().set_alias("click-button2-on-marked-atom", "pick-objects ${1} ; print-atoms -id ${2}");

		script_partitioner().set_alias("click-button1-on-unmarked-contact", "pick-objects ${1} ; mark-contacts -id ${2} ; print-contacts -id ${2}");
		script_partitioner().set_alias("click-button1-on-marked-contact", "pick-objects ${1} ; unmark-contacts -id ${2} ; print-contacts -id ${2}");
		script_partitioner().set_alias("click-button2-on-unmarked-contact", "pick-objects ${1} ; print-contacts -id ${2}");
		script_partitioner().set_alias("click-button2-on-marked-contact", "pick-objects ${1} ; print-contacts -id ${2}");

		script_partitioner().set_alias("ctrl-click-button1-on-unmarked-atom", "pick-objects ${1} ; unmark-atoms ; mark-atoms -id ${2} ; print-atoms -id ${2}");
		script_partitioner().set_alias("ctrl-click-button1-on-marked-atom", "pick-objects ${1} ; unmark-atoms -id ${2} ; print-atoms -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-unmarked-atom", "pick-objects ${1} ; unmark-atoms ; print-atoms -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-marked-atom", "pick-objects ${1} ; unmark-atoms ; print-atoms -id ${2}");

		script_partitioner().set_alias("ctrl-click-button1-on-unmarked-contact", "pick-objects ${1} ; unmark-contacts ; mark-contacts -id ${2} ; print-contacts -id ${2}");
		script_partitioner().set_alias("ctrl-click-button1-on-marked-contact", "pick-objects ${1} ; unmark-contacts -id ${2} ; print-contacts -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-unmarked-contact", "pick-objects ${1} ; unmark-contacts ; print-contacts -id ${2}");
		script_partitioner().set_alias("ctrl-click-button2-on-marked-contact", "pick-objects ${1} ; unmark-contacts ; print-contacts -id ${2}");

		script_partitioner().set_alias("shift-click-button1-on-unmarked-atom", "pick-objects ${1} ; mark-atoms -id ${2} -full-residues ; print-atoms -id ${2}");
		script_partitioner().set_alias("shift-click-button1-on-marked-atom", "pick-objects ${1} ; unmark-atoms -id ${2} -full-residues ; print-atoms -id ${2}");
		script_partitioner().set_alias("shift-click-button2-on-unmarked-atom", "pick-objects ${1} ; print-atoms -id ${2} -full-residues");
		script_partitioner().set_alias("shift-click-button2-on-marked-atom", "pick-objects ${1} ; print-atoms -id ${2} -full-residues");

		script_partitioner().set_alias("shift-click-button1-on-unmarked-contact", "pick-objects ${1} ; mark-contacts -id ${2} -full-residues ; print-contacts -id ${2}");
		script_partitioner().set_alias("shift-click-button1-on-marked-contact", "pick-objects ${1} ; unmark-contacts -id ${2} -full-residues ; print-contacts -id ${2}");
		script_partitioner().set_alias("shift-click-button2-on-unmarked-contact", "pick-objects ${1} ; print-contacts -id ${2} -full-residues");
		script_partitioner().set_alias("shift-click-button2-on-marked-contact", "pick-objects ${1} ; print-contacts -id ${2} -full-residues");

		script_partitioner().set_alias("ctrl-shift-click-button1-on-unmarked-atom", "pick-objects ${1} ; unmark-atoms ; mark-atoms -id ${2} -full-residues ; print-atoms -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button1-on-marked-atom", "pick-objects ${1} ; unmark-atoms -id ${2} -full-residues ; print-atoms -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-unmarked-atom", "pick-objects ${1} ; unmark-atoms ; print-atoms -id ${2} -full-residues");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-marked-atom", "pick-objects ${1} ; unmark-atoms ; print-atoms -id ${2} -full-residues");

		script_partitioner().set_alias("ctrl-shift-click-button1-on-unmarked-contact", "pick-objects ${1} ; unmark-contacts ; mark-contacts -id ${2} -full-residues ; print-contacts -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button1-on-marked-contact", "pick-objects ${1} ; unmark-contacts -id ${2} -full-residues ; print-contacts -id ${2}");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-unmarked-contact", "pick-objects ${1} ; unmark-contacts ; print-contacts -id ${2} -full-residues");
		script_partitioner().set_alias("ctrl-shift-click-button2-on-marked-contact", "pick-objects ${1} ; unmark-contacts ; print-contacts -id ${2} -full-residues");

		script_partitioner().set_alias("click-button1-on-figure", "pick-objects ${1} ; print-figures -id ${2}");
		script_partitioner().set_alias("click-button2-on-figure", "pick-objects ${1} ; print-figures -id ${2}");

		script_partitioner().set_alias("modify-just-loaded-object", "set-tag-of-atoms-by-secondary-structure");
		script_partitioner().set_alias("zoom-just-loaded-object", "zoom-by-atoms");
		script_partitioner().set_alias("show-just-loaded-object", "hide-atoms ; hide-contacts ; show-atoms [-t! het] -rep cartoon ; show-atoms [-t het] -rep sticks");
		script_partitioner().set_alias("color-just-loaded-object", "color-atoms [-t! het] -next-random-color ; color-atoms [-t het] -next-random-color");
		script_partitioner().set_alias("process-just-loaded-object", "modify-just-loaded-object ; zoom-just-loaded-object ; show-just-loaded-object ; color-just-loaded-object");
	}

	template<typename CommandRecord>
	void insert_additional_script_if_requested(const CommandRecord& cr)
	{
		if(cr.successful && cr.heterostorage.summaries_of_atoms.count("loaded")==1)
		{
			script_partitioner().add_pending_sentences_from_string_to_front("process-just-loaded-object");
		}
	}

	template<typename CommandRecord>
	void zoom_if_requested(const CommandRecord& cr)
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
			}
		}
	}

	CongregationOfDrawersForDataManagers congregation_of_drawers_;
};

}

}

#endif /* VIEWER_SCRIPT_EXECUTION_MANAGER_H_ */
