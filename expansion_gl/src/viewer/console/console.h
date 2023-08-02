#ifndef VIEWER_CONSOLE_CONSOLE_H_
#define VIEWER_CONSOLE_CONSOLE_H_

#include "command_line_interface_panel.h"
#include "script_editor_panel.h"
#include "documentation_viewer_panel.h"
#include "display_control_toolbar_panel.h"
#include "shading_control_toolbar_panel.h"
#include "object_list_viewer_panel.h"
#include "sequence_viewer_panel.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class Console
{
public:
	static Console& instance()
	{
		static Console console;
		return console;
	}

	TextInterfaceInfo& text_interface_info()
	{
		static TextInterfaceInfo info;
		return info;
	}

	ObjectsInfo& objects_info()
	{
		static ObjectsInfo info;
		return info;
	}

	MarkingInfo& marking_info()
	{
		static MarkingInfo info;
		return info;
	}

	DocumentationInfo& documentation_info()
	{
		static DocumentationInfo info;
		return info;
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
			const float actual_min_height=std::max(static_cast<float>(min_height), (command_line_interface_panel_.height_for_command_line+15.0f)*GUIStyleWrapper::scale_factor()+sequence_viewer_panel_.calc_total_container_height());
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

			command_line_interface_panel_.execute(result, sequence_viewer_panel_.calc_total_container_height());

			sequence_viewer_panel_.execute(result);

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

			if(script_editor_panel_.visible && ImGui::CollapsingHeader("Script editor##header_for_script_editor"))
			{
				script_editor_panel_.execute(result);
			}
			else
			{
				text_interface_info().set_script_editor_focused(false);
			}

			if(documentation_viewer_panel_.visible && ImGui::CollapsingHeader("Commands reference##header_for_list_of_documentation"))
			{
				documentation_viewer_panel_.execute();
			}

			if(shading_control_toolbar_panel_.visible && ImGui::CollapsingHeader("Shading controls##header_for_shading_controls"))
			{
				shading_control_toolbar_panel_.execute(result);
			}

			if(display_control_toolbar_panel_.visible && ImGui::CollapsingHeader("Display controls##header_for_display_controls", ImGuiTreeNodeFlags_DefaultOpen))
			{
				display_control_toolbar_panel_.execute(result);
			}

			if(object_list_viewer_panel_.visible && ImGui::CollapsingHeader("Objects##header_for_list_of_objects", ImGuiTreeNodeFlags_DefaultOpen))
			{
				object_list_viewer_panel_.execute(result);
			}

			ImGui::End();
		}

		return result;
	}

private:
	Console() :
		current_width_(0.0f),
		current_heigth_(0.0f),
		current_max_width_(0.0f),
		current_max_heigth_(0.0f),
		current_menu_bar_height_(0.0f),
		shrink_to_minimal_view_(false),
		file_search_root_dir_("."),
		command_line_interface_panel_(text_interface_info()),
		script_editor_panel_(text_interface_info()),
		documentation_viewer_panel_(documentation_info()),
		object_list_viewer_panel_(objects_info(), marking_info()),
		sequence_viewer_panel_(objects_info())
	{
	}

	float current_width_;
	float current_heigth_;
	float current_max_width_;
	float current_max_heigth_;
	float current_menu_bar_height_;
	bool shrink_to_minimal_view_;
	std::string file_search_root_dir_;

	CommandLineInterfacePanel command_line_interface_panel_;
	ScriptEditorPanel script_editor_panel_;
	DocumentationViewerPanel documentation_viewer_panel_;
	DisplayControlToolbarPanel display_control_toolbar_panel_;
	ShadingControlToolbarPanel shading_control_toolbar_panel_;
	ObjectListViewerPanel object_list_viewer_panel_;
	SequenceViewerPanel sequence_viewer_panel_;
};

}

}

}


#endif /* VIEWER_CONSOLE_CONSOLE_H_ */
