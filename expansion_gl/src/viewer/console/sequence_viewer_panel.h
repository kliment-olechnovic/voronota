#ifndef VIEWER_CONSOLE_SEQUENCE_VIEWER_STATE_H_
#define VIEWER_CONSOLE_SEQUENCE_VIEWER_STATE_H_

#include "objects_info.h"

#include "common_for_gui.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class SequenceViewerPanel
{
public:
	const ObjectsInfo& objects_info;
	bool& visible;
	int max_slots;
	int max_visible_slots;
	float sequence_frame_height;
	float button_width_unit;

	SequenceViewerPanel(const ObjectsInfo& objects_info) :
		objects_info(objects_info),
		visible(GUIConfiguration::instance().enabled_sequence_view),
		max_slots(100),
		max_visible_slots(5),
		sequence_frame_height(40.0f),
		button_width_unit(10.0f)
	{
	}

	float calc_total_container_height() const
	{
		if(!visible)
		{
			return 0.0f;
		}
		return static_cast<float>(std::min(objects_info.num_of_visible_objects(), std::min(max_visible_slots, max_slots))*sequence_frame_height)*GUIStyleWrapper::scale_factor();
	}

	float calc_name_column_width(const float total_width) const
	{
		const std::vector<ObjectsInfo::ObjectState>& object_states=objects_info.get_object_states();

		std::size_t max_name_size=1;

		{
			int used_slots=0;
			for(std::size_t i=0;used_slots<max_slots && i<object_states.size();i++)
			{
				if(object_states[i].visible)
				{
					used_slots++;
					max_name_size=std::max(max_name_size, object_states[i].name.size());
				}
			}
		}

		const float width=std::min(static_cast<float>(max_name_size)*7.0f+5.0f, total_width*0.2f);

		return width*GUIStyleWrapper::scale_factor();
	}

	void execute(std::string& result) const
	{
		if(!visible)
		{
			return;
		}

		if(objects_info.num_of_visible_objects()<1)
		{
			return;
		}

		const std::vector<ObjectsInfo::ObjectState>& object_states=objects_info.get_object_states();
		const std::map<std::string, ObjectsInfo::ObjectDetails>& object_details=objects_info.get_object_details();

		const float total_container_height=calc_total_container_height();
		const float names_frame_width=calc_name_column_width(ImGui::GetWindowWidth());

		static std::vector< std::pair< std::pair<std::string, int>, int> > dragged_buttons;

		ImGui::BeginChild("##sequence_view_container", ImVec2(0, total_container_height), false);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f*GUIStyleWrapper::scale_factor(), 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f*GUIStyleWrapper::scale_factor(), 0.0f));

		int used_slots=0;
		int used_buttons=0;
		bool any_button_held=false;
		bool any_button_not_held_but_hovered=false;
		for(std::size_t i=0;used_slots<max_slots && i<object_states.size();i++)
		{
			if(object_states[i].visible)
			{
				used_slots++;

				{
					const std::string region_id=std::string("##name_of_sequence_scrolling_region_")+object_states[i].name;
					ImGui::BeginChild(region_id.c_str(), ImVec2(names_frame_width, sequence_frame_height*GUIStyleWrapper::scale_factor()), false, ImGuiWindowFlags_HorizontalScrollbar);
					ImGui::TextUnformatted(object_states[i].name.c_str());
					ImGui::EndChild();
				}

				ImGui::SameLine();

				{
					ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

					const std::string region_id=std::string("##sequence_scrolling_region_")+object_states[i].name;
					ImGui::BeginChild(region_id.c_str(), ImVec2(0, sequence_frame_height*GUIStyleWrapper::scale_factor()), true, ImGuiWindowFlags_HorizontalScrollbar|ImGuiWindowFlags_AlwaysHorizontalScrollbar);

					std::map<std::string, ObjectsInfo::ObjectDetails>::const_iterator details_it=object_details.find(object_states[i].name);
					if(details_it!=object_details.end())
					{
						const ObjectsInfo::ObjectSequenceInfo& sequence=details_it->second.sequence;

						for(std::size_t j=0;j<sequence.chains.size();j++)
						{
							if(j>0)
							{
								ImGui::SameLine();
								ImGui::Dummy(ImVec2(button_width_unit*GUIStyleWrapper::scale_factor(), 0.0f));
								ImGui::SameLine();
							}
							if(!sequence.chains[j].name.empty())
							{
								char button_label[64];
								snprintf(button_label, 64, "%s##chain_button_%d", sequence.chains[j].name.c_str(), used_buttons++);
								if(ImGui::Button(button_label, ImVec2(button_width_unit*GUIStyleWrapper::scale_factor()*static_cast<float>(sequence.chains[j].name.size()), 0.0f)))
								{
									if(!sequence.chains[j].residues.empty())
									{
										result=std::string(sequence.chains[j].residues[0].marked ? "unmark-atoms" : "mark-atoms")
											+" -on-objects "+object_states[i].name
											+" -use ["
											+" -chain "+sequence.chains[j].name
											+"]";
									}
								}
								ImGui::SameLine();
								ImGui::Dummy(ImVec2(button_width_unit*GUIStyleWrapper::scale_factor(), 0.0f));
								ImGui::SameLine();
							}
							for(std::size_t e=0;e<sequence.chains[j].residues.size();e++)
							{
								if(e>0)
								{
									ImGui::SameLine();
									if(sequence.chains[j].residues[e].num!=sequence.chains[j].residues[e-1].num+1)
									{
										ImGui::Dummy(ImVec2(button_width_unit*GUIStyleWrapper::scale_factor(), 0.0f));
										ImGui::SameLine();
									}
								}
								const ObjectsInfo::ObjectSequenceInfo::ResidueInfo& residue=sequence.chains[j].residues[e];
								char button_label[64];
								snprintf(button_label, 64, "%s##seq_num_button_%d", residue.num_label.c_str(), used_buttons++);
								if(ImGui::Button(button_label, ImVec2(button_width_unit*GUIStyleWrapper::scale_factor()*static_cast<float>(residue.display_size()), 0.0f)))
								{
									result=std::string("vsb: ")+(residue.marked ? "unmark-atoms" : "mark-atoms")
										+" -on-objects "+object_states[i].name
										+" -use ["
										+(sequence.chains[j].name.empty() ? std::string() : std::string(" -chain ")+sequence.chains[j].name)
										+" -rnum "+std::to_string(residue.num)
										+"]";
								}
							}
						}

						for(std::size_t j=0;j<sequence.chains.size();j++)
						{
							if(j>0)
							{
								ImGui::SameLine();
								ImGui::Dummy(ImVec2(button_width_unit*GUIStyleWrapper::scale_factor(), 0.0f));
								ImGui::SameLine();
							}
							if(!sequence.chains[j].name.empty())
							{
								ImGui::Dummy(ImVec2(button_width_unit*GUIStyleWrapper::scale_factor()*static_cast<float>(sequence.chains[j].name.size()), 0.0f));
								ImGui::SameLine();
								ImGui::Dummy(ImVec2(button_width_unit*GUIStyleWrapper::scale_factor(), 0.0f));
								ImGui::SameLine();
							}
							for(std::size_t e=0;e<sequence.chains[j].residues.size();e++)
							{
								if(e>0)
								{
									ImGui::SameLine();
									if(sequence.chains[j].residues[e].num!=sequence.chains[j].residues[e-1].num+1)
									{
										ImGui::Dummy(ImVec2(button_width_unit*GUIStyleWrapper::scale_factor(), 0.0f));
										ImGui::SameLine();
									}
								}
								const ObjectsInfo::ObjectSequenceInfo::ResidueInfo& residue=sequence.chains[j].residues[e];
								char button_label[64];
								snprintf(button_label, 64, "%s##seq_button_%d", residue.name.c_str(), used_buttons++);

								if(residue.marked)
								{
									ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(residue.rgb[0]*0.8f, residue.rgb[1]*0.8f, residue.rgb[2]*0.8f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(residue.rgb[0],      residue.rgb[1],      residue.rgb[2],      1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(residue.rgb[0]*0.9f, residue.rgb[1]*0.9f, residue.rgb[2]*0.9f, 1.0f));
								}
								else
								{
									ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(residue.rgb[0], residue.rgb[1], residue.rgb[2], 1.0f));
								}

								ImGui::Button(button_label, ImVec2(button_width_unit*GUIStyleWrapper::scale_factor()*static_cast<float>(residue.display_size()), 0.0f));

								bool action_needed=false;

								if(ImGui::IsItemActive() && !any_button_held)
								{
									any_button_held=true;
									std::pair< std::pair<std::string, int>, int> active_button_id(std::pair<std::string, int>(object_states[i].name, j), e);
									if(dragged_buttons.empty() || dragged_buttons.front()!=active_button_id)
									{
										dragged_buttons.clear();
										dragged_buttons.push_back(active_button_id);
										action_needed=true;
									}
								}
								else if(ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && !any_button_not_held_but_hovered)
								{
									any_button_not_held_but_hovered=true;
									std::pair< std::pair<std::string, int>, int> hovered_button_id(std::pair<std::string, int>(object_states[i].name, j), e);
									if(!dragged_buttons.empty() && hovered_button_id.first==dragged_buttons.back().first)
									{
										if(hovered_button_id.second!=dragged_buttons.back().second)
										{
											dragged_buttons.push_back(hovered_button_id);
											action_needed=true;
										}
									}
								}

								if(action_needed)
								{
									result="vsb: \n";

									static std::vector<int> ids_for_action;
									ids_for_action.clear();
									ids_for_action.push_back(e);

									if(dragged_buttons.size()>1)
									{
										const std::size_t p=(dragged_buttons.size()-1);

										if(dragged_buttons.size()>3)
										{
											if((dragged_buttons[p-1].second>dragged_buttons[0].second && dragged_buttons[p-1].second>dragged_buttons[p-2].second && dragged_buttons[p-1].second>dragged_buttons[p].second)
														|| (dragged_buttons[p-1].second<dragged_buttons[0].second && dragged_buttons[p-1].second<dragged_buttons[p-2].second && dragged_buttons[p-1].second<dragged_buttons[p].second))
											{
												ids_for_action.push_back(dragged_buttons[p-1].second);
											}
										}
										else if(dragged_buttons.size()>2)
										{
											if(((dragged_buttons[p-1].second>dragged_buttons[0].second && dragged_buttons[p-1].second>dragged_buttons[p].second)
													|| (dragged_buttons[p-1].second<dragged_buttons[0].second && dragged_buttons[p-1].second<dragged_buttons[p].second)))
											{
												ids_for_action.push_back(dragged_buttons[p-1].second);
											}
										}

										if((dragged_buttons[p].second-dragged_buttons[p-1].second)>1)
										{
											for(int l=(dragged_buttons[p-1].second+1);l<dragged_buttons[p].second;l++)
											{
												ids_for_action.push_back(l);
											}
										}
										else if((dragged_buttons[p-1].second-dragged_buttons[p].second)>1)
										{
											for(int l=(dragged_buttons[p].second+1);l<dragged_buttons[p-1].second;l++)
											{
												ids_for_action.push_back(l);
											}
										}
									}

									{
										std::string rnum_strings[2];
										for(std::size_t l=0;l<ids_for_action.size();l++)
										{
											const ObjectsInfo::ObjectSequenceInfo::ResidueInfo& sresidue=sequence.chains[j].residues[ids_for_action[l]];
											std::string& rnum_string=rnum_strings[sresidue.marked ? 0 : 1];
											if(!rnum_string.empty())
											{
												rnum_string+=",";
											}
											rnum_string+=std::to_string(sresidue.num);
										}
										for(int m=0;m<2;m++)
										{
											if(!rnum_strings[m].empty())
											{
												result+=std::string(m==0 ? "unmark-atoms" : "mark-atoms")
													+" -on-objects "+object_states[i].name
													+" -use ["
													+(sequence.chains[j].name.empty() ? std::string() : std::string(" -chain ")+sequence.chains[j].name)
													+" -rnum "+rnum_strings[m]
													+"]\n"
													+"clear-last\n";
											}
										}
									}
								}

								if(residue.marked)
								{
									ImGui::PopStyleColor(3);
								}
								else
								{
									ImGui::PopStyleColor(1);
								}
							}
						}
					}

					ImGui::EndChild();

					ImGui::PopStyleColor(4);
				}
			}
		}

		ImGui::PopStyleVar(3);

		ImGui::EndChild();

		if(!any_button_held)
		{
			dragged_buttons.clear();
		}
	}
};


}

}

}


#endif /* VIEWER_CONSOLE_SEQUENCE_VIEWER_STATE_H_ */
