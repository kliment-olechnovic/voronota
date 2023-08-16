#ifndef VIEWER_CONSOLE_COMMAND_LINE_INTERFACE_STATE_H_
#define VIEWER_CONSOLE_COMMAND_LINE_INTERFACE_STATE_H_

#include "text_interface_info.h"

#include "common_for_gui.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class CommandLineInterfacePanel
{
public:
	TextInterfaceInfo& text_interface_info;
	float height_for_command_line;

	CommandLineInterfacePanel(TextInterfaceInfo& text_interface_info) :
		text_interface_info(text_interface_info),
		height_for_command_line(20.0f),
		command_buffer_(1024, 0),
		index_of_history_of_commands_(0)
	{
	}

	void execute(std::string& result, const float leave_more_space)
	{
		const float height_for_command_line_and_end=(height_for_command_line*GUIStyleWrapper::scale_factor()+leave_more_space);
		const float height_for_output_block=ImGui::GetWindowHeight()-height_for_command_line_and_end;

		if(height_for_output_block>20.0f*GUIStyleWrapper::scale_factor())
		{
			ImGui::BeginChild("##console_scrolling_region", ImVec2(0, 0-height_for_command_line_and_end), true);
			ImGui::PushItemWidth(-1);
			ImGui::PushTextWrapPos();
			for(std::size_t i=0;i<text_interface_info.outputs.size();i++)
			{
				const TextInterfaceInfo::OutputToken& ot=text_interface_info.outputs[i];
				if(ot.content==TextInterfaceInfo::separator_string())
				{
					ImGui::Separator();
				}
				else
				{
					ImVec4 color_text=ImVec4(ot.r, ot.g, ot.b, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					{
						const std::size_t max_text_chunk_size=5000;
						if(ot.content.size()<=(max_text_chunk_size*2+100))
						{
							ImGuiAddonGlobalTextColorVector gtcv(ot.char_colors.size(), ot.char_colors.data());
							ImGui::TextUnformatted(ot.content.c_str());
						}
						else
						{
							{
								ImGuiAddonGlobalTextColorVector gtcv(max_text_chunk_size, ot.char_colors.data());
								ImGui::TextUnformatted(ot.content.c_str(), ot.content.c_str()+max_text_chunk_size);
							}
							ImGui::TextUnformatted("-------------------------------------------\n");
							ImGui::TextUnformatted("-------- middle part not displayed --------\n");
							ImGui::TextUnformatted("-------------------------------------------\n");
							{
								ImGuiAddonGlobalTextColorVector gtcv(max_text_chunk_size, ot.char_colors.data()+(ot.content.size()-max_text_chunk_size));
								ImGui::TextUnformatted(ot.content.c_str()+(ot.content.size()-max_text_chunk_size), ot.content.c_str()+ot.content.size());
							}
						}
					}
					ImGui::PopStyleColor();
					execute_copy_menu(i, ot.content);
				}
			}
			ImGui::PopTextWrapPos();
			ImGui::PopItemWidth();
			if(text_interface_info.scroll_output)
			{
				ImGui::SetScrollHereY(1.0);
			}
			text_interface_info.scroll_output=false;
			ImGui::EndChild();
		}

		{
			ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec4 color_background=ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, color_text);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
			ImGui::PushItemWidth(-1);
			if(ImGui::InputText("##console_command_input", command_buffer_.data(), command_buffer_.size(), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackHistory|ImGuiInputTextFlags_CallbackAlways, &on_command_input_data_request, this))
			{
				result=(std::string(command_buffer_.data()));
				update_history_of_commands(result);
				command_buffer_.assign(command_buffer_.size(), 0);
				text_interface_info.need_keyboard_focus_in_command_input=true;
			}
			if(text_interface_info.need_keyboard_focus_in_command_input || ((ImGui::IsWindowFocused() || !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) && !ImGui::IsAnyItemActive()))
			{
				ImGui::SetKeyboardFocusHere(-1);
				text_interface_info.need_keyboard_focus_in_command_input=false;
			}
			ImGui::PopItemWidth();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
	}

private:
	static void execute_copy_menu(const std::size_t index, const std::string& content)
	{
		std::ostringstream menu_id_output;
		menu_id_output << "Copy text##copy_output_" << index;
		const std::string menu_id=menu_id_output.str();
		if(ImGui::BeginPopupContextItem(menu_id.c_str(), 1))
		{
			if(ImGui::Selectable("Copy to clipboard"))
			{
				ImGui::SetClipboardText(content.c_str());
			}
			ImGui::EndPopup();
		}
	}

	static int on_command_input_data_request(ImGuiInputTextCallbackData* data)
	{
		CommandLineInterfacePanel* obj=static_cast<CommandLineInterfacePanel*>(data->UserData);
		return obj->handle_command_input_data_request(data);
	}

	int handle_command_input_data_request(ImGuiInputTextCallbackData* data)
	{
		if(
				!text_interface_info.history_of_commands.empty() &&
				data->EventFlag==ImGuiInputTextFlags_CallbackHistory &&
				(data->EventKey==ImGuiKey_UpArrow || data->EventKey==ImGuiKey_DownArrow) &&
				data->BufSize>0
			)
		{
			if(dynamic_history_of_commands_.empty())
			{
				index_of_history_of_commands_=0;
				dynamic_history_of_commands_.push_back(std::string(data->Buf));
				dynamic_history_of_commands_.insert(dynamic_history_of_commands_.end(), text_interface_info.history_of_commands.rbegin(), text_interface_info.history_of_commands.rend());
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
				!text_interface_info.next_prefix.empty() &&
				(text_interface_info.next_prefix.size()+2)<static_cast<std::size_t>(data->BufSize)
			)
		{
			for(size_t i=0;i<=text_interface_info.next_prefix.size();i++)
			{
				data->Buf[i]=text_interface_info.next_prefix.c_str()[i];
			}
			data->BufDirty=true;
			data->BufTextLen=static_cast<int>(text_interface_info.next_prefix.size());
			data->CursorPos=data->BufTextLen;
			data->SelectionStart=data->BufTextLen;
			data->SelectionEnd=data->BufTextLen;
			text_interface_info.next_prefix.clear();
		}
		return 0;
	}

	void update_history_of_commands(const std::string& command)
	{
		if(!command.empty() && (text_interface_info.history_of_commands.empty() || command!=text_interface_info.history_of_commands.back()))
		{
			text_interface_info.history_of_commands.push_back(command);
		}
		dynamic_history_of_commands_.clear();
	}

	std::vector<std::string> dynamic_history_of_commands_;
	std::vector<char> command_buffer_;
	std::size_t index_of_history_of_commands_;
};

}

}

}

#endif /* VIEWER_CONSOLE_COMMAND_LINE_INTERFACE_STATE_H_ */
