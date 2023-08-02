#ifndef VIEWER_CONSOLE_DOCUMENTATION_VIEWER_STATE_H_
#define VIEWER_CONSOLE_DOCUMENTATION_VIEWER_STATE_H_

#include "documentation_info.h"

#include "common_for_gui.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class DocumentationViewerPanel
{
public:
	bool visible;
	const DocumentationInfo& documentation_info;

	DocumentationViewerPanel(const DocumentationInfo& documentation_info) :
		visible(true),
		documentation_info(documentation_info)
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

			for(std::map<std::string, std::string>::const_iterator it=documentation_info.documentation.begin();it!=documentation_info.documentation.end();++it)
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

}

}

}

#endif /* VIEWER_CONSOLE_DOCUMENTATION_VIEWER_STATE_H_ */
