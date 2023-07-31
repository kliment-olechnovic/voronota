#ifndef VIEWER_CONSOLE_H_
#define VIEWER_CONSOLE_H_

#include <string>
#include <list>
#include <vector>
#include <deque>

#include "../dependencies/imgui/addons/global_text_color_vector.h"
#include "../dependencies/imgui/addons/simple_splitter.h"

#ifndef FOR_WEB
#include "../dependencies/ImGuiFileDialog/ImGuiFileDialog.h"
#endif

#include "../dependencies/ImGuiColorTextEdit/TextEditor.h"

#include "../uv/viewer_application.h"
#include "../../../expansion_js/src/duktaper/stocked_data_resources.h"

#include "gui_style_wrapper.h"
#include "gui_configuration.h"

namespace voronota
{

namespace viewer
{

class Console
{
public:
	struct OutputToken
	{
		float r;
		float g;
		float b;
		std::string content;
		std::vector<unsigned int> char_colors;

		OutputToken(const std::string& content, const float r, const float g, const float b) : r(r), g(g), b(b), content(content)
		{
			const ImU32 cui_default=0xFFAAAAAA;
			const ImU32 cui_string=ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.0f));
			const ImU32 cui_number=0xFF55EE55;
			const ImU32 cui_key=0xFF55EEEE;
			const ImU32 cui_punctuation=0xFFCCCCCC;

			char_colors.resize(content.size(), cui_default);

			{
				int i=0;
				while(i<static_cast<int>(content.size()))
				{
					if(content[i]=='"')
					{
						char_colors[i]=cui_string;
						i++;
						bool ended=false;
						while(i<static_cast<int>(content.size()) && !ended)
						{
							if(content[i]=='"' && content[i-1]!='\\')
							{
								ended=true;
							}
							char_colors[i]=cui_string;
							i++;
						}
					}
					else
					{
						i++;
					}
				}
			}

			{
				int i=0;
				while(i<static_cast<int>(content.size()))
				{
					if(char_colors[i]==cui_default && ((content[i]>='0' && content[i]<='9') || (content[i]=='-' && (i+1)<static_cast<int>(content.size()) && char_colors[i+1]==cui_default && content[i+1]>='0' && content[i+1]<='9')))
					{
						char_colors[i]=cui_number;
						i++;
						bool ended=false;
						while(i<static_cast<int>(content.size()) && !ended)
						{
							if(char_colors[i]==cui_default && ((content[i]>='0' && content[i]<='9') || content[i]=='.'))
							{
								char_colors[i]=cui_number;
							}
							else
							{
								ended=true;
							}
							i++;
						}
					}
					else
					{
						i++;
					}
				}
			}

			for(int i=0;i<static_cast<int>(content.size());i++)
			{
				if(i>=2 && content[i]==':' && i>0 && content[i-1]=='"')
				{
					char_colors[i]=cui_punctuation;
					char_colors[i-1]=cui_key;
					int j=(i-2);
					for(;j>=0;j--)
					{
						char_colors[j]=cui_key;
						if(content[j]=='"')
						{
							j=-1;
						}
					}
				}
			}

			for(int i=0;i<static_cast<int>(content.size());i++)
			{
				if(char_colors[i]==cui_default)
				{
					const char c=content[i];
					if(c=='{' || c=='}' || c=='[' || c==']' || c==',')
					{
						char_colors[i]=cui_punctuation;
					}
				}
			}
		}
	};

	class ObjectsInfo
	{
	public:
		struct ObjectState
		{
			std::string name;
			bool picked;
			bool visible;
		};

		struct ObjectSequenceInfo
		{
			struct ResidueInfo
			{
				std::string name;
				std::string num_label;
				int num;
				bool marked;
				float rgb[3];

				ResidueInfo() : num(0), marked(false)
				{
				}

				std::size_t display_size() const
				{
					return std::max(name.size(), num_label.size());
				}
			};

			struct ChainInfo
			{
				std::string name;
				std::vector<ResidueInfo> residues;
			};

			std::vector<ChainInfo> chains;

			bool empty() const
			{
				return chains.empty();
			}
		};

		struct ObjectDetails
		{
			ObjectSequenceInfo sequence;
		};

		ObjectsInfo() : num_of_picked_objects_(0), num_of_visible_objects_(0)
		{
		}

		const std::vector<ObjectState>& get_object_states() const
		{
			return object_states_;
		}

		const std::map<std::string, ObjectDetails>& get_object_details() const
		{
			return object_details_;
		}

		int num_of_picked_objects() const
		{
			return num_of_picked_objects_;
		}

		int num_of_visible_objects() const
		{
			return num_of_visible_objects_;
		}

		bool object_has_details(const std::string& name) const
		{
			return (object_details_.count(name)>0);
		}

		void set_object_states(const std::vector<ObjectState>& object_states, const bool preserve_details_if_all_names_match)
		{
			if(object_states.empty())
			{
				object_details_.clear();
				object_states_.clear();
			}
			else
			{
				if(preserve_details_if_all_names_match)
				{
					bool same_names=(object_states.size()==object_states_.size());
					for(std::size_t i=0;same_names && i<object_states.size();i++)
					{
						same_names=(same_names && object_states[i].name==object_states_[i].name);
					}
					if(!same_names)
					{
						object_details_.clear();
					}
				}
				else
				{
					object_details_.clear();
				}
				object_states_=object_states;
			}
			count_objects();
		}

		void set_object_sequence_info(const std::string& name, const ObjectSequenceInfo& sequence)
		{
			bool found_name=false;
			for(std::size_t i=0;!found_name && i<object_states_.size();i++)
			{
				found_name=(found_name || name==object_states_[i].name);
			}
			if(found_name)
			{
				object_details_[name].sequence=sequence;
			}
		}
	private:
		void count_objects()
		{
			num_of_picked_objects_=0;
			num_of_visible_objects_=0;
			for(std::size_t i=0;i<object_states_.size();i++)
			{
				const ObjectState& os=object_states_[i];
				if(os.picked)
				{
					num_of_picked_objects_++;
				}
				if(os.visible)
				{
					num_of_visible_objects_++;
				}
			}
		}

		std::vector<ObjectState> object_states_;
		std::map<std::string, ObjectDetails> object_details_;
		int num_of_picked_objects_;
		int num_of_visible_objects_;
	};

	static Console& instance()
	{
		static Console console;
		return console;
	}

	ObjectsInfo& objects_info()
	{
		return objects_info_;
	}

	void set_need_keyboard_focus_in_command_input(const bool status)
	{
		command_line_interface_state_.need_keyboard_focus_in_command_input=(status && !script_editor_state_.focused);
	}

	void set_next_prefix(const std::string& prefix)
	{
		command_line_interface_state_.next_prefix=prefix;
	}

	void add_output(const std::string& content, const float r, const float g, const float b)
	{
		command_line_interface_state_.outputs.push_back(OutputToken(content, r, g, b));
		if(command_line_interface_state_.outputs.size()>50)
		{
			command_line_interface_state_.outputs.pop_front();
		}
		command_line_interface_state_.scroll_output=true;
	}

	void add_output_separator()
	{
		if(!command_line_interface_state_.outputs.empty() && command_line_interface_state_.outputs.back().content!=separator_string())
		{
			add_output(separator_string(), 0.0f, 0.0f, 0.0f);
		}
	}

	void add_history_output(const std::size_t n)
	{
		if(!command_line_interface_state_.history_of_commands.empty())
		{
			const std::size_t first_i=((n>0 && n<command_line_interface_state_.history_of_commands.size()) ? (command_line_interface_state_.history_of_commands.size()-n) : 0);
			std::ostringstream output;
			for(std::size_t i=first_i;i<command_line_interface_state_.history_of_commands.size();i++)
			{
				output << command_line_interface_state_.history_of_commands[i] << "\n";
			}
			add_output(output.str(), 0.75f, 0.50f, 0.0f);
		}
	}

	void clear_outputs()
	{
		command_line_interface_state_.outputs.clear();
		command_line_interface_state_.scroll_output=true;
	}

	void clear_last_output()
	{
		if(!command_line_interface_state_.outputs.empty() && command_line_interface_state_.outputs.back().content==separator_string())
		{
			command_line_interface_state_.outputs.pop_back();
		}
		if(!command_line_interface_state_.outputs.empty())
		{
			command_line_interface_state_.outputs.pop_back();
		}
		command_line_interface_state_.scroll_output=true;
	}

	void set_documentation(const std::map<std::string, std::string>& documentation)
	{
		documentation_viewer_state_.documentation=documentation;
	}

	void shrink_to_minimal_view()
	{
		if(GUIConfiguration::instance().enabled_console)
		{
			shrink_to_minimal_view_=true;
		}
	}

