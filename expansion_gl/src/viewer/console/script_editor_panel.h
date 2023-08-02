#ifndef VIEWER_CONSOLE_SCRIPT_EDITOR_STATE_H_
#define VIEWER_CONSOLE_SCRIPT_EDITOR_STATE_H_

#include "text_interface_info.h"

#include "common_for_gui.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class ScriptEditorPanel
{
public:
	bool visible;
	TextInterfaceInfo& text_interface_info;

	ScriptEditorPanel(TextInterfaceInfo& text_interface_info) :
		visible(true),
		text_interface_info(text_interface_info),
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
			text_interface_info.set_script_editor_focused(editor_.IsFocused());
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

}

}

}



#endif /* VIEWER_CONSOLE_SCRIPT_EDITOR_STATE_H_ */
