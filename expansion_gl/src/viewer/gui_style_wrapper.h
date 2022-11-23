#ifndef VIEWER_GUI_STYLE_WRAPPER_H_
#define VIEWER_GUI_STYLE_WRAPPER_H_

#include "../dependencies/imgui/imgui_impl_glfw.h"

namespace voronota
{

namespace viewer
{

class GUIStyleWrapper
{
public:
	static bool& initialized()
	{
		static bool value=false;
		return value;
	}

	static float scale_factor()
	{
		return mutable_scale_factor();
	}

	static bool set_scale_factor(const float value)
	{
		if(initialized() && value>0.2f && value<=5.0f)
		{
			mutable_scale_factor()=value;

			{
				ImGuiIO& io=ImGui::GetIO();
				io.FontGlobalScale=value;
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
	static float& mutable_scale_factor()
	{
		static float value=1.0f;
		return value;
	}
};


}

}

#endif /* VIEWER_GUI_STYLE_WRAPPER_H_ */