	std::string execute(
			const int x_pos, const int y_pos,
			const int recommended_width,
			const int recommended_height,
			const int min_width, const int max_width,
			const int min_height, const int max_height)
	{
		std::string result;

		if(!GUIConfiguration::instance().enabled_console)
		{
			return result;
		}

#ifndef FOR_WEB
		static bool menu_open_include_heteroatoms=true;
		static bool menu_open_as_assembly=false;
		static bool menu_open_split_pdb_files=false;
#endif
		{
			current_menu_bar_height_=0.0f;
			if(ImGui::BeginMainMenuBar())
			{
#ifndef FOR_WEB
				if(ImGui::BeginMenu("File"))
				{
					if(ImGui::BeginMenu("Structures"))
					{
						if(ImGui::MenuItem("Open ..."))
						{
							ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey_OpenFiles", "Open files", ".*,.pdb,.cif", file_search_root_dir_, 0, 0, ImGuiFileDialogFlags_Modal|ImGuiFileDialogFlags_DontShowHiddenFiles|ImGuiFileDialogFlags_DisableCreateDirectoryButton);
						}
						if(ImGui::BeginMenu("Fetch"))
						{
							static std::vector<char> pdbid_buffer;
							if(pdbid_buffer.empty())
							{
								const std::string example("2zsk");
								pdbid_buffer=std::vector<char>(example.begin(), example.end());
								pdbid_buffer.resize(128, 0);
							}
							const std::string textbox_id=std::string("##pdbid");
							bool requested=false;
							ImGui::PushItemWidth(70*GUIStyleWrapper::scale_factor());
							if(ImGui::InputText(textbox_id.c_str(), pdbid_buffer.data(), pdbid_buffer.size()-1, ImGuiInputTextFlags_EnterReturnsTrue))
							{
								requested=true;
							}
							ImGui::PopItemWidth();
							ImGui::SameLine();
							{
								const std::string button_id=std::string("fetch");
								if(ImGui::Button(button_id.c_str()))
								{
									requested=true;
								}
							}
							if(requested && pdbid_buffer.data()[0]!=0)
							{
								const std::string pdbid_str(pdbid_buffer.data());
								result="fetch ";
								result+=pdbid_str;
								if(!menu_open_include_heteroatoms)
								{
									result+=" -no-heteroatoms ";
								}
								if(menu_open_split_pdb_files)
								{
									result+=" -all-states ";
								}
							}
							ImGui::EndMenu();
						}
						ImGui::Separator();
						{
							const std::string checkbox_id=std::string("include heteroatoms");
							ImGui::Checkbox(checkbox_id.c_str(), &menu_open_include_heteroatoms);
						}
						{
							const std::string checkbox_id=std::string("load assemblies from PDB files");
							ImGui::Checkbox(checkbox_id.c_str(), &menu_open_as_assembly);
							menu_open_split_pdb_files=(menu_open_split_pdb_files && !menu_open_as_assembly);
						}
						{
							const std::string checkbox_id=std::string("split PDB files by models");
							ImGui::Checkbox(checkbox_id.c_str(), &menu_open_split_pdb_files);
							menu_open_as_assembly=(menu_open_as_assembly && !menu_open_split_pdb_files);
						}
						ImGui::EndMenu();
					}
					ImGui::Separator();
					if(ImGui::BeginMenu("Session"))
					{
						if(ImGui::MenuItem("Open ..."))
						{
							ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey_ImportSession", "Import session file", ".vses", file_search_root_dir_, 1, 0, ImGuiFileDialogFlags_Modal|ImGuiFileDialogFlags_DontShowHiddenFiles|ImGuiFileDialogFlags_DisableCreateDirectoryButton);
						}
						if(ImGui::MenuItem("Save ..."))
						{
							ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey_ExportSession", "Export session file", ".vses", file_search_root_dir_, 1, 0, ImGuiFileDialogFlags_Modal|ImGuiFileDialogFlags_DontShowHiddenFiles|ImGuiFileDialogFlags_ConfirmOverwrite);
						}
						ImGui::EndMenu();
					}
					ImGui::Separator();
					if(ImGui::MenuItem("Exit"))
					{
						result="exit";
					}
					ImGui::EndMenu();
				}
#endif
				if(ImGui::BeginMenu("Help"))
				{
					if(ImGui::MenuItem("About"))
					{
						result="about";
					}
					ImGui::EndMenu();
				}

				current_menu_bar_height_=ImGui::GetWindowHeight();
				ImGui::EndMainMenuBar();
			}
		}

#ifndef FOR_WEB
		{
			ImVec2 file_dialog_min_size(600*GUIStyleWrapper::scale_factor(), 450*GUIStyleWrapper::scale_factor());
			ImVec2 file_dialog_max_size(800*GUIStyleWrapper::scale_factor(), 600*GUIStyleWrapper::scale_factor());

			if(ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey_OpenFiles", ImGuiWindowFlags_NoCollapse, file_dialog_min_size, file_dialog_max_size))
			{
				if(ImGuiFileDialog::Instance()->IsOk())
				{
					file_search_root_dir_=ImGuiFileDialog::Instance()->GetCurrentPath()+"/";
					const std::map<std::string, std::string> file_paths=ImGuiFileDialog::Instance()->GetSelection();
					if(!file_paths.empty())
					{
						result="";
						for(std::map<std::string, std::string>::const_iterator it=file_paths.begin();it!=file_paths.end();++it)
						{
							result+="import-many -files ";
							result+=" '";
							result+=it->second;
							result+="' ";
							if(menu_open_include_heteroatoms)
							{
								result+=" -include-heteroatoms ";
							}
							if(menu_open_as_assembly)
							{
								result+=" -as-assembly ";
							}
							if(menu_open_split_pdb_files)
							{
								result+=" -split-pdb-files ";
							}
							result+="\n";
						}
					}
				}
				ImGuiFileDialog::Instance()->Close();
			}

			if(ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey_ImportSession", ImGuiWindowFlags_NoCollapse, file_dialog_min_size, file_dialog_max_size))
			{
				if(ImGuiFileDialog::Instance()->IsOk())
				{
					file_search_root_dir_=ImGuiFileDialog::Instance()->GetCurrentPath()+"/";
					const std::string file_path=ImGuiFileDialog::Instance()->GetFilePathName();
					result=std::string("import-session '")+file_path+"'";
				}
				ImGuiFileDialog::Instance()->Close();
			}

			if(ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey_ExportSession", ImGuiWindowFlags_NoCollapse, file_dialog_min_size, file_dialog_max_size))
			{
				if(ImGuiFileDialog::Instance()->IsOk())
				{
					file_search_root_dir_=ImGuiFileDialog::Instance()->GetCurrentPath()+"/";
					const std::string file_path=ImGuiFileDialog::Instance()->GetFilePathName();
					result=std::string("export-session '")+file_path+"'";
				}
				ImGuiFileDialog::Instance()->Close();
			}
		}
#endif

		{
			const float actual_min_height=std::max(static_cast<float>(min_height), (command_line_interface_state_.height_for_command_line+15.0f)*GUIStyleWrapper::scale_factor()+sequence_viewer_state_.calc_total_container_height());
			const float actual_max_height=std::max(0.0f, static_cast<float>(max_height)-current_menu_bar_height_);

			ImGui::SetNextWindowPos(ImVec2(x_pos, y_pos+current_menu_bar_height_));
			ImGui::SetNextWindowSizeConstraints(ImVec2(min_width, actual_min_height), ImVec2(max_width, actual_max_height));

			if(shrink_to_minimal_view_)
			{
				GUIConfiguration::instance().hint_render_area_width=-1;
				GUIConfiguration::instance().hint_render_area_height=-1;
			}

			if(GUIConfiguration::instance().hint_render_area_width>0 || GUIConfiguration::instance().hint_render_area_height>0)
			{
				int hint_width=GUIConfiguration::instance().hint_render_area_width;
				if(hint_width<=0)
				{
					hint_width=(current_width_>0.0f ? static_cast<int>(current_width_) : recommended_width);
				}
				else
				{
					hint_width=std::min(std::max(min_width, hint_width), max_width);
				}

				int hint_height=GUIConfiguration::instance().hint_render_area_height;
				if(hint_height<=0)
				{
					hint_height=(current_heigth_>0.0f ? static_cast<int>(current_heigth_) : recommended_height);
				}
				else
				{
					hint_height=(actual_max_height-hint_height);
					hint_height=std::min(std::max(static_cast<int>(actual_min_height), hint_height), static_cast<int>(actual_max_height));
				}

				GUIConfiguration::instance().hint_render_area_width=-1;
				GUIConfiguration::instance().hint_render_area_height=-1;

				ImGui::SetNextWindowSize(ImVec2(hint_width, hint_height), ImGuiCond_Always);
			}
			else
			{
				ImGui::SetNextWindowSize(ImVec2(recommended_width, recommended_height), ImGuiCond_FirstUseEver);
				if(shrink_to_minimal_view_ && current_max_width_>0.0f)
				{
					ImGui::SetNextWindowSize(ImVec2(max_width, actual_min_height), ImGuiCond_Always);
					shrink_to_minimal_view_=false;
				}
				else if(current_width_>0.0f && current_max_width_>0.0f && current_heigth_>0.0f)
				{
					ImGui::SetNextWindowSize(ImVec2(current_width_+(max_width-current_max_width_), ((current_heigth_!=current_max_heigth_) ? current_heigth_ : actual_max_height)), ImGuiCond_Always);
				}
			}

			ImGui::SetNextWindowBgAlpha(0.5f);
			if(!ImGui::Begin("Console", 0, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoScrollbar))
			{
				ImGui::End();
				return result;
			}

			command_line_interface_state_.execute(result, sequence_viewer_state_.calc_total_container_height());

			sequence_viewer_state_.execute(result);

			current_width_=ImGui::GetWindowWidth();
			current_heigth_=ImGui::GetWindowHeight();
			current_max_width_=max_width;
			current_max_heigth_=actual_max_height;

			ImGui::End();
		}

		{
			ImGui::SetNextWindowPos(ImVec2(current_width_, y_pos+current_menu_bar_height_));

			ImVec2 panel_size(current_max_width_-current_width_, current_max_heigth_);
			ImGui::SetNextWindowSizeConstraints(panel_size, panel_size);

			ImGui::SetNextWindowSize(panel_size, ImGuiCond_Always);
			ImGui::SetNextWindowBgAlpha(0.5f);
			if(!ImGui::Begin("Panel", 0, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize))
			{
				ImGui::End();
				return result;
			}

			if(script_editor_state_.visible && ImGui::CollapsingHeader("Script editor##header_for_script_editor"))
			{
				script_editor_state_.execute(result);
			}
			else
			{
				script_editor_state_.focused=false;
			}

			if(documentation_viewer_state_.visible && ImGui::CollapsingHeader("Commands reference##header_for_list_of_documentation"))
			{
				documentation_viewer_state_.execute();
			}

			if(shading_control_toolbar_state_.visible && ImGui::CollapsingHeader("Shading controls##header_for_shading_controls"))
			{
				shading_control_toolbar_state_.execute(result);
			}

			if(display_control_toolbar_state_.visible && ImGui::CollapsingHeader("Display controls##header_for_display_controls", ImGuiTreeNodeFlags_DefaultOpen))
			{
				display_control_toolbar_state_.execute(result);
			}

			if(object_list_viewer_state_.visible && ImGui::CollapsingHeader("Objects##header_for_list_of_objects", ImGuiTreeNodeFlags_DefaultOpen))
			{
				object_list_viewer_state_.execute(result);
			}

			ImGui::End();
		}

		return result;
	}

	int current_width() const
	{
		if(!GUIConfiguration::instance().enabled_console)
		{
			return 0;
		}
		return static_cast<int>(current_width_);
	}

	int current_heigth() const
	{
		if(!GUIConfiguration::instance().enabled_console)
		{
			return 0;
		}
		return (current_heigth_+current_menu_bar_height_);
	}

private:
	class CommandLineInterfaceState
	{
	public:
		std::deque<OutputToken> outputs;
		std::vector<std::string> history_of_commands;
		std::string next_prefix;
		bool need_keyboard_focus_in_command_input;
		bool scroll_output;
		float height_for_command_line;

		CommandLineInterfaceState() :
			need_keyboard_focus_in_command_input(false),
			scroll_output(false),
			height_for_command_line(20.0f),
			command_buffer_(1024, 0),
			index_of_history_of_commands_(0)
		{
		}

		void execute(std::string& result, const float leave_more_space)
		{
			const float height_for_command_line_and_end=(height_for_command_line*GUIStyleWrapper::scale_factor()+leave_more_space);
			const float height_for_output_block=ImGui::GetWindowHeight()-height_for_command_line_and_end;

			if(height_for_output_block>20.0f*GUIStyleWrapper::scale_factor())
			{
				ImGui::BeginChild("##console_scrolling_region", ImVec2(0, 0-height_for_command_line_and_end), true);
				ImGui::PushItemWidth(-1);
				ImGui::PushTextWrapPos();
				for(std::size_t i=0;i<outputs.size();i++)
				{
					const OutputToken& ot=outputs[i];
					if(ot.content==separator_string())
					{
						ImGui::Separator();
					}
					else
					{
						ImVec4 color_text=ImVec4(ot.r, ot.g, ot.b, 1.0f);
						ImGui::PushStyleColor(ImGuiCol_Text, color_text);
						{
							ImGuiAddonGlobalTextColorVector gtcv(ot.char_colors.size(), ot.char_colors.data());
							ImGui::TextUnformatted(ot.content.c_str());
						}
						ImGui::PopStyleColor();
						execute_copy_menu(i, ot.content);
					}
				}
				ImGui::PopTextWrapPos();
				ImGui::PopItemWidth();
				if(scroll_output)
				{
					ImGui::SetScrollHereY();
				}
				scroll_output=false;
				ImGui::EndChild();
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				ImVec4 color_background=ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
				ImGui::PushItemWidth(-1);
				if(ImGui::InputText("##console_command_input", command_buffer_.data(), command_buffer_.size(), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackHistory|ImGuiInputTextFlags_CallbackAlways, &on_command_input_data_request, this))
				{
					result=(std::string(command_buffer_.data()));
					update_history_of_commands(result);
					command_buffer_.assign(command_buffer_.size(), 0);
					need_keyboard_focus_in_command_input=true;
				}
				if(need_keyboard_focus_in_command_input || ((ImGui::IsWindowFocused() || !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) && !ImGui::IsAnyItemActive()))
				{
					ImGui::SetKeyboardFocusHere(-1);
					need_keyboard_focus_in_command_input=false;
				}
				ImGui::PopItemWidth();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
		}

	private:
		static void execute_copy_menu(const std::size_t index, const std::string& content)
		{
			std::ostringstream menu_id_output;
			menu_id_output << "Copy text##copy_output_" << index;
			const std::string menu_id=menu_id_output.str();
			if(ImGui::BeginPopupContextItem(menu_id.c_str(), 1))
			{
				if(ImGui::Selectable("Copy to clipboard"))
				{
					ImGui::SetClipboardText(content.c_str());
				}
				ImGui::EndPopup();
			}
		}

		static int on_command_input_data_request(ImGuiInputTextCallbackData* data)
		{
			CommandLineInterfaceState* obj=static_cast<CommandLineInterfaceState*>(data->UserData);
			return obj->handle_command_input_data_request(data);
		}

		int handle_command_input_data_request(ImGuiInputTextCallbackData* data)
		{
			if(
					!history_of_commands.empty() &&
					data->EventFlag==ImGuiInputTextFlags_CallbackHistory &&
					(data->EventKey==ImGuiKey_UpArrow || data->EventKey==ImGuiKey_DownArrow) &&
					data->BufSize>0
				)
			{
				if(dynamic_history_of_commands_.empty())
				{
					index_of_history_of_commands_=0;
					dynamic_history_of_commands_.push_back(std::string(data->Buf));
					dynamic_history_of_commands_.insert(dynamic_history_of_commands_.end(), history_of_commands.rbegin(), history_of_commands.rend());
				}
				else
				{
					if(index_of_history_of_commands_<dynamic_history_of_commands_.size())
					{
						dynamic_history_of_commands_[index_of_history_of_commands_]=std::string(data->Buf);
					}
					else
					{
						index_of_history_of_commands_=0;
					}
				}

				if(data->EventKey==ImGuiKey_UpArrow)
				{
					if(index_of_history_of_commands_+1<dynamic_history_of_commands_.size())
					{
						index_of_history_of_commands_++;
					}
				}
				else if(data->EventKey==ImGuiKey_DownArrow)
				{
					if(index_of_history_of_commands_>0)
					{
						index_of_history_of_commands_--;
					}
				}

				{
					std::string val=dynamic_history_of_commands_[index_of_history_of_commands_];
					if(val.size()>static_cast<std::size_t>(data->BufSize-1))
					{
						val=val.substr(0, static_cast<std::size_t>(data->BufSize-1));
					}
					for(size_t i=0;i<=val.size();i++)
					{
						data->Buf[i]=val.c_str()[i];
					}
					data->BufDirty=true;
					data->BufTextLen=static_cast<int>(val.size());
					data->CursorPos=data->BufTextLen;
					data->SelectionStart=data->BufTextLen;
					data->SelectionEnd=data->BufTextLen;
				}
			}
			else if(
					data->EventFlag==ImGuiInputTextFlags_CallbackAlways &&
					data->BufTextLen==0 &&
					!next_prefix.empty() &&
					(next_prefix.size()+2)<static_cast<std::size_t>(data->BufSize)
				)
			{
				for(size_t i=0;i<=next_prefix.size();i++)
				{
					data->Buf[i]=next_prefix.c_str()[i];
				}
				data->BufDirty=true;
				data->BufTextLen=static_cast<int>(next_prefix.size());
				data->CursorPos=data->BufTextLen;
				data->SelectionStart=data->BufTextLen;
				data->SelectionEnd=data->BufTextLen;
				next_prefix.clear();
			}
			return 0;
		}

		void update_history_of_commands(const std::string& command)
		{
			if(!command.empty() && (history_of_commands.empty() || command!=history_of_commands.back()))
			{
				history_of_commands.push_back(command);
			}
			dynamic_history_of_commands_.clear();
		}

		std::vector<std::string> dynamic_history_of_commands_;
		std::vector<char> command_buffer_;
		std::size_t index_of_history_of_commands_;
	};

	class ScriptEditorState
	{
	public:
		bool visible;
		bool focused;

		ScriptEditorState() :
			visible(true),
			focused(false),
			script_editor_colors_black_on_white_(false)
		{
			if(script_editor_colors_black_on_white_)
			{
				editor_.SetPalette(TextEditor::GetLightPalette());
			}
			else
			{
				editor_.SetPalette(TextEditor::GetDarkPalette());
			}
		}

		void execute(std::string& result)
		{
			if(ImGui::Button("Run##script_editor", ImVec2(70*GUIStyleWrapper::scale_factor(),0)))
			{
				result=editor_.GetText();
			}

			ImGui::SameLine();

			if(ImGui::Button("Clear##script_editor", ImVec2(70*GUIStyleWrapper::scale_factor(),0)))
			{
				editor_.SetText(std::string());
			}

			ImGui::SameLine();

			{
				ImGui::Button("Load example##script_editor", ImVec2(100*GUIStyleWrapper::scale_factor(),0));
				if(ImGui::BeginPopupContextItem("load example script context menu", 0))
				{
					for(std::size_t i=0;i<example_scripts().size();i++)
					{
						const std::string& script_name=example_scripts()[i].first;
						const std::string& script_body=example_scripts()[i].second;
						if(ImGui::Selectable(script_name.c_str()))
						{
							editor_.SetText(script_body);
						}
					}
					ImGui::EndPopup();
				}
			}

			ImGui::SameLine();
			ImGui::TextUnformatted(" ");
			ImGui::SameLine();

			{
				ImGui::Button("Options##script_editor", ImVec2(70*GUIStyleWrapper::scale_factor(),0));
				if(ImGui::BeginPopupContextItem("script editor options context menu", 0))
				{
					if(ImGui::Checkbox("Black on white", &script_editor_colors_black_on_white_))
					{
						if(script_editor_colors_black_on_white_)
						{
							editor_.SetPalette(TextEditor::GetLightPalette());
						}
						else
						{
							editor_.SetPalette(TextEditor::GetDarkPalette());
						}
					}

					ImGui::EndPopup();
				}
			}

			{
				const float flexible_region_size_min=100;
				const float flexible_region_size_max=1000*GUIStyleWrapper::scale_factor();

				static float flexible_region_size=300;
				static float flexible_region_size_left=flexible_region_size_max-flexible_region_size;

				flexible_region_size=std::max(flexible_region_size_min, std::min(flexible_region_size, flexible_region_size_max));

				ImGuiAddonSimpleSplitter::set_splitter("##script_editor_splitter", &flexible_region_size, &flexible_region_size_left, flexible_region_size_min, flexible_region_size_min, GUIStyleWrapper::scale_factor());

				ImGui::BeginChild("##script_editor_scrolling_region", ImVec2(0, (flexible_region_size-4.0f*GUIStyleWrapper::scale_factor())));
				editor_.Render("ScriptEditor");
				focused=editor_.IsFocused();
				ImGui::EndChild();

				ImGui::Dummy(ImVec2(0.0f, 4.0f*GUIStyleWrapper::scale_factor()));
			}
		}

	private:
		static const std::vector< std::pair<std::string, std::string> >& example_scripts()
		{
			static std::vector< std::pair<std::string, std::string> > collection;
			if(collection.empty())
			{
				std::istringstream input(duktaper::resources::data_script_examples());
				while(input.good())
				{
					std::string line;
					std::getline(input, line);
					if(line.size()>4 && line.rfind("### ", 0)==0)
					{
						collection.push_back(std::make_pair(line.substr(4), line));
						collection.back().second+="\n";
					}
					else if(!collection.empty())
					{
						collection.back().second+=line;
						collection.back().second+="\n";
					}
				}
			}
			return collection;
		}

		static void write_string_to_vector(const std::string& str, std::vector<char>& v)
		{
			for(std::size_t i=0;i<str.size() && (i+1)<v.size();i++)
			{
				v[i]=str[i];
				v[i+1]=0;
			}
		}

		bool script_editor_colors_black_on_white_;
		TextEditor editor_;
	};

	class DocumentationViewerState
	{
	public:
		bool visible;
		std::map<std::string, std::string> documentation;

		DocumentationViewerState() :
			visible(true)
		{
		}

		void execute()
		{
			static std::vector<char> search_string_buffer;
			if(search_string_buffer.empty())
			{
				search_string_buffer.resize(256, 0);
			}

			ImGui::PushItemWidth(200);
			ImGui::InputText("##doc_viewer_search_string", search_string_buffer.data(), 128);
			ImGui::PopItemWidth();

			ImGui::SameLine();

			if(ImGui::Button("Clear##doc_viewer", ImVec2(70*GUIStyleWrapper::scale_factor(),0)))
			{
				search_string_buffer.clear();
				search_string_buffer.resize(256, 0);
			}

			const std::string search_string(search_string_buffer.data());

			static bool doc_viewer_colors_black_on_white=false;

			ImGui::SameLine();
			ImGui::TextUnformatted(" ");
			ImGui::SameLine();

			{
				ImGui::Button("Options##doc_viewer", ImVec2(70*GUIStyleWrapper::scale_factor(),0));
				if(ImGui::BeginPopupContextItem("doc viewer options context menu", 0))
				{
					ImGui::Checkbox("Black on white", &doc_viewer_colors_black_on_white);

					ImGui::EndPopup();
				}
			}

			ImVec4 color_text=(doc_viewer_colors_black_on_white ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImVec4 color_background=(doc_viewer_colors_black_on_white ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, color_text);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, color_background);

			{
				const float flexible_region_size_min=100;
				const float flexible_region_size_max=800*GUIStyleWrapper::scale_factor();

				static float flexible_region_size=300;
				static float flexible_region_size_left=flexible_region_size_max-flexible_region_size;

				flexible_region_size=std::max(flexible_region_size_min, std::min(flexible_region_size, flexible_region_size_max));

				ImGuiAddonSimpleSplitter::set_splitter("##documentation_viewer_splitter", &flexible_region_size, &flexible_region_size_left, flexible_region_size_min, flexible_region_size_min, GUIStyleWrapper::scale_factor());

				ImGui::BeginChild("##area_for_list_of_documentation", ImVec2(0, (flexible_region_size-4.0f*GUIStyleWrapper::scale_factor())));

				for(std::map<std::string, std::string>::const_iterator it=documentation.begin();it!=documentation.end();++it)
				{
					const std::string& title=it->first;
					const std::string& content=it->second;
					if(search_string.empty() || title.find(search_string)!=std::string::npos)
					{
						if(ImGui::TreeNode(title.c_str()))
						{
							if(content.empty())
							{
								ImGui::TextUnformatted("    no arguments");
							}
							else
							{
								ImGui::TextUnformatted(content.c_str());
							}
							ImGui::TreePop();
						}
					}
				}

				ImGui::EndChild();

				ImGui::Dummy(ImVec2(0.0f, 4.0f*GUIStyleWrapper::scale_factor()));
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
	};

	class DisplayControlToolbarState
	{
	public:
		bool visible;

		DisplayControlToolbarState() :
			visible(true)
		{
		}

		void execute(std::string& result)
		{
			{
				static bool sequence=false;

				sequence=GUIConfiguration::instance().enabled_sequence_view;

				if(ImGui::Checkbox("Sequence", &sequence))
				{
					if(sequence)
					{
						result="vsb: configure-gui-enable-sequence-view";
					}
					else
					{
						result="vsb: configure-gui-disable-sequence-view";
					}
				}
			}
			ImGui::SameLine();
			{
				static bool grid=false;

				grid=uv::ViewerApplication::instance().rendering_mode_is_grid();

				if(ImGui::Checkbox("Grid", &grid))
				{
					if(grid)
					{
						result="vsb: grid-by-object";
					}
					else
					{
						result="vsb: mono";
					}
				}
			}
			ImGui::SameLine();
			{
				static bool stereo=false;

				stereo=uv::ViewerApplication::instance().rendering_mode_is_stereo();

				if(ImGui::Checkbox("Stereo", &stereo))
				{
					if(stereo)
					{
						result="vsb: stereo";
					}
					else
					{
						result="vsb: mono";
					}
				}
			}

			{
				static bool perspective=false;

				perspective=uv::ViewerApplication::instance().projection_mode_is_perspective();

				if(ImGui::Checkbox("Perspective", &perspective))
				{
					if(perspective)
					{
						result="vsb: perspective";
					}
					else
					{
						result="vsb: ortho";
					}
				}
			}
			ImGui::SameLine();
			{
				static bool compact_printing=false;

				compact_printing=(GUIConfiguration::instance().json_writing_level<6);

				if(ImGui::Checkbox("Compact printing", &compact_printing))
				{
					if(compact_printing)
					{
						result="vsb: configure-gui-json-write-level-0";
					}
					else
					{
						result="vsb: configure-gui-json-write-level-6";
					}
				}
			}

			{
				const std::string button_id=std::string("Set background##background_color_button");
				const std::string menu_id=std::string("Background##background_color_button_menu");
				ImGui::Button(button_id.c_str(), ImVec2(130*GUIStyleWrapper::scale_factor(),0));
				if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
				{
					if(ImGui::Selectable("white"))
					{
						result="background white";
					}
					if(ImGui::Selectable("black"))
					{
						result="background black";
					}
					if(ImGui::Selectable("gray"))
					{
						result="background 0xCCCCCC";
					}
					ImGui::EndPopup();
				}
			}
		}
	};

	class ShadingControlToolbarState
	{
	public:
		bool visible;

		ShadingControlToolbarState() :
			visible(true)
		{
		}

		void execute(std::string& result)
		{
			{
				static bool antialiasing=false;

				antialiasing=uv::ViewerApplication::instance().antialiasing_mode_is_fast();

				if(ImGui::Checkbox("Antialiasing", &antialiasing))
				{
					if(antialiasing)
					{
						result="vsb: antialiasing-fast ; multisampling-none";
					}
					else
					{
						result="vsb: antialiasing-none";
					}
				}
			}
			ImGui::SameLine();
			{
				static bool multisampling=false;

				multisampling=uv::ViewerApplication::instance().multisampling_mode_is_basic();

				if(ImGui::Checkbox("Multisampling", &multisampling))
				{
					if(multisampling)
					{
						result="vsb: multisampling-basic ; antialiasing-none";
					}
					else
					{
						result="vsb: multisampling-none";
					}
				}
			}

			{
				static bool occlusion_smooth=false;

				occlusion_smooth=uv::ViewerApplication::instance().occlusion_mode_is_smooth();

				if(ImGui::Checkbox("Occlusion (smooth)", &occlusion_smooth))
				{
					if(occlusion_smooth)
					{
						result="vsb: occlusion-smooth";
					}
					else
					{
						result="vsb: occlusion-none";
					}
				}
			}
			ImGui::SameLine();
			{
				static bool occlusion_noisy=false;

				occlusion_noisy=uv::ViewerApplication::instance().occlusion_mode_is_noisy();

				if(ImGui::Checkbox("Occlusion (noisy)", &occlusion_noisy))
				{
					if(occlusion_noisy)
					{
						result="vsb: occlusion-noisy";
					}
					else
					{
						result="vsb: occlusion-none";
					}
				}
			}

			{
				static bool impostoring=false;

				impostoring=(GUIConfiguration::instance().impostoring_variant==GUIConfiguration::IMPOSTORING_VARIANT_SIMPLE);

				if(ImGui::Checkbox("Impostors", &impostoring))
				{
					if(impostoring)
					{
						result="vsb: impostoring-simple";
					}
					else
					{
						result="vsb: impostoring-none";
					}
				}
			}
			ImGui::SameLine();
			{
				static bool show_fps=false;

				ImGui::Checkbox(show_fps ? "Show stats:" : "Show stats", &show_fps);

				if(show_fps)
				{
					ImGui::SameLine();
					ImGui::Text("FPS = %.1f", static_cast<double>(ImGui::GetIO().Framerate));
				}
			}
		}
	};

	class ObjectListViewerState
	{
	public:
		const ObjectsInfo& objects_info;
		bool visible;

		ObjectListViewerState(const ObjectsInfo& objects_info) :
			objects_info(objects_info),
			visible(true)
		{
		}

		void execute(std::string& result) const
		{
			const std::vector<ObjectsInfo::ObjectState>& object_states=objects_info.get_object_states();

			if(object_states.empty())
			{
				ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				ImGui::TextUnformatted("No objects loaded yet.");
				ImGui::PopStyleColor();
				return;
			}

			{
				{
					{
						static std::vector<char> atoms_selection_buffer;

						ImGui::TextUnformatted("Atoms:");
						ImGui::SameLine();
						
						{
							std::string button_id=atoms_selection_string()+"##button_atoms_selection_change";
							ImGui::Button(button_id.c_str());
						}

						const std::string submenu_id=std::string("Change##submenu_atoms_selection");
						if(ImGui::BeginPopupContextItem(submenu_id.c_str(), 0))
						{
							if(atoms_selection_buffer.empty())
							{
								atoms_selection_buffer=std::vector<char>(atoms_selection_string().begin(), atoms_selection_string().end());
								atoms_selection_buffer.resize(atoms_selection_string().size()+128, 0);
							}
							const std::string textbox_id=std::string("##atoms_selection");
							ImGui::PushItemWidth(400.0f*GUIStyleWrapper::scale_factor());
							if(ImGui::InputText(textbox_id.c_str(), atoms_selection_buffer.data(), 128, ImGuiInputTextFlags_EnterReturnsTrue))
							{
								const std::string newvalue(atoms_selection_buffer.data());
								if(!newvalue.empty())
								{
									set_atoms_selection_string_and_save_suggestion(newvalue);
									atoms_selection_buffer.clear();
									ImGui::CloseCurrentPopup();
								}
							}
							ImGui::PopItemWidth();
							{
								const std::string button_id=std::string("OK##button_atoms_selection_ok");
								if(ImGui::Button(button_id.c_str()))
								{
									set_atoms_selection_string_and_save_suggestion(std::string(atoms_selection_buffer.data()));
									atoms_selection_buffer.clear();
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
								const std::string button_id=std::string("Reset##button_atoms_selection_reset");
								if(ImGui::Button(button_id.c_str()))
								{
									set_atoms_selection_string_and_save_suggestion(default_atoms_selection_string());
									atoms_selection_buffer.clear();
									ImGui::CloseCurrentPopup();
								}
							}
							if(!atoms_selection_string_previous().empty())
							{
								ImGui::SameLine();
								std::string button_id="Restore ";
								button_id+=atoms_selection_string_previous();
								button_id+="##button_atoms_selection_change_previous";
								if(ImGui::Button(button_id.c_str()))
								{
									set_atoms_selection_string_and_save_suggestion(atoms_selection_string_previous());
									atoms_selection_buffer.clear();
									ImGui::CloseCurrentPopup();
								}
							}

							if(!atoms_selection_string_suggestions().first.empty())
							{
								ImGui::Separator();

								for(std::size_t i=0;i<atoms_selection_string_suggestions().first.size();i++)
								{
									if(ImGui::Selectable(atoms_selection_string_suggestions().first[i].c_str()))
									{
										set_atoms_selection_string_and_save_suggestion(atoms_selection_string_suggestions().first[i]);
										atoms_selection_buffer.clear();
									}
								}
							}

							if(!atoms_selection_string_suggestions().second.empty())
							{
								ImGui::Separator();

								for(std::size_t i=0;i<atoms_selection_string_suggestions().second.size();i++)
								{
									if(ImGui::Selectable(atoms_selection_string_suggestions().second[i].c_str()))
									{
										set_atoms_selection_string_and_save_suggestion(atoms_selection_string_suggestions().second[i]);
										atoms_selection_buffer.clear();
									}
								}
							}

							ImGui::EndPopup();
						}
					}

					{
						static std::vector<char> contacts_selection_buffer;

						ImGui::TextUnformatted("Contacts:");
						ImGui::SameLine();
						
						{
							std::string button_id=contacts_selection_string()+"##button_contacts_selection_change";
							ImGui::Button(button_id.c_str());
						}

						const std::string submenu_id=std::string("Change##submenu_contacts_selection");
						if(ImGui::BeginPopupContextItem(submenu_id.c_str(), 0))
						{
							if(contacts_selection_buffer.empty())
							{
								contacts_selection_buffer=std::vector<char>(contacts_selection_string().begin(), contacts_selection_string().end());
								contacts_selection_buffer.resize(contacts_selection_string().size()+128, 0);
							}
							const std::string textbox_id=std::string("##contacts_selection");
							ImGui::PushItemWidth(400.0f*GUIStyleWrapper::scale_factor());
							if(ImGui::InputText(textbox_id.c_str(), contacts_selection_buffer.data(), 128, ImGuiInputTextFlags_EnterReturnsTrue))
							{
								const std::string newvalue(contacts_selection_buffer.data());
								if(!newvalue.empty())
								{
									set_contacts_selection_string_and_save_suggestion(newvalue);
									contacts_selection_buffer.clear();
									ImGui::CloseCurrentPopup();
								}
							}
							ImGui::PopItemWidth();
							{
								const std::string button_id=std::string("OK##button_contacts_selection_ok");
								if(ImGui::Button(button_id.c_str()))
								{
									set_contacts_selection_string_and_save_suggestion(std::string(contacts_selection_buffer.data()));
									contacts_selection_buffer.clear();
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
								const std::string button_id=std::string("Reset##button_contacts_selection_reset");
								if(ImGui::Button(button_id.c_str()))
								{
									set_contacts_selection_string_and_save_suggestion(default_contacts_selection_string());
									contacts_selection_buffer.clear();
									ImGui::CloseCurrentPopup();
								}
							}
							if(!contacts_selection_string_previous().empty())
							{
								ImGui::SameLine();
								std::string button_id="Restore ";
								button_id+=contacts_selection_string_previous();
								button_id+="##button_contacts_selection_change_previous";
								if(ImGui::Button(button_id.c_str()))
								{
									set_contacts_selection_string_and_save_suggestion(contacts_selection_string_previous());
									contacts_selection_buffer.clear();
									ImGui::CloseCurrentPopup();
								}
							}

							if(!contacts_selection_string_suggestions().first.empty())
							{
								ImGui::Separator();

								for(std::size_t i=0;i<contacts_selection_string_suggestions().first.size();i++)
								{
									if(ImGui::Selectable(contacts_selection_string_suggestions().first[i].c_str()))
									{
										set_contacts_selection_string_and_save_suggestion(contacts_selection_string_suggestions().first[i]);
										contacts_selection_buffer.clear();
									}
								}
							}

							if(!contacts_selection_string_suggestions().second.empty())
							{
								ImGui::Separator();

								for(std::size_t i=0;i<contacts_selection_string_suggestions().second.size();i++)
								{
									if(ImGui::Selectable(contacts_selection_string_suggestions().second[i].c_str()))
									{
										set_contacts_selection_string_and_save_suggestion(contacts_selection_string_suggestions().second[i]);
										contacts_selection_buffer.clear();
									}
								}
							}

							ImGui::EndPopup();
						}
					}
				}

				ImGui::Separator();

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

						{
							bool separated=false;

							if(GUIConfiguration::instance().animation_variant!=GUIConfiguration::ANIMATION_VARIANT_NONE)
							{
								if(!separated)
								{
									ImGui::Separator();
									separated=true;
								}

								if(ImGui::Selectable("Stop animation"))
								{
									result="animate-none\n";
								}
							}

							if(objects_info.num_of_picked_objects()>1)
							{
								if(GUIConfiguration::instance().animation_variant==GUIConfiguration::ANIMATION_VARIANT_NONE || (GUIConfiguration::instance().animation_variant==GUIConfiguration::ANIMATION_VARIANT_LOOP_PICKED_OBJECTS && GUIConfiguration::instance().animation_step_miliseconds>12.0))
								{
									if(!separated)
									{
										ImGui::Separator();
										separated=true;
									}

									if(ImGui::Selectable("Loop picked objects, faster"))
									{
										result="animate-loop-picked-objects -time-step 5.0\n";
									}
								}

								if(GUIConfiguration::instance().animation_variant==GUIConfiguration::ANIMATION_VARIANT_NONE || (GUIConfiguration::instance().animation_variant==GUIConfiguration::ANIMATION_VARIANT_LOOP_PICKED_OBJECTS && GUIConfiguration::instance().animation_step_miliseconds<12.0))
								{
									if(!separated)
									{
										ImGui::Separator();
										separated=true;
									}

									if(ImGui::Selectable("Loop picked objects, slower"))
									{
										result="animate-loop-picked-objects -time-step 25.0\n";
									}
								}
							}
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
				{
					const std::string button_id=std::string("A##button_actions");
					const std::string menu_id=std::string("Actions##menu_actions");
					ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
					if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
					{
						{
							ImGui::TextUnformatted("Atoms actions:");

							ImGui::Separator();

							if(ImGui::Selectable("  Mark atoms"))
							{
								result=std::string("mark-atoms -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  Unmark atoms"))
							{
								result=std::string("unmark-atoms -use (")+atoms_selection_string()+")";
							}

							ImGui::Separator();

							if(ImGui::Selectable("  Unmark all atoms"))
							{
								result=std::string("unmark-atoms");
							}

							ImGui::Separator();

							{
								ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);

								if(ImGui::Selectable("  Restrict atoms to selection"))
								{
									result=std::string("restrict-atoms -use (")+atoms_selection_string()+")";
								}

								if(ImGui::Selectable("  Restrict atoms to not selection"))
								{
									result=std::string("restrict-atoms -use (not (")+atoms_selection_string()+"))";
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
								result=std::string("construct-contacts");
							}
							ImGui::Separator();
							if(ImGui::Selectable("  Mark contacts"))
							{
								result=std::string("mark-contacts -use (")+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  Unmark contacts"))
							{
								result=std::string("unmark-contacts -use (")+contacts_selection_string()+")";
							}
							ImGui::Separator();
							if(ImGui::Selectable("  Unmark all contacts"))
							{
								result=std::string("unmark-contacts");
							}
						}

						ImGui::EndPopup();
					}
				}
				ImGui::SameLine();
				{
					const std::string button_id=std::string("S##button_show");
					const std::string menu_id=std::string("Show##menu_show");
					ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
					if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
					{
						{
							ImGui::TextUnformatted("Show atoms:");

							if(ImGui::BeginMenu("  as##atoms_show_as"))
							{
								if(ImGui::MenuItem("cartoon##show_as"))
								{
									result=std::string("hide-atoms -use (")+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep cartoon -use (")+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("trace##show_as"))
								{
									result=std::string("hide-atoms -use (")+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep trace -use (")+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("sticks##show_as"))
								{
									result=std::string("hide-atoms -use (")+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep sticks -use (")+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("balls##show_as"))
								{
									result=std::string("hide-atoms -use (")+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep balls -use (")+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("points##show_as"))
								{
									result=std::string("hide-atoms -use (")+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep points -use (")+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("molsurf##show_as"))
								{
									result=std::string("hide-atoms -use (")+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep molsurf -use (")+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("molsurf-mesh##show_as"))
								{
									result=std::string("hide-atoms -use (")+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep molsurf-mesh -use (")+atoms_selection_string()+")";
								}

								ImGui::EndMenu();
							}

							if(ImGui::Selectable("  cartoon##show"))
							{
								result=std::string("show-atoms -rep cartoon -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  trace##show"))
							{
								result=std::string("show-atoms -rep trace -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  sticks##show"))
							{
								result=std::string("show-atoms -rep sticks -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  balls##show"))
							{
								result=std::string("show-atoms -rep balls -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  points##show"))
							{
								result=std::string("show-atoms -rep points -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  molsurf##show"))
							{
								result=std::string("show-atoms -rep molsurf -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  molsurf-mesh##show"))
							{
								result=std::string("show-atoms -rep molsurf-mesh -use (")+atoms_selection_string()+")";
							}
						}

						ImGui::Separator();

						{
							ImGui::TextUnformatted("Show contacts:");

							if(ImGui::BeginMenu("  as##contacts_show_as"))
							{
								if(ImGui::MenuItem("faces##show_as"))
								{
									result="construct-contacts\n";
									result+=std::string("hide-contacts -use (")+contacts_selection_string()+")\n";
									result+=std::string("show-contacts -rep faces -use (")+contacts_selection_string()+")";
								}
								if(ImGui::MenuItem("edges##show_as"))
								{
									result="construct-contacts\n";
									result+=std::string("hide-contacts -use (")+contacts_selection_string()+")\n";
									result+=std::string("show-contacts -rep edges -use (")+contacts_selection_string()+")";
								}
								if(ImGui::MenuItem("sas-mesh##show_as"))
								{
									result="construct-contacts\n";
									result+=std::string("hide-contacts -use (")+contacts_selection_string()+")\n";
									result+=std::string("show-contacts -rep sas-mesh -use (")+contacts_selection_string()+")";
								}

								ImGui::EndMenu();
							}

							if(ImGui::Selectable("  faces##show"))
							{
								result="construct-contacts\n";
								result+=std::string("show-contacts -rep faces -use (")+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  edges##show"))
							{
								result="construct-contacts\n";
								result+=std::string("show-contacts -rep edges -use (")+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  sas-mesh##show"))
							{
								result="construct-contacts\n";
								result+=std::string("show-contacts -rep sas-mesh -use (")+contacts_selection_string()+")";
							}
						}

						ImGui::EndPopup();
					}
				}
				ImGui::SameLine();
				{
					const std::string button_id=std::string("H##button_hide");
					const std::string menu_id=std::string("Hide##menu_hide");
					ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
					if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
					{
						{
							ImGui::TextUnformatted("Hide atoms:");
							if(ImGui::Selectable("  all##atoms_hide"))
							{
								result=std::string("hide-atoms -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  cartoon##atoms_hide"))
							{
								result=std::string("hide-atoms -rep cartoon -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  trace##atoms_hide"))
							{
								result=std::string("hide-atoms -rep trace -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  sticks##atoms_hide"))
							{
								result=std::string("hide-atoms -rep sticks -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  balls##atoms_hide"))
							{
								result=std::string("hide-atoms -rep balls -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  points##atoms_hide"))
							{
								result=std::string("hide-atoms -rep points -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  molsurf##atoms_hide"))
							{
								result=std::string("hide-atoms -rep molsurf -use (")+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  molsurf-mesh##atoms_hide"))
							{
								result=std::string("hide-atoms -rep molsurf-mesh -use (")+atoms_selection_string()+")";
							}
						}

						ImGui::Separator();

						{
							ImGui::TextUnformatted("Hide contacts:");
							if(ImGui::Selectable("  all##contacts_hide"))
							{
								result=std::string("hide-contacts -use (")+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  faces##contacts_hide"))
							{
								result=std::string("hide-contacts -rep faces -use (")+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  edges##contacts_hide"))
							{
								result=std::string("hide-contacts -rep edges -use (")+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  sas-mesh##contacts_hide"))
							{
								result=std::string("hide-contacts -rep sas-mesh -use (")+contacts_selection_string()+")";
							}
						}

						ImGui::EndPopup();
					}
				}
				ImGui::SameLine();
				{
					const std::string button_id=std::string("C##button_color");
					const std::string menu_id=std::string("Color##menu_color");
					ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
					if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
					{
						if(ImGui::BeginMenu("Color atoms##submenu_atoms_color"))
						{
							static bool rep_cartoon=true;
							static bool rep_trace=true;
							static bool rep_sticks=true;
							static bool rep_balls=true;
							static bool rep_points=true;
							static bool rep_molsurf=true;
							static bool rep_molsurf_mesh=true;

							{
								const std::string checkbox_id=std::string("cartoon##cartoon_checkbox_rep");
								ImGui::Checkbox(checkbox_id.c_str(), &rep_cartoon);
							}
							{
								const std::string checkbox_id=std::string("trace##trace_checkbox_rep");
								ImGui::Checkbox(checkbox_id.c_str(), &rep_trace);
							}
							{
								const std::string checkbox_id=std::string("sticks##sticks_checkbox_rep");
								ImGui::Checkbox(checkbox_id.c_str(), &rep_sticks);
							}
							{
								const std::string checkbox_id=std::string("balls##balls_checkbox_rep");
								ImGui::Checkbox(checkbox_id.c_str(), &rep_balls);
							}
							{
								const std::string checkbox_id=std::string("points##points_checkbox_rep");
								ImGui::Checkbox(checkbox_id.c_str(), &rep_points);
							}
							{
								const std::string checkbox_id=std::string("molsurf##molsurf_checkbox_rep");
								ImGui::Checkbox(checkbox_id.c_str(), &rep_molsurf);
							}
							{
								const std::string checkbox_id=std::string("molsurf-mesh##molsurf_mesh_checkbox_rep");
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
								result=std::string("spectrum-atoms -use (")+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by residue ID"))
							{
								result=std::string("spectrum-atoms -by residue-id -use (")+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by residue ID randomly"))
							{
								result=std::string("spectrum-atoms -by residue-id -scheme random -use (")+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by chain"))
							{
								result=std::string("spectrum-atoms -by chain -use (")+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by chain randomly"))
							{
								result=std::string("spectrum-atoms -by chain -scheme random -use (")+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by secondary structure"))
							{
								result=std::string("spectrum-atoms -by secondary-structure -use (")+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::BeginMenu("  by atom type"))
							{
								if(ImGui::MenuItem("all"))
								{
									result=std::string("spectrum-atoms -by atom-type -use (")+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("all except carbon"))
								{
									result=std::string("spectrum-atoms -by atom-type -use ((")+atoms_selection_string()+") and ([-t! el=C]))"+rep_string;
								}

								ImGui::EndMenu();
							}

							if(ImGui::BeginMenu("  by B-factor"))
							{
								if(ImGui::MenuItem("blue-white-red"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme bwr -use (")+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("red-white-blue"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme rwb -use (")+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("blue-white-red, 0-100"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme bwr -min-val 0 -max-val 100 -use (")+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("red-white-blue, 0-100"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme rwb -min-val 0 -max-val 100 -use (")+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("blue-white-red, 0-1"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme bwr -min-val 0 -max-val 1 -use (")+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("red-white-blue, 0-1"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme rwb -min-val 0 -max-val 1 -use (")+atoms_selection_string()+")"+rep_string;
								}

								ImGui::EndMenu();
							}

							if(ImGui::Selectable("  by hydropathy"))
							{
								result=std::string("spectrum-atoms -by hydropathy -use (")+atoms_selection_string()+")"+rep_string;
							}

							ImGui::Separator();

							ImGui::TextUnformatted("Color atoms:");

							if(ImGui::Selectable("  random"))
							{
								result=std::string("color-atoms -next-random-color -use (")+atoms_selection_string()+")"+rep_string;
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  red"))
								{
									result=std::string("color-atoms -col 0xFF0000 -use (")+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  yellow"))
								{
									result=std::string("color-atoms -col 0xFFFF00 -use (")+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}
							{
								ImVec4 color_text=ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  green"))
								{
									result=std::string("color-atoms -col 0x00FF00 -use (")+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  cyan"))
								{
									result=std::string("color-atoms -col 0x00FFFF -use (")+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  blue"))
								{
									result=std::string("color-atoms -col 0x0000FF -use (")+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  magenta"))
								{
									result=std::string("color-atoms -col 0xFF00FF -use (")+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  white"))
								{
									result=std::string("color-atoms -col white -use (")+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  light gray"))
								{
									result=std::string("color-atoms -col 0xAAAAAA -use (")+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  dark gray"))
								{
									result=std::string("color-atoms -col 0x555555 -use (")+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							ImGui::EndMenu();
						}

						if(ImGui::BeginMenu("Color contacts##submenu_contacts_color"))
						{
							static bool rep_faces=true;
							static bool rep_edges=true;
							static bool rep_sas_mesh=true;

							{
								const std::string checkbox_id=std::string("faces##faces_checkbox_rep");
								ImGui::Checkbox(checkbox_id.c_str(), &rep_faces);
							}
							{
								const std::string checkbox_id=std::string("edges##edges_checkbox_rep");
								ImGui::Checkbox(checkbox_id.c_str(), &rep_edges);
							}
							{
								const std::string checkbox_id=std::string("sas-mesh##sas_mesh_checkbox_rep");
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
								result=std::string("spectrum-contacts -by residue-ids -scheme random -use (")+contacts_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by inter-residue area, 0-45"))
							{
								result=std::string("spectrum-contacts -by residue-area -min-val 0 -max-val 45 -use (")+contacts_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by inter-atom area, 0-15"))
							{
								result=std::string("spectrum-contacts -by area -min-val 0 -max-val 15 -use (")+contacts_selection_string()+")"+rep_string;
							}

							ImGui::Separator();

							ImGui::TextUnformatted("Color contacts:");

							if(ImGui::Selectable("  random"))
							{
								result=std::string("color-contacts -next-random-color -use (")+contacts_selection_string()+")"+rep_string;
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  red"))
								{
									result=std::string("color-contacts -col 0xFF0000 -use (")+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  yellow"))
								{
									result=std::string("color-contacts -col 0xFFFF00 -use (")+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}
							{
								ImVec4 color_text=ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  green"))
								{
									result=std::string("color-contacts -col 0x00FF00 -use (")+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  cyan"))
								{
									result=std::string("color-contacts -col 0x00FFFF -use (")+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  blue"))
								{
									result=std::string("color-contacts -col 0x0000FF -use (")+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  magenta"))
								{
									result=std::string("color-contacts -col 0xFF00FF -use (")+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  white"))
								{
									result=std::string("color-contacts -col white -use (")+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  light gray"))
								{
									result=std::string("color-contacts -col 0xAAAAAA -use (")+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  dark gray"))
								{
									result=std::string("color-contacts -col 0x555555 -use (")+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							ImGui::EndMenu();
						}

						ImGui::EndPopup();
					}
				}
			}

			ImGui::Separator();

			ImGui::BeginChild("##object_list_scrolling_region", ImVec2(0, 0), false);

			for(std::size_t i=0;i<object_states.size();i++)
			{
				const ObjectsInfo::ObjectState& os=object_states[i];
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
				{
					const std::string button_id=std::string("A##button_actions_")+os.name;
					const std::string menu_id=std::string("Actions##menu_actions_")+os.name;
					ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
					if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
					{
						{
							ImGui::TextUnformatted("Atoms actions:");

							ImGui::Separator();

							if(ImGui::Selectable("  Mark atoms"))
							{
								result=std::string("mark-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  Unmark atoms"))
							{
								result=std::string("unmark-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}

							ImGui::Separator();

							if(ImGui::Selectable("  Unmark all atoms"))
							{
								result=std::string("unmark-atoms -on-objects '")+os.name+"'";
							}

							if(object_states.size()>1)
							{
								ImGui::Separator();

								ImVec4 color_text=ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);

								if(ImGui::Selectable("  Align all"))
								{
									const bool with_music_background=(object_states.size()>10);
									result="";
									if(with_music_background)
									{
										result+="music-background waiting\n";
									}
									result+=std::string("tmalign-many -target '")+os.name+"' -target-sel '("+atoms_selection_string()+")' -model-sel '("+atoms_selection_string()+")'";
									if(with_music_background)
									{
										result+="\nmusic-background stop\n";
									}
								}

								int num_of_other_picked_objects=0;
								for(std::size_t j=0;j<object_states.size();j++)
								{
									if(j!=i && object_states[j].picked)
									{
										num_of_other_picked_objects++;
									}
								}

								if(num_of_other_picked_objects>0 && ImGui::Selectable("  Align picked"))
								{
									const bool with_music_background=(num_of_other_picked_objects>10);
									result="";
									if(with_music_background)
									{
										result+="music-background waiting\n";
									}
									result+=std::string("tmalign-many -picked -target '")+os.name+"' -target-sel '("+atoms_selection_string()+")' -model-sel '("+atoms_selection_string()+")'";
									if(with_music_background)
									{
										result+="\nmusic-background stop\n";
									}
								}

								ImGui::PopStyleColor();
							}

							{
								ImGui::Separator();

								ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);

								if(ImGui::Selectable("  Restrict atoms to selection"))
								{
									result=std::string("restrict-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
								}

								if(ImGui::Selectable("  Restrict atoms to not selection"))
								{
									result=std::string("restrict-atoms -on-objects '")+os.name+"' -use (not ("+atoms_selection_string()+"))";
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
								result=std::string("construct-contacts -on-objects '")+os.name+"'";
							}
							ImGui::Separator();
							if(ImGui::Selectable("  Mark contacts"))
							{
								result=std::string("mark-contacts -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  Unmark contacts"))
							{
								result=std::string("unmark-contacts -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
							}
							ImGui::Separator();
							if(ImGui::Selectable("  Unmark all contacts"))
							{
								result=std::string("unmark-contacts -on-objects '")+os.name+"'";
							}
						}
						ImGui::EndPopup();
					}
				}
				ImGui::SameLine();
				{
					const std::string button_id=std::string("S##button_show_")+os.name;
					const std::string menu_id=std::string("Show##menu_show_")+os.name;
					ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
					if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
					{
						{
							ImGui::TextUnformatted("Show atoms:");

							if(ImGui::BeginMenu("  as##atoms_show_as"))
							{
								if(ImGui::MenuItem("cartoon##show_as"))
								{
									result=std::string("hide-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep cartoon -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("trace##show_as"))
								{
									result=std::string("hide-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep trace -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("sticks##show_as"))
								{
									result=std::string("hide-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep sticks -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("balls##show_as"))
								{
									result=std::string("hide-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep balls  -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("points##show_as"))
								{
									result=std::string("hide-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep points -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("molsurf##show_as"))
								{
									result=std::string("hide-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep molsurf -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
								}
								if(ImGui::MenuItem("molsurf-mesh##show_as"))
								{
									result=std::string("hide-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")\n";
									result+=std::string("show-atoms -rep molsurf-mesh -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
								}

								ImGui::EndMenu();
							}

							if(ImGui::Selectable("  cartoon##show"))
							{
								result=std::string("show-atoms -rep cartoon -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  trace##show"))
							{
								result=std::string("show-atoms -rep trace -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  sticks##show"))
							{
								result=std::string("show-atoms -rep sticks -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  balls##show"))
							{
								result=std::string("show-atoms -rep balls -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  points##show"))
							{
								result=std::string("show-atoms -rep points -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  molsurf##show"))
							{
								result=std::string("show-atoms -rep molsurf -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  molsurf-mesh##show"))
							{
								result=std::string("show-atoms -rep molsurf-mesh -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
						}

						ImGui::Separator();

						{
							ImGui::TextUnformatted("Show contacts:");

							if(ImGui::BeginMenu("  as##contacts_show_as"))
							{
								if(ImGui::MenuItem("faces##show_as"))
								{
									result=std::string("construct-contacts -on-objects '")+os.name+"'\n";
									result+=std::string("hide-contacts -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")\n";
									result+=std::string("show-contacts -rep faces -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
								}
								if(ImGui::MenuItem("edges##show_as"))
								{
									result=std::string("construct-contacts -on-objects '")+os.name+"'\n";
									result+=std::string("hide-contacts -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")\n";
									result+=std::string("show-contacts -rep edges -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
								}
								if(ImGui::MenuItem("sas-mesh##show_as"))
								{
									result=std::string("construct-contacts -on-objects '")+os.name+"'\n";
									result+=std::string("hide-contacts -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")\n";
									result+=std::string("show-contacts -rep sas-mesh -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
								}

								ImGui::EndMenu();
							}

							if(ImGui::Selectable("  faces##show"))
							{
								result=std::string("construct-contacts -on-objects '")+os.name+"'\n";
								result+=std::string("show-contacts -rep faces -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  edges##show"))
							{
								result=std::string("construct-contacts -on-objects '")+os.name+"'\n";
								result+=std::string("show-contacts -rep edges -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  sas-mesh##show"))
							{
								result=std::string("construct-contacts -on-objects '")+os.name+"'\n";
								result+=std::string("show-contacts -rep sas-mesh -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
							}
						}

						ImGui::EndPopup();
					}
				}
				ImGui::SameLine();
				{
					const std::string button_id=std::string("H##button_hide_")+os.name;
					const std::string menu_id=std::string("Hide##menu_hide_")+os.name;
					ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
					if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
					{
						{
							ImGui::TextUnformatted("Hide atoms:");
							if(ImGui::Selectable("  all##atoms_hide"))
							{
								result=std::string("hide-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  cartoon##atoms_hide"))
							{
								result=std::string("hide-atoms -rep cartoon -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  trace##atoms_hide"))
							{
								result=std::string("hide-atoms -rep trace -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  sticks##atoms_hide"))
							{
								result=std::string("hide-atoms -rep sticks -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  balls##atoms_hide"))
							{
								result=std::string("hide-atoms -rep balls -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  points##atoms_hide"))
							{
								result=std::string("hide-atoms -rep points -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  molsurf##atoms_hide"))
							{
								result=std::string("hide-atoms -rep molsurf -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
							if(ImGui::Selectable("  molsurf-mesh##atoms_hide"))
							{
								result=std::string("hide-atoms -rep molsurf-mesh -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")";
							}
						}

						ImGui::Separator();

						{
							ImGui::TextUnformatted("Hide contacts:");
							if(ImGui::Selectable("  all##contacts_hide"))
							{
								result=std::string("hide-contacts -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  faces##contacts_hide"))
							{
								result=std::string("hide-contacts -rep faces -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  edges##contacts_hide"))
							{
								result=std::string("hide-contacts -rep edges -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
							}
							if(ImGui::Selectable("  sas-mesh##contacts_hide"))
							{
								result=std::string("hide-contacts -rep sas-mesh -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")";
							}
						}

						ImGui::EndPopup();
					}
				}
				ImGui::SameLine();
				{
					const std::string button_id=std::string("C##button_color_")+os.name;
					const std::string menu_id=std::string("Color##menu_color_")+os.name;
					ImGui::Button(button_id.c_str(), ImVec2(19*GUIStyleWrapper::scale_factor(),0));
					if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
					{
						const std::string submenu_atoms_color_id=std::string("Color atoms##submenu_atoms_color")+os.name;
						if(ImGui::BeginMenu(submenu_atoms_color_id.c_str()))
						{
							static bool rep_cartoon=true;
							static bool rep_trace=true;
							static bool rep_sticks=true;
							static bool rep_balls=true;
							static bool rep_points=true;
							static bool rep_molsurf=true;
							static bool rep_molsurf_mesh=true;

							{
								const std::string checkbox_id=std::string("cartoon##cartoon_checkbox_rep_")+os.name;
								ImGui::Checkbox(checkbox_id.c_str(), &rep_cartoon);
							}
							{
								const std::string checkbox_id=std::string("trace##trace_checkbox_rep_")+os.name;
								ImGui::Checkbox(checkbox_id.c_str(), &rep_trace);
							}
							{
								const std::string checkbox_id=std::string("sticks##sticks_checkbox_rep_")+os.name;
								ImGui::Checkbox(checkbox_id.c_str(), &rep_sticks);
							}
							{
								const std::string checkbox_id=std::string("balls##balls_checkbox_rep_")+os.name;
								ImGui::Checkbox(checkbox_id.c_str(), &rep_balls);
							}
							{
								const std::string checkbox_id=std::string("points##points_checkbox_rep_")+os.name;
								ImGui::Checkbox(checkbox_id.c_str(), &rep_points);
							}
							{
								const std::string checkbox_id=std::string("molsurf##molsurf_checkbox_rep_")+os.name;
								ImGui::Checkbox(checkbox_id.c_str(), &rep_molsurf);
							}
							{
								const std::string checkbox_id=std::string("molsurf-mesh##molsurf_mesh_checkbox_rep_")+os.name;
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
								result=std::string("spectrum-atoms -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by residue ID"))
							{
								result=std::string("spectrum-atoms -by residue-id -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by residue ID randomly"))
							{
								result=std::string("spectrum-atoms -by residue-id -scheme random -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by chain"))
							{
								result=std::string("spectrum-atoms -by chain -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by chain randomly"))
							{
								result=std::string("spectrum-atoms -by chain -scheme random -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by secondary structure"))
							{
								result=std::string("spectrum-atoms -by secondary-structure -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
							}

							if(ImGui::BeginMenu("  by atom type"))
							{
								if(ImGui::MenuItem("all"))
								{
									result=std::string("spectrum-atoms -by atom-type -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("all except carbon"))
								{
									result=std::string("spectrum-atoms -by atom-type -on-objects '")+os.name+"' -use (("+atoms_selection_string()+") and ([-t! el=C]))"+rep_string;
								}

								ImGui::EndMenu();
							}

							if(ImGui::BeginMenu("  by B-factor"))
							{
								if(ImGui::MenuItem("blue-white-red"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme bwr -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("red-white-blue"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme rwb -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("blue-white-red, 0-100"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme bwr -min-val 0 -max-val 100 -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("red-white-blue, 0-100"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme rwb -min-val 0 -max-val 100 -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("blue-white-red, 0-1"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme bwr -min-val 0 -max-val 1 -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}

								if(ImGui::MenuItem("red-white-blue, 0-1"))
								{
									result=std::string("spectrum-atoms -adjunct tf -scheme rwb -min-val 0 -max-val 1 -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}

								ImGui::EndMenu();
							}

							if(ImGui::Selectable("  by hydropathy"))
							{
								result=std::string("spectrum-atoms -by hydropathy -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
							}

							ImGui::Separator();

							ImGui::TextUnformatted("Color atoms:");

							if(ImGui::Selectable("  random"))
							{
								result=std::string("color-atoms -next-random-color -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  red"))
								{
									result=std::string("color-atoms -col 0xFF0000 -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  yellow"))
								{
									result=std::string("color-atoms -col 0xFFFF00 -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}
							{
								ImVec4 color_text=ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  green"))
								{
									result=std::string("color-atoms -col 0x00FF00 -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  cyan"))
								{
									result=std::string("color-atoms -col 0x00FFFF -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  blue"))
								{
									result=std::string("color-atoms -col 0x0000FF -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  magenta"))
								{
									result=std::string("color-atoms -col 0xFF00FF -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  white"))
								{
									result=std::string("color-atoms -col white -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  light gray"))
								{
									result=std::string("color-atoms -col 0xAAAAAA -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  dark gray"))
								{
									result=std::string("color-atoms -col 0x555555 -on-objects '")+os.name+"' -use ("+atoms_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							ImGui::EndMenu();
						}

						const std::string submenu_contacts_color_id=std::string("Color contacts##submenu_contacts_color")+os.name;
						if(ImGui::BeginMenu(submenu_contacts_color_id.c_str()))
						{
							static bool rep_faces=true;
							static bool rep_edges=true;
							static bool rep_sas_mesh=true;

							{
								const std::string checkbox_id=std::string("faces##faces_checkbox_rep_")+os.name;
								ImGui::Checkbox(checkbox_id.c_str(), &rep_faces);
							}
							{
								const std::string checkbox_id=std::string("edges##edges_checkbox_rep_")+os.name;
								ImGui::Checkbox(checkbox_id.c_str(), &rep_edges);
							}
							{
								const std::string checkbox_id=std::string("sas-mesh##sas_mesh_checkbox_rep_")+os.name;
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
								result=std::string("spectrum-contacts -by residue-ids -scheme random -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by inter-residue area, 0-45"))
							{
								result=std::string("spectrum-contacts -by residue-area -min-val 0 -max-val 45 -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
							}

							if(ImGui::Selectable("  by inter-atom area, 0-15"))
							{
								result=std::string("spectrum-contacts -by area -min-val 0 -max-val 15 -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
							}

							ImGui::Separator();

							ImGui::TextUnformatted("Color contacts:");

							if(ImGui::Selectable("  random"))
							{
								result=std::string("color-contacts -next-random-color -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  red"))
								{
									result=std::string("color-contacts -col 0xFF0000 -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  yellow"))
								{
									result=std::string("color-contacts -col 0xFFFF00 -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}
							{
								ImVec4 color_text=ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  green"))
								{
									result=std::string("color-contacts -col 0x00FF00 -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  cyan"))
								{
									result=std::string("color-contacts -col 0x00FFFF -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  blue"))
								{
									result=std::string("color-contacts -col 0x0000FF -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  magenta"))
								{
									result=std::string("color-contacts -col 0xFF00FF -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  white"))
								{
									result=std::string("color-contacts -col white -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  light gray"))
								{
									result=std::string("color-contacts -col 0xAAAAAA -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							{
								ImVec4 color_text=ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
								ImGui::PushStyleColor(ImGuiCol_Text, color_text);
								if(ImGui::Selectable("  dark gray"))
								{
									result=std::string("color-contacts -col 0x555555 -on-objects '")+os.name+"' -use ("+contacts_selection_string()+")"+rep_string;
								}
								ImGui::PopStyleColor();
							}

							ImGui::EndMenu();
						}

						ImGui::EndPopup();
					}
				}
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
		static const std::string& default_atoms_selection_string()
		{
			static std::string value="[]";
			return value;
		}

		static const std::string& marked_atoms_selection_string()
		{
			static std::string value="[_marked]";
			return value;
		}

		static std::string& atoms_selection_string()
		{
			static std::string value;
			if(value.empty() || value=="[" || value=="]")
			{
				value=default_atoms_selection_string();
			}
			return value;
		}

		static std::string& atoms_selection_string_previous()
		{
			static std::string value;
			return value;
		}

		static std::pair< std::deque<std::string>, std::deque<std::string> >& atoms_selection_string_suggestions()
		{
			static std::pair< std::deque<std::string>, std::deque<std::string> > suggestions;
			if(suggestions.first.empty())
			{
				suggestions.first.push_back("[]");
				suggestions.first.push_back("[_marked]");
				suggestions.first.push_back("[-protein]");
				suggestions.first.push_back("[-nucleic]");
				suggestions.first.push_back("[-chain A]");
				suggestions.first.push_back("[-chain A -rnum 1:200]");
				suggestions.first.push_back("(not [-aname C,N,O,CA])");
				suggestions.first.push_back("[-sel-of-contacts _visible]");
			}
			return suggestions;
		}

		static void set_atoms_selection_string_and_save_suggestion(const std::string& value)
		{
			const std::string future_previous_string=atoms_selection_string();
			atoms_selection_string()=value;
			bool already_suggested=false;
			for(std::size_t i=0;i<atoms_selection_string_suggestions().first.size() && !already_suggested;i++)
			{
				already_suggested=already_suggested || (atoms_selection_string_suggestions().first[i]==atoms_selection_string());
			}
			for(std::size_t i=0;i<atoms_selection_string_suggestions().second.size() && !already_suggested;i++)
			{
				already_suggested=already_suggested || (atoms_selection_string_suggestions().second[i]==atoms_selection_string());
			}
			if(!already_suggested)
			{
				atoms_selection_string_suggestions().second.push_back(atoms_selection_string());
				if(atoms_selection_string_suggestions().second.size()>5)
				{
					atoms_selection_string_suggestions().second.pop_front();
				}
			}
			if(future_previous_string!=atoms_selection_string_previous() && future_previous_string!=atoms_selection_string())
			{
				atoms_selection_string_previous()=future_previous_string;
			}
		}

		static const std::string& default_contacts_selection_string()
		{
			static std::string value="[-no-solvent]";
			return value;
		}

		static const std::string& marked_contacts_selection_string()
		{
			static std::string value="[_marked]";
			return value;
		}

		static std::string& contacts_selection_string()
		{
			static std::string value;
			if(value.empty() || value=="[" || value=="]")
			{
				value=default_contacts_selection_string();
			}
			return value;
		}

		static std::string& contacts_selection_string_previous()
		{
			static std::string value;
			return value;
		}

		static std::pair< std::deque<std::string>, std::deque<std::string> >& contacts_selection_string_suggestions()
		{
			static std::pair< std::deque<std::string>, std::deque<std::string> > suggestions;
			if(suggestions.first.empty())
			{
				suggestions.first.push_back("[]");
				suggestions.first.push_back("[-no-solvent]");
				suggestions.first.push_back("[-min-seq-sep 1]");
				suggestions.first.push_back("[-no-solvent -min-seq-sep 1]");
				suggestions.first.push_back("[-inter-chain]");
				suggestions.first.push_back("[-solvent]");
				suggestions.first.push_back("[_marked]");
				suggestions.first.push_back("[_visible]");
				suggestions.first.push_back("[-a1 [_marked] -a2! [_marked]]");
				suggestions.first.push_back("[-a1 [-protein] -a2 [-nucleic]]");
				suggestions.first.push_back("[-a1 [-chain A] -a2 [-chain B]]");
			}
			return suggestions;
		}

		static void set_contacts_selection_string_and_save_suggestion(const std::string& value)
		{
			const std::string future_previous_string=contacts_selection_string();
			contacts_selection_string()=value;
			bool already_suggested=false;
			for(std::size_t i=0;i<contacts_selection_string_suggestions().first.size() && !already_suggested;i++)
			{
				already_suggested=already_suggested || (contacts_selection_string_suggestions().first[i]==contacts_selection_string());
			}
			for(std::size_t i=0;i<contacts_selection_string_suggestions().second.size() && !already_suggested;i++)
			{
				already_suggested=already_suggested || (contacts_selection_string_suggestions().second[i]==contacts_selection_string());
			}
			if(!already_suggested)
			{
				contacts_selection_string_suggestions().second.push_back(contacts_selection_string());
				if(contacts_selection_string_suggestions().second.size()>5)
				{
					contacts_selection_string_suggestions().second.pop_front();
				}
			}
			if(future_previous_string!=contacts_selection_string_previous() && future_previous_string!=contacts_selection_string())
			{
				contacts_selection_string_previous()=future_previous_string;
			}
		}
	};

	class SequenceViewerState
	{
	public:
		const ObjectsInfo& objects_info;
		bool& visible;
		int max_slots;
		int max_visible_slots;
		float sequence_frame_height;
		float button_width_unit;

		SequenceViewerState(const ObjectsInfo& objects_info) :
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

	Console() :
		current_width_(0.0f),
		current_heigth_(0.0f),
		current_max_width_(0.0f),
		current_max_heigth_(0.0f),
		current_menu_bar_height_(0.0f),
		shrink_to_minimal_view_(false),
		file_search_root_dir_("."),
		object_list_viewer_state_(objects_info_),
		sequence_viewer_state_(objects_info_)
	{
	}

	static const std::string& separator_string()
	{
		static std::string str="---";
		return str;
	}

	float current_width_;
	float current_heigth_;
	float current_max_width_;
	float current_max_heigth_;
	float current_menu_bar_height_;
	bool shrink_to_minimal_view_;
	std::string file_search_root_dir_;

	ObjectsInfo objects_info_;
	CommandLineInterfaceState command_line_interface_state_;
	ScriptEditorState script_editor_state_;
	DocumentationViewerState documentation_viewer_state_;
	DisplayControlToolbarState display_control_toolbar_state_;
	ShadingControlToolbarState shading_control_toolbar_state_;
	ObjectListViewerState object_list_viewer_state_;
	SequenceViewerState sequence_viewer_state_;
};

}

}

#endif /* VIEWER_CONSOLE_H_ */
