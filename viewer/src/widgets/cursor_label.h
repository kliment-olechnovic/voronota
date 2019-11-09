#ifndef WIDGETS_CURSOR_LABEL_H_
#define WIDGETS_CURSOR_LABEL_H_

#include <string>

#include "../imgui/imgui_impl_glfw_gl3.h"

namespace widgets
{

class CursorLabel
{
public:
	CursorLabel()
	{
	}

	void set(const std::string& text)
	{
		text_=text;
	}

	void clear()
	{
		text_.clear();
	}

	static void execute(const float mouse_x, const float mouse_y, const std::string& text)
	{
		if(text.empty())
		{
			return;
		}

		static bool open=false;

		ImGui::SetNextWindowPos(ImVec2(mouse_x+5.0f, std::max(0.0f, mouse_y-35.0f)), 0);
		ImGui::SetNextWindowSize(ImVec2(3+(text.size()*8), 30));
		ImGui::Begin("Label", &open, ImVec2(0, 0), 0.75f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("%s", text.c_str());
		ImGui::End();
	}

	void execute(const float mouse_x, const float mouse_y)
	{
		execute(mouse_x, mouse_y, text_);
	}

private:
	std::string text_;
};

}

#endif /* WIDGETS_CURSOR_LABEL_H_ */
