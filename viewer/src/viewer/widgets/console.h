#ifndef VIEWER_WIDGETS_CONSOLE_H_
#define VIEWER_WIDGETS_CONSOLE_H_

#include <string>
#include <list>
#include <vector>
#include <deque>

#include "../../dependencies/imgui/imgui_impl_glfw_gl3.h"

namespace voronota
{

namespace viewer
{

namespace widgets
{

class Console
{
public:
	struct OutputToken
	{
		std::string content;
		float r;
		float g;
		float b;

		OutputToken(const std::string& content, const float r, const float g, const float b) : content(content), r(r), g(g), b(b)
		{
		}
	};

	static Console& instance()
	{
		static Console console;
		return console;
	}

	bool focused() const
	{
		return need_keyboard_focus_;
	}

	bool open_script_editor() const
	{
		return open_script_editor_;
	}

	void set_focused(const bool status)
	{
		need_keyboard_focus_=status;
	}

	void set_next_prefix(const std::string& prefix)
	{
		next_prefix_=prefix;
	}

	void add_output(const std::string& content, const float r, const float g, const float b)
	{
		outputs_.push_back(OutputToken(content, r, g, b));
		if(outputs_.size()>50)
		{
			outputs_.pop_front();
		}
		scroll_output_=true;
	}

	std::string execute(const int x_pos, const int y_pos, const int width, const int recommended_height, const int min_height, const int max_height)
	{
		std::string result;

		ImGui::SetNextWindowPos(ImVec2(x_pos, y_pos));
		ImGui::SetNextWindowSizeConstraints(ImVec2(width, min_height), ImVec2(width, max_height));

		if(!ImGui::Begin("Console", 0, ImVec2(width, recommended_height), 0.5f, ImGuiWindowFlags_ShowBorders|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_MenuBar))
		{
			current_heigth_=ImGui::GetWindowHeight();
			ImGui::End();
			return result;
		}

		bool focus_in_script_editor=false;

        if(ImGui::BeginMenuBar())
        {
            if(ImGui::BeginMenu("Windows"))
            {
				if(ImGui::MenuItem("Script editor"))
				{
					open_script_editor_=true;
					focus_in_script_editor=true;
				}
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

		{
			ImGui::BeginChild("##console_scrolling_region", ImVec2(0,-ImGui::GetItemsLineHeightWithSpacing()));
			ImGui::PushItemWidth(-1);
			ImGui::PushTextWrapPos();
			for(std::size_t i=0;i<outputs_.size();i++)
			{
				const OutputToken& ot=outputs_[i];
				if(ot.content=="---")
				{
					ImGui::Separator();
				}
				else
				{
					float col[4]={ot.r, ot.g, ot.b, 1.0f};
					ImVec4 color_text=ImVec4(col[0], col[1], col[2], col[3]);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					if(ot.content.size()<10000)
					{
						ImGui::TextUnformatted(ot.content.c_str());
					}
					else
					{
						ImGui::TextUnformatted(ot.content.c_str(), &(ot.content.c_str()[4000]));
						ImGui::TextUnformatted("------------------------------");
						ImGui::TextUnformatted("------------ skip ------------");
						ImGui::TextUnformatted("------------------------------");
						ImGui::TextUnformatted(&(ot.content.c_str()[ot.content.size()-4000]), &(ot.content.c_str()[ot.content.size()-1]));
					}
					ImGui::PopStyleColor();
				}
			}
			ImGui::PopTextWrapPos();
			ImGui::PopItemWidth();
			if(scroll_output_)
			{
				ImGui::SetScrollHere();
			}
			scroll_output_=false;
			ImGui::EndChild();
		}

		{
			ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec4 color_background=ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, color_text);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
			ImGui::PushItemWidth(-1);
			if(ImGui::InputText("##console_input", command_buffer_.data(), command_buffer_.size(), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackHistory|ImGuiInputTextFlags_CallbackAlways, &on_command_input_data_request, this))
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

		current_heigth_=ImGui::GetWindowHeight();

		ImGui::End();

		if(open_script_editor_)
		{
			if(!ImGui::Begin("Script editor", &open_script_editor_, ImVec2(600, 400), 0.8f, ImGuiWindowFlags_ShowBorders|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_MenuBar))
			{
				ImGui::End();
				return result;
			}

			if(ImGui::BeginMenuBar())
			{
				if(ImGui::BeginMenu("Examples"))
				{
					if(ImGui::MenuItem("Import structure"))
					{
						const std::string script="import 2zsk.pdb";
						for(std::size_t i=0;i<script.size() && (i+1)<multiline_command_buffer_.size();i++)
						{
							multiline_command_buffer_[i]=script[i];
							multiline_command_buffer_[i+1]=0;
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::BeginChild("##script_editor_scrolling_region", ImVec2(0,-ImGui::GetItemsLineHeightWithSpacing()));
			ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec4 color_background=ImVec4(0.25f, 0.25f, 0.25f, 0.4f);
			ImGui::PushStyleColor(ImGuiCol_Text, color_text);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
			ImGui::PushItemWidth(-1);
			ImGui::InputTextMultiline("##script_editor_input", multiline_command_buffer_.data(), multiline_command_buffer_.size(), ImVec2(-1,-1), ImGuiInputTextFlags_AllowTabInput);
			if(focus_in_script_editor)
			{
				ImGui::SetKeyboardFocusHere(-1);
			}
			ImGui::PopItemWidth();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::EndChild();

			if(ImGui::Button("Run and close", ImVec2(100,0)))
			{
				result=(std::string(multiline_command_buffer_.data()));
				open_script_editor_=false;
			}

			ImGui::SameLine();

			if(ImGui::Button("Run", ImVec2(100,0)))
			{
				result=(std::string(multiline_command_buffer_.data()));
			}

			ImGui::SameLine();

			if(ImGui::Button("Clear", ImVec2(100,0)))
			{
				multiline_command_buffer_[0]=0;
			}

			ImGui::SameLine();

			if(ImGui::Button("Close", ImVec2(100,0)))
			{
				open_script_editor_=false;
			}

			ImGui::End();
		}

		return result;
	}

	int current_heigth() const
	{
		return static_cast<int>(current_heigth_);
	}

private:
	Console() :
		command_buffer_(1024, 0),
		multiline_command_buffer_(16384, 0),
		index_of_history_of_commands_(0),
		scroll_output_(false),
		need_keyboard_focus_(false),
		current_heigth_(0.0f),
		open_script_editor_(false)
	{
	}

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

	std::deque<OutputToken> outputs_;
	std::vector<std::string> history_of_commands_;
	std::vector<std::string> dynamic_history_of_commands_;
	std::vector<char> command_buffer_;
	std::vector<char> multiline_command_buffer_;
	std::string next_prefix_;
	std::size_t index_of_history_of_commands_;
	bool scroll_output_;
	bool need_keyboard_focus_;
	float current_heigth_;
	bool open_script_editor_;
};

}

}

}

#endif /* VIEWER_WIDGETS_CONSOLE_H_ */
