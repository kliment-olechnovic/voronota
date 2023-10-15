#ifndef VIEWER_CONSOLE_OBJECT_LIST_VIEWER_STATE_H_
#define VIEWER_CONSOLE_OBJECT_LIST_VIEWER_STATE_H_

#include "objects_info.h"
#include "named_selections_info.h"
#include "marking_info.h"

#include "common_for_gui.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class ObjectListViewerPanel
{
public:
	const ObjectsInfo& objects_info;
	const NamedSelectionsInfo& named_selections_info;
	MarkingInfo& marking_info;

	bool visible;

	ObjectListViewerPanel(const ObjectsInfo& objects_info, const NamedSelectionsInfo& named_selections_info, MarkingInfo& marking_info) :
		objects_info(objects_info),
		named_selections_info(named_selections_info),
		marking_info(marking_info),
		visible(true),
		need_atoms_unmark_button_(false),
		need_contacts_unmark_button_(false),
		current_atoms_selection_not_default_(false),
		current_contacts_selection_not_default_(false),
		current_atoms_selection_marked_(false),
		current_contacts_selection_marked_(false),
		default_atoms_selection_string_("[]"),
		marked_atoms_selection_string_("[_marked]"),
		default_contacts_selection_string_("[]"),
		marked_contacts_selection_string_("[_marked]")
	{
		atoms_selection_string_suggestions_.first.push_back(default_atoms_selection_string_);
		atoms_selection_string_suggestions_.first.push_back(marked_atoms_selection_string_);
		atoms_selection_string_suggestions_.first.push_back("[_visible]");
		atoms_selection_string_suggestions_.first.push_back("[-protein]");
		atoms_selection_string_suggestions_.first.push_back("[-nucleic]");
		atoms_selection_string_suggestions_.first.push_back("[-sel-of-contacts _visible]");
		atoms_selection_string_suggestions_.first.push_back("[-sel-of-contacts _marked]");
		atoms_selection_string_suggestions_.first.push_back("(not [-aname C,N,O,CA])");
		atoms_selection_string_suggestions_.second.push_back("[-chain A]");
		atoms_selection_string_suggestions_.second.push_back("[-chain A -rnum 1:200]");

		contacts_selection_string_suggestions_.first.push_back(default_contacts_selection_string_);
		contacts_selection_string_suggestions_.first.push_back(marked_contacts_selection_string_);
		contacts_selection_string_suggestions_.first.push_back("[_visible]");
		contacts_selection_string_suggestions_.first.push_back("[-no-solvent]");
		contacts_selection_string_suggestions_.first.push_back("[-no-solvent -min-seq-sep 1]");
		contacts_selection_string_suggestions_.first.push_back("[-solvent]");
		contacts_selection_string_suggestions_.first.push_back("[-inter-chain]");
		contacts_selection_string_suggestions_.first.push_back("[-a1 [_marked] -a2! [_marked] -no-solvent]");
		contacts_selection_string_suggestions_.first.push_back("[-a1 [_marked] -a2 [_marked] -min-seq-sep 1]");
		contacts_selection_string_suggestions_.second.push_back("[-a1 [-protein] -a2 [-nucleic]]");
		contacts_selection_string_suggestions_.second.push_back("[-a1 [-chain A] -a2 [-chain B]]");
	}

	void execute(std::string& result)
	{
		const float small_buttons_spacing=5.0f*GUIStyleWrapper::scale_factor();

		const std::vector<ObjectsInfo::ObjectState>& object_states=objects_info.get_object_states();

		if(object_states.empty())
		{
			ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, color_text);
			ImGui::TextUnformatted("No objects loaded yet.");
			ImGui::PopStyleColor();
			return;
		}

		if(marking_info.atoms_marking_updated)
		{
			if(marking_info.atoms_marking_present)
			{
				if(atoms_selection_string_safe()!=marked_atoms_selection_string_)
				{
					set_atoms_selection_string_and_save_suggestion(marked_atoms_selection_string_);
				}
			}
			else
			{
				if(atoms_selection_string_safe()==marked_atoms_selection_string_)
				{
					set_atoms_selection_string_and_save_suggestion(atoms_selection_string_previous_.empty() ? default_atoms_selection_string_ : atoms_selection_string_previous_);
				}
				if(atoms_selection_string_previous_==marked_atoms_selection_string_)
				{
					atoms_selection_string_previous_.clear();
				}
			}
			need_atoms_unmark_button_=marking_info.atoms_marking_present;
		}

		if(marking_info.contacts_marking_updated)
		{
			if(marking_info.contacts_marking_present)
			{
				if(contacts_selection_string_safe()!=marked_contacts_selection_string_)
				{
					set_contacts_selection_string_and_save_suggestion(marked_contacts_selection_string_);
				}
			}
			else
			{
				if(contacts_selection_string_safe()==marked_contacts_selection_string_)
				{
					set_contacts_selection_string_and_save_suggestion(contacts_selection_string_previous_.empty() ? default_contacts_selection_string_ : contacts_selection_string_previous_);
				}
				if(contacts_selection_string_previous_==marked_contacts_selection_string_)
				{
					contacts_selection_string_previous_.clear();
				}
			}
			need_contacts_unmark_button_=marking_info.contacts_marking_present;
		}

		marking_info.reset();

		current_atoms_selection_not_default_=(atoms_selection_string_safe()!=default_atoms_selection_string_);
		current_contacts_selection_not_default_=(contacts_selection_string_safe()!=default_contacts_selection_string_);
		current_atoms_selection_marked_=current_atoms_selection_not_default_ && (atoms_selection_string_safe()==marked_atoms_selection_string_);
		current_contacts_selection_marked_=current_contacts_selection_not_default_ && (contacts_selection_string_safe()==marked_contacts_selection_string_);

		{
			{
				{
					ImGui::TextUnformatted("Atoms scope:");

					ImGui::SameLine();

					{
						{
							if(current_atoms_selection_not_default_)
							{
								if(current_atoms_selection_marked_)
								{
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.4f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.5f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.4f, 1.0f));
								}
								else
								{
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.1f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.2f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.4f, 0.1f, 1.0f));
								}
							}
							else
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.7f, 1.0f));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.8f, 0.8f, 1.0f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.7f, 1.0f));
							}

							std::string button_id=atoms_selection_string_safe()+"##button_atoms_selection_change";
							ImGui::Button(button_id.c_str());

							ImGui::PopStyleColor(3);
						}
						const std::string submenu_id=std::string("Change##submenu_atoms_selection");
						if(ImGui::BeginPopupContextItem(submenu_id.c_str(), 0))
						{
							if(atoms_selection_buffer_.empty())
							{
								atoms_selection_buffer_=std::vector<char>(atoms_selection_string_.begin(), atoms_selection_string_.end());
								atoms_selection_buffer_.resize(atoms_selection_string_.size()+128, 0);
							}
							const std::string textbox_id=std::string("##atoms_selection");
							ImGui::PushItemWidth(400.0f*GUIStyleWrapper::scale_factor());
							if(ImGui::InputText(textbox_id.c_str(), atoms_selection_buffer_.data(), 128, ImGuiInputTextFlags_EnterReturnsTrue))
							{
								const std::string newvalue(atoms_selection_buffer_.data());
								if(!newvalue.empty())
								{
									set_atoms_selection_string_and_save_suggestion(newvalue);
									ImGui::CloseCurrentPopup();
								}
							}
							ImGui::PopItemWidth();
							{
								const std::string button_id=std::string("OK##button_atoms_selection_ok");
								if(ImGui::Button(button_id.c_str()))
								{
									set_atoms_selection_string_and_save_suggestion(std::string(atoms_selection_buffer_.data()));
									ImGui::CloseCurrentPopup();
								}
							}
							ImGui::SameLine();
							{
								const std::string button_id=std::string("Cancel##button_atoms_selection_cancel");
								if(ImGui::Button(button_id.c_str()))
								{
									ImGui::CloseCurrentPopup();
								}
							}
							ImGui::SameLine();
							{
								const std::string button_id=std::string("Default##button_atoms_selection_reset");
								if(ImGui::Button(button_id.c_str()))
								{
									set_atoms_selection_string_and_save_suggestion(default_atoms_selection_string_);
									ImGui::CloseCurrentPopup();
								}
							}
							if(!atoms_selection_string_previous_.empty())
							{
								ImGui::SameLine();
								std::string button_id="Restore ";
								button_id+=atoms_selection_string_previous_;
								button_id+="##button_atoms_selection_change_previous";
								if(ImGui::Button(button_id.c_str()))
								{
									set_atoms_selection_string_and_save_suggestion(atoms_selection_string_previous_);
									ImGui::CloseCurrentPopup();
								}
							}

							if(!atoms_selection_string_suggestions_.first.empty())
							{
								ImGui::Separator();

								for(std::size_t i=0;i<atoms_selection_string_suggestions_.first.size();i++)
								{
									if(ImGui::Selectable(atoms_selection_string_suggestions_.first[i].c_str()))
									{
										set_atoms_selection_string_and_save_suggestion(atoms_selection_string_suggestions_.first[i]);
									}
								}
							}

							if(!atoms_selection_string_suggestions_.second.empty())
							{
								ImGui::Separator();

								for(std::size_t i=0;i<atoms_selection_string_suggestions_.second.size();i++)
								{
									if(ImGui::Selectable(atoms_selection_string_suggestions_.second[i].c_str()))
									{
										set_atoms_selection_string_and_save_suggestion(atoms_selection_string_suggestions_.second[i]);
									}
								}
							}

							ImGui::EndPopup();
						}
					}

					if(need_atoms_unmark_button_)
					{
						ImGui::SameLine();
						const std::string button_id=std::string("unmark all##button_atoms_selection_unmark_all");
						if(ImGui::Button(button_id.c_str()))
						{
							need_atoms_unmark_button_=false;
							result=std::string("unmark-atoms");
						}
					}
				}

				{
					ImGui::TextUnformatted("Contacts scope:");

					ImGui::SameLine();

					{
						{
							if(current_contacts_selection_not_default_)
							{
								if(current_contacts_selection_marked_)
								{
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.4f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.5f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.4f, 1.0f));
								}
								else
								{
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.1f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.2f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.4f, 0.1f, 1.0f));
								}
							}
							else
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.7f, 1.0f));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.8f, 0.8f, 1.0f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.7f, 1.0f));
							}

							std::string button_id=contacts_selection_string_safe()+"##button_contacts_selection_change";
							ImGui::Button(button_id.c_str());

							ImGui::PopStyleColor(3);
						}
						const std::string submenu_id=std::string("Change##submenu_contacts_selection");
						if(ImGui::BeginPopupContextItem(submenu_id.c_str(), 0))
						{
							if(contacts_selection_buffer_.empty())
							{
								contacts_selection_buffer_=std::vector<char>(contacts_selection_string_.begin(), contacts_selection_string_.end());
								contacts_selection_buffer_.resize(contacts_selection_string_.size()+128, 0);
							}
							const std::string textbox_id=std::string("##contacts_selection");
							ImGui::PushItemWidth(400.0f*GUIStyleWrapper::scale_factor());
							if(ImGui::InputText(textbox_id.c_str(), contacts_selection_buffer_.data(), 128, ImGuiInputTextFlags_EnterReturnsTrue))
							{
								const std::string newvalue(contacts_selection_buffer_.data());
								if(!newvalue.empty())
								{
									set_contacts_selection_string_and_save_suggestion(newvalue);
									ImGui::CloseCurrentPopup();
								}
							}
							ImGui::PopItemWidth();
							{
								const std::string button_id=std::string("OK##button_contacts_selection_ok");
								if(ImGui::Button(button_id.c_str()))
								{
									set_contacts_selection_string_and_save_suggestion(std::string(contacts_selection_buffer_.data()));
									ImGui::CloseCurrentPopup();
								}
							}
							ImGui::SameLine();
							{
								const std::string button_id=std::string("Cancel##button_contacts_selection_cancel");
								if(ImGui::Button(button_id.c_str()))
								{
									ImGui::CloseCurrentPopup();
								}
							}
							ImGui::SameLine();
							{
								const std::string button_id=std::string("Default##button_contacts_selection_reset");
								if(ImGui::Button(button_id.c_str()))
								{
									set_contacts_selection_string_and_save_suggestion(default_contacts_selection_string_);
									ImGui::CloseCurrentPopup();
								}
							}
							if(!contacts_selection_string_previous_.empty())
							{
								ImGui::SameLine();
								std::string button_id="Restore ";
								button_id+=contacts_selection_string_previous_;
								button_id+="##button_contacts_selection_change_previous";
								if(ImGui::Button(button_id.c_str()))
								{
									set_contacts_selection_string_and_save_suggestion(contacts_selection_string_previous_);
									ImGui::CloseCurrentPopup();
								}
							}

							if(!contacts_selection_string_suggestions_.first.empty())
							{
								ImGui::Separator();

								for(std::size_t i=0;i<contacts_selection_string_suggestions_.first.size();i++)
								{
									if(ImGui::Selectable(contacts_selection_string_suggestions_.first[i].c_str()))
									{
										set_contacts_selection_string_and_save_suggestion(contacts_selection_string_suggestions_.first[i]);
									}
								}
							}

							if(!contacts_selection_string_suggestions_.second.empty())
							{
								ImGui::Separator();

								for(std::size_t i=0;i<contacts_selection_string_suggestions_.second.size();i++)
								{
									if(ImGui::Selectable(contacts_selection_string_suggestions_.second[i].c_str()))
									{
										set_contacts_selection_string_and_save_suggestion(contacts_selection_string_suggestions_.second[i]);
									}
								}
							}

							ImGui::EndPopup();
						}
					}

					if(need_contacts_unmark_button_)
					{
						ImGui::SameLine();
						const std::string button_id=std::string("unmark all##button_contacts_selection_unmark_all");
						if(ImGui::Button(button_id.c_str()))
						{
							need_contacts_unmark_button_=false;
							result=std::string("unmark-contacts");
						}
					}
				}
			}

			ImGui::Separator();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(small_buttons_spacing, ImGui::GetStyle().ItemSpacing.y));

			{
				const std::string button_id=std::string("P##button_picking_all");
				const std::string menu_id=std::string("Picking##menu_picking");
				ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
				if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
				{
					if(ImGui::Selectable("Pick all"))
					{
						result="pick-objects";
					}
					if(ImGui::Selectable("Unpick all"))
					{
						result="unpick-objects";
					}

					ImGui::Separator();

					if(ImGui::Selectable("Pick visible only"))
					{
						result="pick-objects -visible";
					}
					if(ImGui::Selectable("Unpick not visible"))
					{
						result="unpick-objects -not-visible";
					}

					ImGui::EndPopup();
				}
			}

			ImGui::SameLine();

			{
				const std::string button_id=std::string("V##button_visibility_all");
				const std::string menu_id=std::string("Visibility##menu_visibility");
				ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
				if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
				{
					if(ImGui::Selectable("Show all"))
					{
						result="show-objects";
					}
					if(ImGui::Selectable("Hide all"))
					{
						result="hide-objects";
					}

					ImGui::Separator();

					if(ImGui::Selectable("Show picked only"))
					{
						result="hide-objects\n";
						result+="show-objects -picked";
					}
					if(ImGui::Selectable("Hide not picked"))
					{
						result="hide-objects -not-picked";
					}

					ImGui::Separator();

					if(GUIConfiguration::instance().animation_variant!=GUIConfiguration::ANIMATION_VARIANT_NONE)
					{
						if(ImGui::Selectable("Stop animation"))
						{
							result="animate-none\n";
						}
					}

					if(ImGui::BeginMenu("Set animation##set_animation"))
					{
						if(GUIConfiguration::instance().animation_variant!=GUIConfiguration::ANIMATION_VARIANT_LOOP_PICKED_OBJECTS && objects_info.num_of_picked_objects()>1)
						{
							if(ImGui::Selectable("Loop picked objects, only forward"))
							{
								result="animate-loop-picked-objects\n";
							}
						}

						if(GUIConfiguration::instance().animation_variant!=GUIConfiguration::ANIMATION_VARIANT_LOOP_PICKED_OBJECTS_BIDIRECTIONALLY && objects_info.num_of_picked_objects()>1)
						{
							if(ImGui::Selectable("Loop picked objects, forward and backward"))
							{
								result="animate-loop-picked-objects-bidirectionally\n";
							}
						}

						if(GUIConfiguration::instance().animation_variant!=GUIConfiguration::ANIMATION_VARIANT_SPIN_LEFT)
						{
							if(ImGui::Selectable("Spin left"))
							{
								result="animate-spin-left\n";
							}
						}

						if(GUIConfiguration::instance().animation_variant!=GUIConfiguration::ANIMATION_VARIANT_SPIN_RIGHT)
						{
							if(ImGui::Selectable("Spin right"))
							{
								result="animate-spin-right\n";
							}
						}

						if(GUIConfiguration::instance().animation_variant!=GUIConfiguration::ANIMATION_VARIANT_SPIN_ON_Z_LEFT)
						{
							if(ImGui::Selectable("Spin clockwise"))
							{
								result="animate-spin-on-z-left\n";
							}
						}

						if(GUIConfiguration::instance().animation_variant!=GUIConfiguration::ANIMATION_VARIANT_SPIN_ON_Z_RIGHT)
						{
							if(ImGui::Selectable("Spin counter-clockwise"))
							{
								result="animate-spin-on-z-right\n";
							}
						}

						ImGui::Separator();

						static float animation_time_step=15.0f;

						if(ImGui::SliderFloat("milliseconds per frame", &animation_time_step, 1.0f, 40.0f, "%.1f", 0))
						{
							GUIConfiguration::instance().animation_step_miliseconds=static_cast<double>(animation_time_step);
						}

						ImGui::EndMenu();
					}

					ImGui::EndPopup();
				}
			}

			ImGui::SameLine();

			{
				const std::string button_id=std::string("O##button_objects");
				const std::string menu_id=std::string("Objects##menu_objects");
				ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
				if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
				{
					if(ImGui::Selectable("Zoom"))
					{
						result="zoom-by-objects";
					}

					if(ImGui::Selectable("Orient"))
					{
						result="orient";
					}

					ImGui::Separator();

					{
						ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
						ImGui::PushStyleColor(ImGuiCol_Text, color_text);

						if(ImGui::Selectable("Delete all"))
						{
							result="delete-objects";
						}
						if(ImGui::Selectable("Delete picked"))
						{
							result="delete-objects -picked";
						}
						if(ImGui::Selectable("Delete not picked"))
						{
							result="delete-objects -not-picked";
						}

						ImGui::PopStyleColor();
					}
					ImGui::EndPopup();
				}
			}

			ImGui::SameLine();
			process_button_A(std::string(), result);

			ImGui::SameLine();
			process_button_S(std::string(), result);

			ImGui::SameLine();
			process_button_H(std::string(), result);

			ImGui::SameLine();
			process_button_Ca(std::string(), result);

			const bool present_contacts=objects_info.any_object_has_contacts();

			if(!present_contacts)
			{
				ImGui::BeginDisabled();
			}

			ImGui::SameLine();
			process_button_Cc(std::string(), result);

			if(!present_contacts)
			{
				ImGui::EndDisabled();
			}

			ImGui::PopStyleVar();

			{
				if(named_selections_info.empty())
				{
					ImGui::BeginDisabled();
				}

				ImGui::SameLine();
				ImGui::Button("selections");
				if(!named_selections_info.empty() && ImGui::BeginPopupContextItem("List##submenu_named_selections", 0))
				{
					ImGui::TextUnformatted("Atoms recorded selections:");

					if(named_selections_info.atoms_mapping_of_names.empty())
					{
						ImGui::TextUnformatted("  none recorded");
					}
					else
					{
						for(std::map< std::string, std::set<std::string> >::const_iterator it=named_selections_info.atoms_mapping_of_names.selections_to_objects.begin();it!=named_selections_info.atoms_mapping_of_names.selections_to_objects.end();++it)
						{
							const std::string& selname=it->first;
							const std::string displayname=std::string("  ")+selname+"##for_atoms_named_selection";
							if(ImGui::BeginMenu(displayname.c_str()))
							{
								if(ImGui::Selectable("Set as atoms scope string##for_atoms_named_selection"))
								{
									set_atoms_selection_string_and_save_suggestion(std::string("[")+selname+"]");
								}

								ImGui::Separator();

								ImGui::TextUnformatted("Objects that have it:");

								if(ImGui::BeginMenu("  list##for_atoms_named_selection"))
								{
									for(std::set<std::string>::const_iterator jt=(it->second).begin();jt!=(it->second).end();++jt)
									{
										ImGui::TextUnformatted(jt->c_str());
									}
									ImGui::EndMenu();
								}

								if(ImGui::Selectable("  pick all##for_atoms_named_selection"))
								{
									result=std::string("pick-objects -names");
									for(std::set<std::string>::const_iterator jt=(it->second).begin();jt!=(it->second).end();++jt)
									{
										result+=" ";
										result+=(*jt);
									}
								}

								if(ImGui::Selectable("  unpick all##for_atoms_named_selection"))
								{
									result=std::string("unpick-objects -names");
									for(std::set<std::string>::const_iterator jt=(it->second).begin();jt!=(it->second).end();++jt)
									{
										result+=" ";
										result+=(*jt);
									}
								}

								ImGui::Separator();

								ImGui::TextUnformatted("Actions on all picked objects:");

								if(ImGui::Selectable("  mark atoms of named selection##for_atoms_named_selection"))
								{
									result=std::string("mark-atoms -use [")+selname+"]";
								}

								if(ImGui::Selectable("  remove named selection##for_atoms_named_selection"))
								{
									result=std::string("delete-selections-of-atoms -names ")+selname;
								}

								ImGui::EndMenu();
							}
						}
					}

					ImGui::Separator();

					ImGui::TextUnformatted("Contacts recorded selections:");

					if(named_selections_info.contacts_mapping_of_names.empty())
					{
						ImGui::TextUnformatted("  none recorded");
					}
					else
					{
						for(std::map< std::string, std::set<std::string> >::const_iterator it=named_selections_info.contacts_mapping_of_names.selections_to_objects.begin();it!=named_selections_info.contacts_mapping_of_names.selections_to_objects.end();++it)
						{
							const std::string& selname=it->first;
							const std::string displayname=std::string("  ")+selname+"##for_contacts_named_selection";
							if(ImGui::BeginMenu(displayname.c_str()))
							{
								if(ImGui::Selectable("Set as contacts scope string##for_contacts_named_selection"))
								{
									set_contacts_selection_string_and_save_suggestion(std::string("[")+selname+"]");
								}

								ImGui::Separator();

								ImGui::TextUnformatted("Objects that have it:");

								if(ImGui::BeginMenu("  list##for_contacts_named_selection"))
								{
									for(std::set<std::string>::const_iterator jt=(it->second).begin();jt!=(it->second).end();++jt)
									{
										ImGui::TextUnformatted(jt->c_str());
									}
									ImGui::EndMenu();
								}

								if(ImGui::Selectable("  pick all##for_contacts_named_selection"))
								{
									result=std::string("pick-objects -names");
									for(std::set<std::string>::const_iterator jt=(it->second).begin();jt!=(it->second).end();++jt)
									{
										result+=" ";
										result+=(*jt);
									}
								}

								if(ImGui::Selectable("  unpick all##for_contacts_named_selection"))
								{
									result=std::string("unpick-objects -names");
									for(std::set<std::string>::const_iterator jt=(it->second).begin();jt!=(it->second).end();++jt)
									{
										result+=" ";
										result+=(*jt);
									}
								}

								ImGui::Separator();

								ImGui::TextUnformatted("Actions on all picked objects:");

								if(ImGui::Selectable("  mark contacts of named selection##for_contacts_named_selection"))
								{
									result=std::string("mark-contacts -use [")+selname+"]";
								}

								if(ImGui::Selectable("  remove named selection##for_contacts_named_selection"))
								{
									result=std::string("delete-selections-of-contacts -names ")+selname;
								}

								ImGui::EndMenu();
							}
						}
					}

					ImGui::EndPopup();
				}

				if(named_selections_info.empty())
				{
					ImGui::EndDisabled();
				}
			}
		}

		ImGui::Separator();

		ImGui::BeginChild("##object_list_scrolling_region", ImVec2(0, 0), false);

		for(std::size_t i=0;i<object_states.size();i++)
		{
			const ObjectsInfo::ObjectState& os=object_states[i];

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(small_buttons_spacing, ImGui::GetStyle().ItemSpacing.y));

			{
				const std::string checkbox_id=std::string("##checkbox_pick_")+os.name;
				bool picked=os.picked;
				if(ImGui::Checkbox(checkbox_id.c_str(), &picked))
				{
					if(picked)
					{
						result=std::string("pick-more-objects -names ")+os.name;
					}
					else
					{
						result=std::string("unpick-objects -names ")+os.name;
					}
				}
			}

			ImGui::SameLine();

			{
				const std::string checkbox_id=std::string("##checkbox_show_")+os.name;
				bool visible=os.visible;
				if(ImGui::Checkbox(checkbox_id.c_str(), &visible))
				{
					if(visible)
					{
						result=std::string("show-objects -names ")+os.name;
					}
					else
					{
						result=std::string("hide-objects -names ")+os.name;
					}
				}
			}

			ImGui::SameLine();

			{
				const std::string button_id=std::string("O##button_object_")+os.name;
				const std::string menu_id=std::string("Object##menu_object_")+os.name;
				ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
				if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
				{
					{
						static std::map< std::string, std::vector<char> > renaming_buffers;
						std::vector<char>& renaming_buffer=renaming_buffers[os.name];
						if(renaming_buffer.empty())
						{
							renaming_buffer=std::vector<char>(os.name.begin(), os.name.end());
							renaming_buffer.resize(os.name.size()+128, 0);
						}

						{
							const std::string textbox_id=std::string("##rename_")+os.name;
							ImGui::InputText(textbox_id.c_str(), renaming_buffer.data(), 128);
						}

						if(std::strcmp(renaming_buffer.data(), os.name.c_str())!=0)
						{
							if(renaming_buffer.data()[0]!=0)
							{
								const std::string newname(renaming_buffer.data());

								{
									const std::string button_id=std::string("rename##button_rename_ok_")+os.name;
									if(ImGui::Button(button_id.c_str()))
									{
										if(!newname.empty() && newname!=os.name)
										{
											result=std::string("rename-object '")+os.name+"' '"+newname+"'";
											renaming_buffers.erase(os.name);
											ImGui::CloseCurrentPopup();
										}
									}
								}

								ImGui::SameLine();

								{
									const std::string button_id=std::string("duplicate##button_duplicate_ok_")+os.name;
									if(ImGui::Button(button_id.c_str()))
									{
										if(!newname.empty() && newname!=os.name)
										{
											result=std::string("copy-object '")+os.name+"' '"+newname+"'";
											renaming_buffers.erase(os.name);
										}
										ImGui::CloseCurrentPopup();
									}
								}

								ImGui::SameLine();
							}

							{
								const std::string button_id=std::string("restore##button_duplicate_ok_")+os.name;
								if(ImGui::Button(button_id.c_str()))
								{
									renaming_buffers.erase(os.name);
								}
							}
						}
					}

					ImGui::Separator();

					if(ImGui::Selectable("Copy name to clipboard"))
					{
						ImGui::SetClipboardText(os.name.c_str());
					}

					ImGui::Separator();

					if(ImGui::Selectable("Duplicate"))
					{
						result=std::string("copy-object '")+os.name+"' '"+os.name+"_copy'";
					}

					ImGui::Separator();

					if(ImGui::Selectable("Zoom"))
					{
						result=std::string("zoom-by-objects -names '")+os.name+"'";
					}

					if(ImGui::Selectable("Orient"))
					{
						result=std::string("orient -names '")+os.name+"'";
					}

					if(object_states.size()>1)
					{
						ImGui::Separator();

						if(ImGui::BeginMenu("Hide others"))
						{
							if(ImGui::MenuItem("all"))
							{
								result="";
								for(std::size_t j=0;j<object_states.size();j++)
								{
									if(j!=i)
									{
										result+=std::string("hide-objects -names '")+object_states[j].name+"'\n";
									}
								}
							}

							if(i>0)
							{
								if(ImGui::MenuItem("above"))
								{
									result="";
									for(std::size_t j=0;j<i;j++)
									{
										result+=std::string("hide-objects -names '")+object_states[j].name+"'\n";
									}
								}
							}

							if((i+1)<object_states.size())
							{
								if(ImGui::MenuItem("below"))
								{
									result="";
									for(std::size_t j=(i+1);j<object_states.size();j++)
									{
										result+=std::string("hide-objects -names '")+object_states[j].name+"'\n";
									}
								}
							}

							ImGui::EndMenu();
						}

						if(ImGui::BeginMenu("Unpick others"))
						{
							if(ImGui::MenuItem("all"))
							{
								result="";
								for(std::size_t j=0;j<object_states.size();j++)
								{
									if(j!=i)
									{
										result+=std::string("unpick-objects -names '")+object_states[j].name+"'\n";
									}
								}
							}

							if(i>0)
							{
								if(ImGui::MenuItem("above"))
								{
									result="";
									for(std::size_t j=0;j<i;j++)
									{
										result+=std::string("unpick-objects -names '")+object_states[j].name+"'\n";
									}
								}
							}

							if((i+1)<object_states.size())
							{
								if(ImGui::MenuItem("below"))
								{
									result="";
									for(std::size_t j=(i+1);j<object_states.size();j++)
									{
										result+=std::string("unpick-objects -names '")+object_states[j].name+"'\n";
									}
								}
							}

							ImGui::EndMenu();
						}
					}

					ImGui::Separator();

					{
						ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
						ImGui::PushStyleColor(ImGuiCol_Text, color_text);

						if(ImGui::Selectable("Delete"))
						{
							result=std::string("delete-objects -names '")+os.name+"'";
						}

						ImGui::PopStyleColor();
					}

					ImGui::EndPopup();
				}
			}

			ImGui::SameLine();
			process_button_A(os.name, result);

			ImGui::SameLine();
			process_button_S(os.name, result);

			ImGui::SameLine();
			process_button_H(os.name, result);

			ImGui::SameLine();
			process_button_Ca(os.name, result);

			const bool present_contacts=objects_info.object_has_contacts(os.name);

			if(!present_contacts)
			{
				ImGui::BeginDisabled();
			}

			ImGui::SameLine();
			process_button_Cc(os.name, result);

			if(!present_contacts)
			{
				ImGui::EndDisabled();
			}

			ImGui::PopStyleVar();

			ImGui::SameLine();

			{
				float lightness=1.0f;
				if(!os.picked)
				{
					lightness=0.5f;
				}
				ImVec4 color_text=ImVec4(lightness, lightness, lightness, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				ImGui::TextUnformatted(os.name.c_str());
				ImGui::PopStyleColor();
			}
		}

		ImGui::EndChild();
	}

