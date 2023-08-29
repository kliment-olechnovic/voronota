#ifndef VIEWER_GUI_STYLE_WRAPPER_H_
#define VIEWER_GUI_STYLE_WRAPPER_H_

#include "../dependencies/imgui/imgui_impl_glfw.h"

#include "../uv/stocked_default_fonts.h"

namespace voronota
{

namespace viewer
{

class GUIStyleWrapper
{
public:
	static GUIStyleWrapper& instance()
	{
		static GUIStyleWrapper wrapper;
		return wrapper;
	}

	static const ImGuiStyle& default_style()
	{
		static ImGuiStyle style;

		style.WindowRounding                          = 0.0f;
		style.FrameRounding                           = 0.0f;
		style.GrabRounding                            = 0.0f;
		style.ScrollbarRounding                       = 0.0f;

		style.Colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		style.Colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		style.Colors[ImGuiCol_Border]                 = ImVec4(0.63f, 0.63f, 0.63f, 0.50f);
		style.Colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
		style.Colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		style.Colors[ImGuiCol_TitleBg]                = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		style.Colors[ImGuiCol_TitleBgActive]          = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_CheckMark]              = ImVec4(0.74f, 0.74f, 0.74f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab]             = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		style.Colors[ImGuiCol_Button]                 = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		style.Colors[ImGuiCol_ButtonHovered]          = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive]           = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		style.Colors[ImGuiCol_Header]                 = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
		style.Colors[ImGuiCol_HeaderHovered]          = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
		style.Colors[ImGuiCol_HeaderActive]           = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip]             = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		style.Colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		style.Colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram]          = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		return style;
	}

	static float scale_factor()
	{
		return instance().scale_factor_;
	}

	void init(const std::string& custom_font_file, const float base_gui_scaling)
	{
		if(!custom_font_file.empty())
		{
			ImGuiIO& io=ImGui::GetIO();
			io.Fonts->AddFontFromFileTTF(custom_font_file.c_str(), 13.0f*base_gui_scaling);
		}
		else
		{
			ImGuiIO& io=ImGui::GetIO();
			static ImFontConfig font_config=ImFontConfig();
			font_config.FontDataOwnedByAtlas=false;
			io.Fonts->AddFontFromMemoryTTF(reinterpret_cast<void*>(voronota::uv::default_font_mono_regular_data()), voronota::uv::default_font_mono_regular_data_size(), 13.0f*base_gui_scaling, &font_config);
		}
		initialized_=true;
		base_font_scale_factor_=base_gui_scaling;
		set_scale_factor(base_gui_scaling);
	}

	bool scale(const float times)
	{
		return (set_scale_factor(base_font_scale_factor_*times));
	}

private:
	GUIStyleWrapper() : initialized_(false), base_font_scale_factor_(1.0), scale_factor_(1.0f)
	{

	}

	bool set_scale_factor(const float new_scale_factor)
	{
		if(initialized_)
		{
			scale_factor_=std::min(std::max(0.5f, new_scale_factor), 6.0f);

			{
				ImGuiIO& io=ImGui::GetIO();
				io.FontGlobalScale=scale_factor_/base_font_scale_factor_;
			}

			{
				ImGuiStyle& style=ImGui::GetStyle();
				style=default_style();
				style.ScaleAllSizes(scale_factor_);
			}

			return true;
		}
		return false;
	}

	bool initialized_;
	float base_font_scale_factor_;
	float scale_factor_;
};


}

}

#endif /* VIEWER_GUI_STYLE_WRAPPER_H_ */
