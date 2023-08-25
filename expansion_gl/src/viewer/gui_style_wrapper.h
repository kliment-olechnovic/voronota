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
		scale(base_gui_scaling, false);
	}

	static float scale_factor()
	{
		return instance().scale_factor_;
	}

	bool scale(const float value, const bool scale_font)
	{
		if(initialized_ && value>=0.2f && value<=5.0f)
		{
			scale_factor_*=value;

			if(scale_font)
			{
				ImGuiIO& io=ImGui::GetIO();
				io.FontGlobalScale=scale_factor_;
			}

			{
				ImGuiStyle& style=ImGui::GetStyle();
				style.ScaleAllSizes(value);
			}

			return true;
		}
		return false;
	}

private:
	GUIStyleWrapper() : initialized_(false), scale_factor_(1.0f)
	{

	}

	bool initialized_;
	float scale_factor_;
};


}

}

#endif /* VIEWER_GUI_STYLE_WRAPPER_H_ */
