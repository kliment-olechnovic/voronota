#ifndef VIEWER_APPLICATION_H_
#define VIEWER_APPLICATION_H_

#include "../uv/viewer_application.h"

#include "../imgui/imgui_impl_glfw_gl3.h"

#include "script_execution_manager.h"

namespace viewer
{

class Application : public uv::ViewerApplication
{
public:
	static Application* self_ptr;

	Application() :
		command_buffer_(1024, 0),
		waiting_stage_(0),
		index_of_history_of_commands_(0),
		script_execution_manager_(*this)
	{
		self_ptr=this;
		set_background_color(0xCCCCCC);
	}

	~Application()
	{
		self_ptr=0;
	}

	void set_console_enabled(const bool enabled)
	{
		console_configuration_.enabled=enabled;
		if(console_configuration_.enabled)
		{
			set_margins(0, 0, (console_configuration_.height+(console_configuration_.padding*2)), 0);
		}
		else
		{
			set_margins(0, 0, 0, 0);
		}
	}

	void add_command(const char* str)
	{
		std::string command(str);
		if(!command.empty())
		{
			pending_commands_.push_back(command);
		}
	}

	void upload_file(const char* name, const char* data)
	{
		std::string object_name=name;
		if(object_name.empty())
		{
			object_name="uploaded.pdb";
		}
		std::string virtual_file_name=std::string("_virtual/")+object_name;
		scripting::VirtualFileStorage::set_file(virtual_file_name, data);
		pending_commands_.push_back(std::string("load --include-heteroatoms --file ")+virtual_file_name+" ; delete-virtual-files "+virtual_file_name);
	}

protected:
	void on_mouse_button_used(int button, int action, int mods)
	{
		ImGui_ImplGlfwGL3_MouseButtonCallback(window(), button, action, mods);
	}

	void on_mouse_cursor_moved(double xpos, double ypos)
	{
		ImGui_ImplGlfwGL3_CursorPosCallback(window(), xpos, ypos);
	}

	void on_key_used(int key, int scancode, int action, int mods)
	{
		if(action==GLFW_PRESS)
		{
			if(key==GLFW_KEY_F4)
			{
				set_console_enabled(!console_configuration_.enabled);
				console_configuration_.need_keyboard_focus=console_configuration_.enabled;
			}
			else if(key==GLFW_KEY_ENTER || key==GLFW_KEY_SPACE || key==GLFW_KEY_UP || key==GLFW_KEY_DOWN)
			{
				console_configuration_.need_keyboard_focus=console_configuration_.enabled;
			}
		}

		ImGui_ImplGlfwGL3_KeyCallback(window(), key, scancode, action, mods);
	}

	void on_character_used(unsigned int codepoint)
	{
		ImGui_ImplGlfwGL3_CharCallback(window(), codepoint);
	}

	void on_draw_simple(const int grid_id)
	{
		script_execution_manager_.draw(false, grid_id);
	}

	void on_draw_with_instancing(const int grid_id)
	{
		script_execution_manager_.draw(true, grid_id);
	}

	void on_draw_overlay()
	{
		const int console_height=30;
		const int console_padding=10;
		const int console_x_pos=console_padding;
		const int console_y_pos=std::max(console_padding, window_height()-(console_height+console_padding));
		const int console_width=window_width()-console_padding*2;

		ImGui_ImplGlfwGL3_NewFrame();

		if(waiting_stage_>0 && waiting_stage_<=2)
		{
			static bool waiting_window=false;
			ImGui::SetNextWindowPos(ImVec2(5, 5));
			ImGui::SetNextWindowSize(ImVec2(150, 30));
			ImGui::Begin("Waiting", &waiting_window, ImVec2(0, 0), 0.75f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
			ImGui::Text("Please wait ...");
			ImGui::End();
		}

		if(console_configuration_.enabled)
		{
			{
				ImGui::SetNextWindowPos(ImVec2(console_x_pos, console_y_pos));
				ImGui::SetNextWindowSize(ImVec2(console_width, console_height));

				ImGui::Begin("Console input", &console_configuration_.enabled, ImVec2(0, 0), 0.75f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);

				{
					ImVec4 color_text=ImVec4(0.0f, 0.0f, 0.7f, 1.0f);
					ImVec4 color_background=ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
					if(ImGui::InputText("", command_buffer_.data(), command_buffer_.size(), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackHistory, &on_command_input_data_request, this))
					{
						pending_commands_.push_back(std::string(command_buffer_.data()));
						update_history_of_commands(pending_commands_.back());
						command_buffer_.assign(command_buffer_.size(), 0);
						console_configuration_.need_keyboard_focus=true;
					}
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}

				if(console_configuration_.need_keyboard_focus)
				{
					ImGui::SetKeyboardFocusHere(-1);
					console_configuration_.need_keyboard_focus=false;
				}

				ImGui::End();
			}
		}

		ImGui::Render();
	}

	void on_before_rendered_frame()
	{
		script_execution_manager_.setup_grid_parameters();
	}

	void on_after_rendered_frame()
	{
		if(!pending_commands_.empty())
		{
			if((++waiting_stage_)>2)
			{
				script_execution_manager_.execute_script(pending_commands_.front(), false);
				pending_commands_.pop_front();
				waiting_stage_=(pending_commands_.empty() ? 0 : 2);
			}
		}

		if(script_execution_manager_.exit_requested())
		{
			close();
		}
	}

	void on_selection(const unsigned int drawing_id, const int button_code, const bool mod_ctrl, const bool mod_shift)
	{
		std::ostringstream output_script;
		script_execution_manager_.generate_click_script(drawing_id, button_code, mod_ctrl, mod_shift, output_script);
		pending_commands_.push_back(output_script.str());
		waiting_stage_=100;
	}

private:
	struct ConsoleConfiguration
	{
		bool enabled;
		bool need_keyboard_focus;
		int padding;
		int height;

		ConsoleConfiguration() :
			enabled(false),
			need_keyboard_focus(false),
			padding(10),
			height(30)
		{
		}
	};

	static int on_command_input_data_request(ImGuiTextEditCallbackData* data)
	{
		Application* app=static_cast<Application*>(data->UserData);
		return app->handle_command_input_data_request(data);
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

	ConsoleConfiguration console_configuration_;
	std::vector<char> command_buffer_;
	std::list<std::string> pending_commands_;
	int waiting_stage_;
	std::vector<std::string> history_of_commands_;
	std::vector<std::string> dynamic_history_of_commands_;
	std::size_t index_of_history_of_commands_;
	ScriptExecutionManager script_execution_manager_;
};

}

#endif /* VIEWER_APPLICATION_H_ */
