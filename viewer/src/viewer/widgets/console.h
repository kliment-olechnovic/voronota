#ifndef VIEWER_WIDGETS_CONSOLE_H_
#define VIEWER_WIDGETS_CONSOLE_H_

#include <string>
#include <list>
#include <vector>

#include "../../imgui/imgui_impl_glfw_gl3.h"

namespace voronota
{

namespace viewer
{

namespace widgets
{

class Console
{
public:
	Console() :
		command_buffer_(1024, 0),
		index_of_history_of_commands_(0),
		enabled_(false),
		need_keyboard_focus_(false)
	{
	}

	static int height()
	{
		return 30;
	}

	bool enabled() const
	{
		return enabled_;
	}

	bool focused() const
	{
		return need_keyboard_focus_;
	}

	void set_enabled(const bool status)
	{
		need_keyboard_focus_=status && (!enabled_ || need_keyboard_focus_);
		enabled_=status;
	}

	void set_focused(const bool status)
	{
		need_keyboard_focus_=status && enabled_;
	}

	void set_next_prefix(const std::string& prefix)
	{
		next_prefix_=prefix;
	}

	std::string execute(const int width, const int x_pos, const int y_pos)
	{
		std::string result;

		if(!enabled_)
		{
			return result;
		}

		ImGui::SetNextWindowPos(ImVec2(x_pos, y_pos));
		ImGui::SetNextWindowSize(ImVec2(width, height()));

		ImGui::Begin("Console input", &enabled_, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);

		{
			ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec4 color_background=ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
			ImGui::PushStyleColor(ImGuiCol_Text, color_text);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
			ImGui::PushItemWidth(-1);
			if(ImGui::InputText("", command_buffer_.data(), command_buffer_.size(), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackHistory|ImGuiInputTextFlags_CallbackAlways, &on_command_input_data_request, this))
			{
				result=(std::string(command_buffer_.data()));
				update_history_of_commands(result);
				command_buffer_.assign(command_buffer_.size(), 0);
				need_keyboard_focus_=true;
			}
			ImGui::PopItemWidth();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}

		if(need_keyboard_focus_)
		{
			ImGui::SetKeyboardFocusHere(-1);
			need_keyboard_focus_=false;
		}

		ImGui::End();

		return result;
	}

	std::string execute_on_bottom(const int window_width, const int window_height, const int padding)
	{
		const int x_pos=padding;
		const int y_pos=std::max(padding, window_height-(height()+padding));
		const int width=window_width-padding*2;
		return execute(width, x_pos, y_pos);
	}

private:
	static int on_command_input_data_request(ImGuiTextEditCallbackData* data)
	{
		Console* obj=static_cast<Console*>(data->UserData);
		return obj->handle_command_input_data_request(data);
	}

	int handle_command_input_data_request(ImGuiTextEditCallbackData* data)
	{
		if(
				!history_of_commands_.empty() &&
				data->EventFlag==ImGuiInputTextFlags_CallbackHistory &&
				(data->EventKey==ImGuiKey_UpArrow || data->EventKey==ImGuiKey_DownArrow) &&
				data->BufSize>0
			)
		{
			if(dynamic_history_of_commands_.empty())
			{
				index_of_history_of_commands_=0;
				dynamic_history_of_commands_.push_back(std::string(data->Buf));
				dynamic_history_of_commands_.insert(dynamic_history_of_commands_.end(), history_of_commands_.rbegin(), history_of_commands_.rend());
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
				!next_prefix_.empty() &&
				(next_prefix_.size()+2)<static_cast<std::size_t>(data->BufSize)
			)
		{
			for(size_t i=0;i<=next_prefix_.size();i++)
			{
				data->Buf[i]=next_prefix_.c_str()[i];
			}
			data->BufDirty=true;
			data->BufTextLen=static_cast<int>(next_prefix_.size());
			data->CursorPos=data->BufTextLen;
			data->SelectionStart=data->BufTextLen;
			data->SelectionEnd=data->BufTextLen;
			next_prefix_.clear();
		}
		return 0;
	}

	void update_history_of_commands(const std::string& command)
	{
		if(!command.empty() && (history_of_commands_.empty() || command!=history_of_commands_.back()))
		{
			history_of_commands_.push_back(command);
		}
		dynamic_history_of_commands_.clear();
	}

	std::vector<std::string> history_of_commands_;
	std::vector<std::string> dynamic_history_of_commands_;
	std::vector<char> command_buffer_;
	std::string next_prefix_;
	std::size_t index_of_history_of_commands_;
	bool enabled_;
	bool need_keyboard_focus_;
};

}

}

}

#endif /* VIEWER_WIDGETS_CONSOLE_H_ */