private:
	void set_atoms_selection_string_and_save_suggestion(const std::string& value)
	{
		const std::string future_previous_string=atoms_selection_string_safe();
		atoms_selection_string_=value;
		bool already_suggested=false;
		for(std::size_t i=0;i<atoms_selection_string_suggestions_.first.size() && !already_suggested;i++)
		{
			already_suggested=already_suggested || (atoms_selection_string_suggestions_.first[i]==atoms_selection_string_safe());
		}
		for(std::size_t i=0;i<atoms_selection_string_suggestions_.second.size() && !already_suggested;i++)
		{
			already_suggested=already_suggested || (atoms_selection_string_suggestions_.second[i]==atoms_selection_string_safe());
		}
		if(!already_suggested)
		{
			atoms_selection_string_suggestions_.second.push_back(atoms_selection_string_safe());
			if(atoms_selection_string_suggestions_.second.size()>5)
			{
				atoms_selection_string_suggestions_.second.pop_front();
			}
		}
		if(future_previous_string!=atoms_selection_string_previous_ && future_previous_string!=atoms_selection_string_safe())
		{
			atoms_selection_string_previous_=future_previous_string;
		}
		atoms_selection_buffer_.clear();
	}

	void set_contacts_selection_string_and_save_suggestion(const std::string& value)
	{
		const std::string future_previous_string=contacts_selection_string_safe();
		contacts_selection_string_=value;
		bool already_suggested=false;
		for(std::size_t i=0;i<contacts_selection_string_suggestions_.first.size() && !already_suggested;i++)
		{
			already_suggested=already_suggested || (contacts_selection_string_suggestions_.first[i]==contacts_selection_string_safe());
		}
		for(std::size_t i=0;i<contacts_selection_string_suggestions_.second.size() && !already_suggested;i++)
		{
			already_suggested=already_suggested || (contacts_selection_string_suggestions_.second[i]==contacts_selection_string_safe());
		}
		if(!already_suggested)
		{
			contacts_selection_string_suggestions_.second.push_back(contacts_selection_string_safe());
			if(contacts_selection_string_suggestions_.second.size()>5)
			{
				contacts_selection_string_suggestions_.second.pop_front();
			}
		}
		if(future_previous_string!=contacts_selection_string_previous_ && future_previous_string!=contacts_selection_string_safe())
		{
			contacts_selection_string_previous_=future_previous_string;
		}
		contacts_selection_buffer_.clear();
	}

	const std::string& atoms_selection_string_safe()
	{
		if(atoms_selection_string_.empty() || atoms_selection_string_=="[" || atoms_selection_string_=="]")
		{
			atoms_selection_string_=default_atoms_selection_string_;
		}
		return atoms_selection_string_;
	}

	const std::string& contacts_selection_string_safe()
	{
		if(contacts_selection_string_.empty() || contacts_selection_string_=="[" || contacts_selection_string_=="]")
		{
			contacts_selection_string_=default_contacts_selection_string_;
		}
		return contacts_selection_string_;
	}

	static std::string objects_selection_option(const std::string& name)
	{
		if(name.empty())
		{
			return std::string();
		}
		return (std::string(" -on-objects '")+name+"' ");
	}

	static std::string flag_option(const std::string& name, const bool value)
	{
		return (value ? (std::string(" ")+name) : std::string());
	}

	template<typename A, typename B>
	static std::string multipart_option(const A& a, const B& b)
	{
		std::ostringstream output;
		output << " " << a << " " << b;
		return output.str();
	}

	void process_button_A(const std::string& os_name, std::string& result)
	{
		const std::string button_id=std::string("A##button_actions_")+os_name;
		const std::string menu_id=std::string("Actions##menu_actions_")+os_name;
		ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
		if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
		{
			{
				ImGui::TextUnformatted("Atoms actions:");

				ImGui::Separator();

				if(ImGui::Selectable("  Summarize atoms"))
				{
					result=std::string("select-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}

				if(ImGui::BeginMenu("  Record named selection of atoms##atoms_record_named_selection"))
				{
					static std::map< std::string, std::vector<char> > naming_buffers;
					std::vector<char>& naming_buffer=naming_buffers[os_name];
					if(naming_buffer.empty())
					{
						const std::string default_atoms_selection_name="sel_atoms";
						naming_buffer=std::vector<char>(default_atoms_selection_name.begin(), default_atoms_selection_name.end());
						naming_buffer.resize(default_atoms_selection_name.size()+128, 0);
					}

					{
						const std::string textbox_id=std::string("##atoms_selection_record_name_")+os_name;
						ImGui::InputText(textbox_id.c_str(), naming_buffer.data(), 128);
					}

					if(naming_buffer.data()[0]!=0)
					{
						const std::string newname(naming_buffer.data());
						const std::string button_id=std::string("record##atoms_selection_button_record_ok_")+os_name;
						if(ImGui::Button(button_id.c_str()))
						{
							if(!newname.empty())
							{
								result=std::string("select-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+") -name '"+newname+"'";
								ImGui::CloseCurrentPopup();
							}
						}
					}
					else
					{
						ImGui::TextUnformatted("please enter a selection name");
					}

					ImGui::EndMenu();
				}

				ImGui::Separator();

				if(ImGui::Selectable("  Mark atoms"))
				{
					result=std::string("mark-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  Unmark atoms"))
				{
					result=std::string("unmark-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}

				ImGui::Separator();

				if(ImGui::Selectable("  Unmark all atoms"))
				{
					result=std::string("unmark-atoms ")+objects_selection_option(os_name)+"";
				}

				ImGui::Separator();

				if(ImGui::BeginMenu("  Add or replace standard labels##add_labels"))
				{
					static bool labels_centered=true;
					static float label_scale_value=1.0f;
					static float label_depth_shift_value=3.0f;

					ImGui::Checkbox("centered##labels_centered_checkbox", &labels_centered);

					ImGui::SliderFloat("scaling factor", &label_scale_value, 0.3f, 5.0f, "%.1f", 0);

					ImGui::SliderFloat("depth shift", &label_depth_shift_value, 2.0f, 30.0f, "%.1f", 0);

					ImGui::Separator();

					ImGui::TextUnformatted("residue-level:");

					if(ImGui::MenuItem("  chain.rnum.rname"))
					{
						result=std::string("add-figures-of-labels -mode residue -text '${chain}.${rnum}${icode}.${rname}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::MenuItem("  chain.rnum.rnameshort"))
					{
						result=std::string("add-figures-of-labels -mode residue -text '${chain}.${rnum}${icode}.${rnameshort}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::MenuItem("  chain.rnum"))
					{
						result=std::string("add-figures-of-labels -mode residue -text '${chain}.${rnum}${icode}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::MenuItem("  rnum.rnameshort"))
					{
						result=std::string("add-figures-of-labels -mode residue -text '${rnum}${icode}.${rnameshort}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::MenuItem("  rname"))
					{
						result=std::string("add-figures-of-labels -mode residue -text '${rname}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::MenuItem("  rnameshort"))
					{
						result=std::string("add-figures-of-labels -mode residue -text '${rnameshort}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::BeginMenu("  custom text##residue"))
					{
						static std::map< std::string, std::vector<char> > text_buffers;
						std::vector<char>& text_buffer=text_buffers[os_name];
						if(text_buffer.empty())
						{
							const std::string default_text="text";
							text_buffer=std::vector<char>(default_text.begin(), default_text.end());
							text_buffer.resize(default_text.size()+128, 0);
						}

						{
							const std::string textbox_id=std::string("##residue_label_custom_text_")+os_name;
							ImGui::InputText(textbox_id.c_str(), text_buffer.data(), 128);
						}

						if(text_buffer.data()[0]!=0)
						{
							const std::string newtext(text_buffer.data());
							const std::string button_id=std::string("OK##residue_label_custom_text_ok_")+os_name;
							if(ImGui::Button(button_id.c_str()))
							{
								if(!newtext.empty())
								{
									result=std::string("add-figures-of-labels -mode residue -text '")+newtext+"' "+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
									ImGui::CloseCurrentPopup();
								}
							}
						}
						else
						{
							ImGui::TextUnformatted("please enter text");
						}

						ImGui::EndMenu();
					}

					ImGui::Separator();

					ImGui::TextUnformatted("atom-level:");

					if(ImGui::MenuItem("  chain.rnum.rname.aname"))
					{
						result=std::string("add-figures-of-labels -mode atom -text '${chain}.${rnum}${icode}.${rname}.${aname}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::MenuItem("  rname.aname"))
					{
						result=std::string("add-figures-of-labels -mode atom -text '${rname}.${aname}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::MenuItem("  rnum.rnameshort.aname"))
					{
						result=std::string("add-figures-of-labels -mode atom -text '${rnum}${icode}.${rnameshort}.${aname}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::MenuItem("  rnum.aname"))
					{
						result=std::string("add-figures-of-labels -mode atom -text '${rnum}${icode}.${aname}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::MenuItem("  aname"))
					{
						result=std::string("add-figures-of-labels -mode atom -text '${aname}' ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
					}

					if(ImGui::BeginMenu("  custom text##atom"))
					{
						static std::map< std::string, std::vector<char> > text_buffers;
						std::vector<char>& text_buffer=text_buffers[os_name];
						if(text_buffer.empty())
						{
							const std::string default_text="text";
							text_buffer=std::vector<char>(default_text.begin(), default_text.end());
							text_buffer.resize(default_text.size()+128, 0);
						}

						{
							const std::string textbox_id=std::string("##atom_label_custom_text_")+os_name;
							ImGui::InputText(textbox_id.c_str(), text_buffer.data(), 128);
						}

						if(text_buffer.data()[0]!=0)
						{
							const std::string newtext(text_buffer.data());
							const std::string button_id=std::string("OK##atom_label_custom_text_ok_")+os_name;
							if(ImGui::Button(button_id.c_str()))
							{
								if(!newtext.empty())
								{
									result=std::string("add-figures-of-labels -mode atom -text '")+newtext+"' "+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+flag_option("-centered", labels_centered)+multipart_option("-scale", label_scale_value)+multipart_option("-depth-shift", label_depth_shift_value);
									ImGui::CloseCurrentPopup();
								}
							}
						}
						else
						{
							ImGui::TextUnformatted("please enter text");
						}

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				if(ImGui::BeginMenu("  Remove standard labels##remove_labels"))
				{
					if(ImGui::MenuItem("residue-level"))
					{
						result=std::string("delete-figures-of-labels -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}

					if(ImGui::MenuItem("atom-level"))
					{
						result=std::string("delete-figures-of-labels -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}

					if(ImGui::MenuItem("all"))
					{
						result=std::string("delete-figures-of-labels -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						result+=std::string(" ; delete-figures-of-labels -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}

					ImGui::EndMenu();
				}

				if(!os_name.empty())
				{
					const std::vector<ObjectsInfo::ObjectState>& object_states=objects_info.get_object_states();
					if(object_states.size()>1)
					{
						ImGui::Separator();

						ImVec4 color_text=ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
						ImGui::PushStyleColor(ImGuiCol_Text, color_text);

						if(ImGui::Selectable("  Align all"))
						{
							result=std::string("tmalign-many -target '")+os_name+"' -target-sel '("+atoms_selection_string_safe()+")' -model-sel '("+atoms_selection_string_safe()+")'";
						}

						int num_of_other_picked_objects=0;
						for(std::size_t j=0;j<object_states.size();j++)
						{
							if(object_states[j].name!=os_name && object_states[j].picked)
							{
								num_of_other_picked_objects++;
							}
						}

						if(num_of_other_picked_objects>0 && ImGui::Selectable("  Align picked"))
						{
							result=std::string("tmalign-many -picked -target '")+os_name+"' -target-sel '("+atoms_selection_string_safe()+")' -model-sel '("+atoms_selection_string_safe()+")'";
						}

						ImGui::PopStyleColor();
					}
				}

				{
					ImGui::Separator();

					ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);

					if(ImGui::Selectable("  Restrict to atoms in scope"))
					{
						result=std::string("restrict-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}

					if(ImGui::Selectable("  Restrict to atoms not in scope"))
					{
						result=std::string("restrict-atoms ")+objects_selection_option(os_name)+" -use (not ("+atoms_selection_string_safe()+"))";
					}

					ImGui::PopStyleColor();
				}
			}

			ImGui::Separator();
			ImGui::Separator();

			{
				ImGui::TextUnformatted("Contacts actions:");

				ImGui::Separator();

				if(ImGui::Selectable("  Construct contacts"))
				{
					result=std::string("construct-contacts ")+objects_selection_option(os_name)+"";
				}

				ImGui::Separator();

				const bool present_contacts=(os_name.empty() ? objects_info.any_object_has_contacts() : objects_info.object_has_contacts(os_name));

				if(!present_contacts)
				{
					ImGui::BeginDisabled();
				}

				if(ImGui::Selectable("  Summarize contacts"))
				{
					result=std::string("select-contacts ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}

				if(ImGui::BeginMenu("  Record named selection of contacts##contacts_record_named_selection"))
				{
					static std::map< std::string, std::vector<char> > naming_buffers;
					std::vector<char>& naming_buffer=naming_buffers[os_name];
					if(naming_buffer.empty())
					{
						const std::string default_contacts_selection_name="sel_contacts";
						naming_buffer=std::vector<char>(default_contacts_selection_name.begin(), default_contacts_selection_name.end());
						naming_buffer.resize(default_contacts_selection_name.size()+128, 0);
					}

					{
						const std::string textbox_id=std::string("##contacts_selection_record_name_")+os_name;
						ImGui::InputText(textbox_id.c_str(), naming_buffer.data(), 128);
					}

					if(naming_buffer.data()[0]!=0)
					{
						const std::string newname(naming_buffer.data());
						const std::string button_id=std::string("record##contacts_selection_button_record_ok_")+os_name;
						if(ImGui::Button(button_id.c_str()))
						{
							if(!newname.empty())
							{
								result=std::string("select-contacts ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+") -name '"+newname+"'";
								ImGui::CloseCurrentPopup();
							}
						}
					}
					else
					{
						ImGui::TextUnformatted("please enter a selection name");
					}

					ImGui::EndMenu();
				}

				ImGui::Separator();
				if(ImGui::Selectable("  Mark contacts"))
				{
					result=std::string("mark-contacts ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  Unmark contacts"))
				{
					result=std::string("unmark-contacts ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}
				ImGui::Separator();
				if(ImGui::Selectable("  Unmark all contacts"))
				{
					result=std::string("unmark-contacts ")+objects_selection_option(os_name)+"";
				}

				if(!present_contacts)
				{
					ImGui::EndDisabled();
				}
			}
			ImGui::EndPopup();
		}
	}

	void process_button_S(const std::string& os_name, std::string& result)
	{
		const std::string button_id=std::string("S##button_show_")+os_name;
		const std::string menu_id=std::string("Show##menu_show_")+os_name;
		ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
		if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
		{
			{
				ImGui::TextUnformatted("Show atoms:");

				if(ImGui::BeginMenu("  as##atoms_show_as"))
				{
					if(ImGui::MenuItem("cartoon##show_as"))
					{
						result=std::string("hide-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")\n";
						result+=std::string("show-atoms -rep cartoon ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
					if(ImGui::MenuItem("trace##show_as"))
					{
						result=std::string("hide-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")\n";
						result+=std::string("show-atoms -rep trace ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
					if(ImGui::MenuItem("sticks##show_as"))
					{
						result=std::string("hide-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")\n";
						result+=std::string("show-atoms -rep sticks ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
					if(ImGui::MenuItem("balls##show_as"))
					{
						result=std::string("hide-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")\n";
						result+=std::string("show-atoms -rep balls  ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
					if(ImGui::MenuItem("points##show_as"))
					{
						result=std::string("hide-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")\n";
						result+=std::string("show-atoms -rep points ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
					if(ImGui::MenuItem("molsurf##show_as"))
					{
						result=std::string("hide-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")\n";
						result+=std::string("show-atoms -rep molsurf ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
					if(ImGui::MenuItem("molsurf-mesh##show_as"))
					{
						result=std::string("hide-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")\n";
						result+=std::string("show-atoms -rep molsurf-mesh ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}

					ImGui::EndMenu();
				}

				if(ImGui::Selectable("  cartoon##show"))
				{
					result=std::string("show-atoms -rep cartoon ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  trace##show"))
				{
					result=std::string("show-atoms -rep trace ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  sticks##show"))
				{
					result=std::string("show-atoms -rep sticks ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  balls##show"))
				{
					result=std::string("show-atoms -rep balls ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  points##show"))
				{
					result=std::string("show-atoms -rep points ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  molsurf##show"))
				{
					result=std::string("show-atoms -rep molsurf ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  molsurf-mesh##show"))
				{
					result=std::string("show-atoms -rep molsurf-mesh ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
			}

			ImGui::Separator();

			{
				ImGui::TextUnformatted("Show added standard labels:");

				if(ImGui::Selectable("  all##labels_show"))
				{
					result=std::string("show-figures-of-labels -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					result+=std::string(" ; show-figures-of-labels -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}

				if(ImGui::Selectable("  residue-level##labels_show"))
				{
					result=std::string("show-figures-of-labels -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}

				if(ImGui::Selectable("  atom-level##labels_show"))
				{
					result=std::string("show-figures-of-labels -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
			}

			ImGui::Separator();
			ImGui::Separator();

			{
				ImGui::TextUnformatted("Show contacts:");

				if(ImGui::BeginMenu("  as##contacts_show_as"))
				{
					if(ImGui::MenuItem("faces##show_as"))
					{
						result=std::string("construct-contacts ")+objects_selection_option(os_name)+"\n";
						result+=std::string("hide-contacts ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")\n";
						result+=std::string("show-contacts -rep faces ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
					}
					if(ImGui::MenuItem("edges##show_as"))
					{
						result=std::string("construct-contacts ")+objects_selection_option(os_name)+"\n";
						result+=std::string("hide-contacts ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")\n";
						result+=std::string("show-contacts -rep edges ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
					}
					if(ImGui::MenuItem("sas-mesh##show_as"))
					{
						result=std::string("construct-contacts ")+objects_selection_option(os_name)+"\n";
						result+=std::string("hide-contacts ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")\n";
						result+=std::string("show-contacts -rep sas-mesh ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
					}

					ImGui::EndMenu();
				}

				if(ImGui::Selectable("  faces##show"))
				{
					result=std::string("construct-contacts ")+objects_selection_option(os_name)+"\n";
					result+=std::string("show-contacts -rep faces ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  edges##show"))
				{
					result=std::string("construct-contacts ")+objects_selection_option(os_name)+"\n";
					result+=std::string("show-contacts -rep edges ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  sas-mesh##show"))
				{
					result=std::string("construct-contacts ")+objects_selection_option(os_name)+"\n";
					result+=std::string("show-contacts -rep sas-mesh ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}
			}

			ImGui::EndPopup();
		}
	}

	void process_button_H(const std::string& os_name, std::string& result)
	{
		const std::string button_id=std::string("H##button_hide_")+os_name;
		const std::string menu_id=std::string("Hide##menu_hide_")+os_name;
		ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
		if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
		{
			{
				ImGui::TextUnformatted("Hide atoms:");
				if(ImGui::Selectable("  all##atoms_hide"))
				{
					result=std::string("hide-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  cartoon##atoms_hide"))
				{
					result=std::string("hide-atoms -rep cartoon ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  trace##atoms_hide"))
				{
					result=std::string("hide-atoms -rep trace ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  sticks##atoms_hide"))
				{
					result=std::string("hide-atoms -rep sticks ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  balls##atoms_hide"))
				{
					result=std::string("hide-atoms -rep balls ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  points##atoms_hide"))
				{
					result=std::string("hide-atoms -rep points ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  molsurf##atoms_hide"))
				{
					result=std::string("hide-atoms -rep molsurf ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  molsurf-mesh##atoms_hide"))
				{
					result=std::string("hide-atoms -rep molsurf-mesh ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
			}

			ImGui::Separator();

			{
				ImGui::TextUnformatted("Hide added standard labels:");

				if(ImGui::Selectable("  all##labels_hide"))
				{
					result=std::string("hide-figures-of-labels -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					result+=std::string(" ; hide-figures-of-labels -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}

				if(ImGui::Selectable("  residue-level##labels_hide"))
				{
					result=std::string("hide-figures-of-labels -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}

				if(ImGui::Selectable("  atom-level##labels_hide"))
				{
					result=std::string("hide-figures-of-labels -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
				}
			}

			ImGui::Separator();
			ImGui::Separator();

			const bool present_contacts=(os_name.empty() ? objects_info.any_object_has_contacts() : objects_info.object_has_contacts(os_name));

			if(!present_contacts)
			{
				ImGui::BeginDisabled();
			}

			{
				ImGui::TextUnformatted("Hide contacts:");
				if(ImGui::Selectable("  all##contacts_hide"))
				{
					result=std::string("hide-contacts ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  faces##contacts_hide"))
				{
					result=std::string("hide-contacts -rep faces ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  edges##contacts_hide"))
				{
					result=std::string("hide-contacts -rep edges ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}
				if(ImGui::Selectable("  sas-mesh##contacts_hide"))
				{
					result=std::string("hide-contacts -rep sas-mesh ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")";
				}
			}

			if(!present_contacts)
			{
				ImGui::EndDisabled();
			}

			ImGui::EndPopup();
		}
	}

	void process_button_Ca(const std::string& os_name, std::string& result)
	{
		const std::string button_id=std::string("Ca##button_color_atoms_")+os_name;
		const std::string menu_id=std::string("Color##menu_color_atoms_")+os_name;
		ImGui::Button(button_id.c_str(), ImVec2(21*GUIStyleWrapper::scale_factor(),0));
		if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
		{
			static bool rep_cartoon=true;
			static bool rep_trace=true;
			static bool rep_sticks=true;
			static bool rep_balls=true;
			static bool rep_points=true;
			static bool rep_molsurf=true;
			static bool rep_molsurf_mesh=true;

			ImGui::TextUnformatted("Representations of atoms:");

			{
				const std::string utility_id=std::string("check all##atoms_rep_utility1_")+os_name;
				if(ImGui::Button(utility_id.c_str()))
				{
					rep_cartoon=true;
					rep_trace=true;
					rep_sticks=true;
					rep_balls=true;
					rep_points=true;
					rep_molsurf=true;
					rep_molsurf_mesh=true;
				}
			}

			ImGui::SameLine();

			{
				const std::string utility_id=std::string("uncheck all##atoms_rep_utility2_")+os_name;
				if(ImGui::Button(utility_id.c_str()))
				{
					rep_cartoon=false;
					rep_trace=false;
					rep_sticks=false;
					rep_balls=false;
					rep_points=false;
					rep_molsurf=false;
					rep_molsurf_mesh=false;
				}
			}

			{
				const std::string checkbox_id=std::string("cartoon##cartoon_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_cartoon);
			}
			{
				const std::string checkbox_id=std::string("trace##trace_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_trace);
			}
			{
				const std::string checkbox_id=std::string("sticks##sticks_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_sticks);
			}
			{
				const std::string checkbox_id=std::string("balls##balls_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_balls);
			}
			{
				const std::string checkbox_id=std::string("points##points_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_points);
			}
			{
				const std::string checkbox_id=std::string("molsurf##molsurf_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_molsurf);
			}
			{
				const std::string checkbox_id=std::string("molsurf-mesh##molsurf_mesh_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_molsurf_mesh);
			}


			const std::string rep_string=std::string(" -rep")
				+(rep_cartoon ? " cartoon" : "")
				+(rep_trace ? " trace" : "")
				+(rep_sticks ? " sticks" : "")
				+(rep_balls ? " balls" : "")
				+(rep_points ? " points" : "")
				+(rep_molsurf ? " molsurf" : "")
				+(rep_molsurf_mesh ? " molsurf-mesh" : "");

			ImGui::Separator();

			ImGui::TextUnformatted("Spectrum atoms:");

			if(ImGui::Selectable("  by residue number"))
			{
				result=std::string("spectrum-atoms ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
			}

			if(ImGui::Selectable("  by residue ID"))
			{
				result=std::string("spectrum-atoms -by residue-id ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
			}

			if(ImGui::Selectable("  by residue ID randomly"))
			{
				result=std::string("spectrum-atoms -by residue-id -scheme random ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
			}

			if(ImGui::Selectable("  by chain"))
			{
				result=std::string("spectrum-atoms -by chain ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
			}

			if(ImGui::Selectable("  by chain randomly"))
			{
				result=std::string("spectrum-atoms -by chain -scheme random ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
			}

			if(ImGui::Selectable("  by secondary structure"))
			{
				result=std::string("spectrum-atoms -by secondary-structure ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
			}

			if(ImGui::BeginMenu("  by atom type"))
			{
				if(ImGui::MenuItem("all"))
				{
					result=std::string("spectrum-atoms -by atom-type ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}

				if(ImGui::MenuItem("all except carbon"))
				{
					result=std::string("spectrum-atoms -by atom-type ")+objects_selection_option(os_name)+" -use (("+atoms_selection_string_safe()+") and ([-t! el=C]))"+rep_string;
				}

				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu("  by B-factor"))
			{
				if(ImGui::MenuItem("blue-white-red"))
				{
					result=std::string("spectrum-atoms -adjunct tf -scheme bwr ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}

				if(ImGui::MenuItem("red-white-blue"))
				{
					result=std::string("spectrum-atoms -adjunct tf -scheme rwb ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}

				if(ImGui::MenuItem("blue-white-red, 0-100"))
				{
					result=std::string("spectrum-atoms -adjunct tf -scheme bwr -min-val 0 -max-val 100 ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}

				if(ImGui::MenuItem("red-white-blue, 0-100"))
				{
					result=std::string("spectrum-atoms -adjunct tf -scheme rwb -min-val 0 -max-val 100 ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}

				if(ImGui::MenuItem("blue-white-red, 0-1"))
				{
					result=std::string("spectrum-atoms -adjunct tf -scheme bwr -min-val 0 -max-val 1 ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}

				if(ImGui::MenuItem("red-white-blue, 0-1"))
				{
					result=std::string("spectrum-atoms -adjunct tf -scheme rwb -min-val 0 -max-val 1 ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}

				ImGui::EndMenu();
			}

			if(ImGui::Selectable("  by hydropathy"))
			{
				result=std::string("spectrum-atoms -by hydropathy ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
			}

			ImGui::Separator();

			ImGui::TextUnformatted("Color atoms:");

			if(ImGui::Selectable("  random"))
			{
				result=std::string("color-atoms -next-random-color ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  red"))
				{
					result=std::string("color-atoms -col 0xFF0000 ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  yellow"))
				{
					result=std::string("color-atoms -col 0xFFFF00 ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}
			{
				ImVec4 color_text=ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  green"))
				{
					result=std::string("color-atoms -col 0x00FF00 ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  cyan"))
				{
					result=std::string("color-atoms -col 0x00FFFF ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  blue"))
				{
					result=std::string("color-atoms -col 0x0000FF ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  magenta"))
				{
					result=std::string("color-atoms -col 0xFF00FF ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  white"))
				{
					result=std::string("color-atoms -col white ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  light gray"))
				{
					result=std::string("color-atoms -col 0xAAAAAA ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  dark gray"))
				{
					result=std::string("color-atoms -col 0x555555 ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			ImGui::Separator();

			if(ImGui::BeginMenu("Color added standard labels##labels_color"))
			{
				static bool mode_residue=true;
				static bool mode_atom=true;

				{
					const std::string checkbox_id=std::string("residue-level##labels_color_checkbox_residue_")+os_name;
					ImGui::Checkbox(checkbox_id.c_str(), &mode_residue);
				}
				{
					const std::string checkbox_id=std::string("atom-level##labels_color_checkbox_atom_")+os_name;
					ImGui::Checkbox(checkbox_id.c_str(), &mode_atom);
				}

				ImGui::Separator();

				if(ImGui::MenuItem("  same as atoms"))
				{
					result="";
					if(mode_residue)
					{
						result+=std::string("color-figures-of-labels -color-for-text _of_atom -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						result+=(mode_atom ? "\n" : "");
					}
					if(mode_atom)
					{
						result+=std::string("color-figures-of-labels -color-for-text _of_atom -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
				}

				{
					ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					if(ImGui::Selectable("  red"))
					{
						result="";
						if(mode_residue)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0xFF0000 -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
							result+=(mode_atom ? "\n" : "");
						}
						if(mode_atom)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0xFF0000 -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						}
					}
					ImGui::PopStyleColor();
				}

				{
					ImVec4 color_text=ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					if(ImGui::Selectable("  yellow"))
					{
						result="";
						if(mode_residue)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0xFFFF00 -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
							result+=(mode_atom ? "\n" : "");
						}
						if(mode_atom)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0xFFFF00 -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						}
					}
					ImGui::PopStyleColor();
				}

				{
					ImVec4 color_text=ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					if(ImGui::Selectable("  green"))
					{
						result="";
						if(mode_residue)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0x00FF00 -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
							result+=(mode_atom ? "\n" : "");
						}
						if(mode_atom)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0x00FF00 -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						}
					}
					ImGui::PopStyleColor();
				}

				{
					ImVec4 color_text=ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					if(ImGui::Selectable("  cyan"))
					{
						result="";
						if(mode_residue)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0x00FFFF -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
							result+=(mode_atom ? "\n" : "");
						}
						if(mode_atom)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0x00FFFF -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						}
					}
					ImGui::PopStyleColor();
				}

				{
					ImVec4 color_text=ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					if(ImGui::Selectable("  blue"))
					{
						result="";
						if(mode_residue)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0x0000FF -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
							result+=(mode_atom ? "\n" : "");
						}
						if(mode_atom)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0x0000FF -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						}
					}
					ImGui::PopStyleColor();
				}

				{
					ImVec4 color_text=ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					if(ImGui::Selectable("  magenta"))
					{
						result="";
						if(mode_residue)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0xFF00FF -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
							result+=(mode_atom ? "\n" : "");
						}
						if(mode_atom)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0xFF00FF -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						}
					}
					ImGui::PopStyleColor();
				}

				{
					ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					if(ImGui::Selectable("  white"))
					{
						result="";
						if(mode_residue)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0xFFFFFF -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
							result+=(mode_atom ? "\n" : "");
						}
						if(mode_atom)
						{
							result+=std::string("color-figures-of-labels -color-for-text 0xFFFFFF -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						}
					}
					ImGui::PopStyleColor();
				}

				ImGui::Separator();

				ImGui::TextUnformatted("Outline color:");

				if(ImGui::MenuItem("  same as atoms "))
				{
					result="";
					if(mode_residue)
					{
						result+=std::string("color-figures-of-labels -color-for-outline _of_atom -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						result+=(mode_atom ? "\n" : "");
					}
					if(mode_atom)
					{
						result+=std::string("color-figures-of-labels -color-for-outline _of_atom -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
				}

				if(ImGui::Selectable("  black "))
				{
					result="";
					if(mode_residue)
					{
						result+=std::string("color-figures-of-labels -color-for-outline 0x000000 -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						result+=(mode_atom ? "\n" : "");
					}
					if(mode_atom)
					{
						result+=std::string("color-figures-of-labels -color-for-outline 0x000000 -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
				}

				if(ImGui::Selectable("  gray "))
				{
					result="";
					if(mode_residue)
					{
						result+=std::string("color-figures-of-labels -color-for-outline 0x555555 -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						result+=(mode_atom ? "\n" : "");
					}
					if(mode_atom)
					{
						result+=std::string("color-figures-of-labels -color-for-outline 0x555555 -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
				}

				if(ImGui::Selectable("  white "))
				{
					result="";
					if(mode_residue)
					{
						result+=std::string("color-figures-of-labels -color-for-outline 0xFFFFFF -mode residue ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
						result+=(mode_atom ? "\n" : "");
					}
					if(mode_atom)
					{
						result+=std::string("color-figures-of-labels -color-for-outline 0xFFFFFF -mode atom ")+objects_selection_option(os_name)+" -use ("+atoms_selection_string_safe()+")";
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}
	}

	void process_button_Cc(const std::string& os_name, std::string& result)
	{
		const std::string button_id=std::string("Cc##button_color_contacts_")+os_name;
		const std::string menu_id=std::string("Color##menu_color_contacts_")+os_name;
		ImGui::Button(button_id.c_str(), ImVec2(21*GUIStyleWrapper::scale_factor(),0));
		if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
		{
			static bool rep_faces=true;
			static bool rep_edges=true;
			static bool rep_sas_mesh=true;

			ImGui::TextUnformatted("Representations of contacts:");

			{
				const std::string utility_id=std::string("check all##contacts_rep_utility1_")+os_name;
				if(ImGui::Button(utility_id.c_str()))
				{
					rep_faces=true;
					rep_edges=true;
					rep_sas_mesh=true;
				}
			}

			ImGui::SameLine();

			{
				const std::string utility_id=std::string("uncheck all##contacts_rep_utility2_")+os_name;
				if(ImGui::Button(utility_id.c_str()))
				{
					rep_faces=false;
					rep_edges=false;
					rep_sas_mesh=false;
				}
			}

			{
				const std::string checkbox_id=std::string("faces##faces_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_faces);
			}
			{
				const std::string checkbox_id=std::string("edges##edges_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_edges);
			}
			{
				const std::string checkbox_id=std::string("sas-mesh##sas_mesh_checkbox_rep_")+os_name;
				ImGui::Checkbox(checkbox_id.c_str(), &rep_sas_mesh);
			}

			const std::string rep_string=std::string(" -rep")
				+(rep_faces ? " faces" : "")
				+(rep_edges ? " edges" : "")
				+(rep_sas_mesh ? " sas-mesh" : "");

			ImGui::Separator();

			ImGui::TextUnformatted("Spectrum contacts:");

			if(ImGui::Selectable("  by inter-residue ID randomly"))
			{
				result=std::string("spectrum-contacts -by residue-ids -scheme random ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
			}

			if(ImGui::Selectable("  by inter-residue area, 0-45"))
			{
				result=std::string("spectrum-contacts -by residue-area -min-val 0 -max-val 45 ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
			}

			if(ImGui::Selectable("  by inter-atom area, 0-15"))
			{
				result=std::string("spectrum-contacts -by area -min-val 0 -max-val 15 ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
			}

			ImGui::Separator();

			ImGui::TextUnformatted("Color contacts:");

			if(ImGui::Selectable("  random"))
			{
				result=std::string("color-contacts -next-random-color ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  red"))
				{
					result=std::string("color-contacts -col 0xFF0000 ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  yellow"))
				{
					result=std::string("color-contacts -col 0xFFFF00 ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}
			{
				ImVec4 color_text=ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  green"))
				{
					result=std::string("color-contacts -col 0x00FF00 ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  cyan"))
				{
					result=std::string("color-contacts -col 0x00FFFF ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  blue"))
				{
					result=std::string("color-contacts -col 0x0000FF ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  magenta"))
				{
					result=std::string("color-contacts -col 0xFF00FF ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  white"))
				{
					result=std::string("color-contacts -col white ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  light gray"))
				{
					result=std::string("color-contacts -col 0xAAAAAA ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			{
				ImVec4 color_text=ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				if(ImGui::Selectable("  dark gray"))
				{
					result=std::string("color-contacts -col 0x555555 ")+objects_selection_option(os_name)+" -use ("+contacts_selection_string_safe()+")"+rep_string;
				}
				ImGui::PopStyleColor();
			}

			ImGui::EndPopup();
		}
	}

	bool need_atoms_unmark_button_;
	bool need_contacts_unmark_button_;

	bool current_atoms_selection_not_default_;
	bool current_contacts_selection_not_default_;
	bool current_atoms_selection_marked_;
	bool current_contacts_selection_marked_;

	std::vector<char> atoms_selection_buffer_;
	std::vector<char> contacts_selection_buffer_;

	std::string default_atoms_selection_string_;
	std::string marked_atoms_selection_string_;
	std::string atoms_selection_string_;
	std::string atoms_selection_string_previous_;

	std::string default_contacts_selection_string_;
	std::string marked_contacts_selection_string_;
	std::string contacts_selection_string_;
	std::string contacts_selection_string_previous_;

	std::pair< std::deque<std::string>, std::deque<std::string> > atoms_selection_string_suggestions_;
	std::pair< std::deque<std::string>, std::deque<std::string> > contacts_selection_string_suggestions_;
};

}

}

}

#endif /* VIEWER_CONSOLE_OBJECT_LIST_VIEWER_STATE_H_ */
