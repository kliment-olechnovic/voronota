#ifndef VIEWER_WIDGETS_CURSOR_LABEL_H_
#define VIEWER_WIDGETS_CURSOR_LABEL_H_

#include <string>

#include "../gui_style_wrapper.h"

namespace voronota
{

namespace viewer
{

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

		ImGui::SetNextWindowPos(ImVec2(mouse_x+5.0f*GUIStyleWrapper::scale_factor(), std::max(0.0f, mouse_y-35.0f*GUIStyleWrapper::scale_factor())), 0);
		ImGui::SetNextWindowSize(ImVec2(3+(text.size()*8)*GUIStyleWrapper::scale_factor(), 30*GUIStyleWrapper::scale_factor()), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.5f);
		ImGui::Begin("Label", &open, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
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

}

}

#endif /* VIEWER_WIDGETS_CURSOR_LABEL_H_ */
