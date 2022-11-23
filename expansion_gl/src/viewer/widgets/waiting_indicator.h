#ifndef VIEWER_WIDGETS_WAITING_INDICATOR_H_
#define VIEWER_WIDGETS_WAITING_INDICATOR_H_

#include "../gui_style_wrapper.h"

namespace voronota
{

namespace viewer
{

namespace widgets
{

class WaitingIndicator
{
public:
	static WaitingIndicator& instance()
	{
		static WaitingIndicator obj;
		return obj;
	}

	bool decided() const
	{
		return (decided_);
	}

	bool activated() const
	{
		return (decided_ && activated_);
	}

	bool executed() const
	{
		return (decided_ && activated_ && executed_>1);
	}

	void set_activated(const bool activated)
	{
		decided_=true;
		activated_=activated;
	}

	void reset()
	{
		decided_=false;
		activated_=false;
		executed_=0;
	}

	void execute(const int box_x, const int box_y, const int box_w, const int box_h)
	{
		if(!activated_)
		{
			return;
		}

		const int label_width=static_cast<int>(150*GUIStyleWrapper::scale_factor());
		const int label_height=static_cast<int>(30*GUIStyleWrapper::scale_factor());
		const int label_x_pos=box_x+(box_w/2)-(label_width/2);
		const int label_y_pos=box_y+(box_h/2)-(label_height/2);

		ImGui::SetNextWindowPos(ImVec2(label_x_pos, label_y_pos));
		ImGui::SetNextWindowSize(ImVec2(label_width, label_height), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.75f);
		ImGui::Begin("Waiting", 0, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("Please wait ...");
		ImGui::End();

		executed_++;
	}

private:
	WaitingIndicator() :
		decided_(false),
		activated_(false),
		executed_(0)
	{
	}

	bool decided_;
	bool activated_;
	int executed_;
};

}

}

}

#endif /* VIEWER_WIDGETS_WAITING_INDICATOR_H_ */
