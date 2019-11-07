#ifndef VIEWER_APPLICATION_H_
#define VIEWER_APPLICATION_H_

#include "../uv/viewer_application.h"

#include "../imgui/imgui_impl_glfw_gl3.h"

#include "script_execution_manager.h"
#include "reading_thread.h"

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
	}

	void add_command(const char* str)
	{
		std::string command(str);
		if(!command.empty())
		{
			pending_commands_.push_back(command);
		}
	}

	const char* execute_command(const char* str)
	{
		static std::string last_output_string;

		const std::string command(str);

		if(!command.empty())
		{
			script_execution_manager_.execute_script(command, false);
			std::ostringstream raw_output;
			scripting::JSONWriter::write(scripting::JSONWriter::Configuration(0), script_execution_manager_.last_output(), raw_output);
			last_output_string=raw_output.str();
		}
		else
		{
			last_output_string.clear();
		}

		return last_output_string.c_str();
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
		pending_commands_.push_back(std::string("import --include-heteroatoms --file ")+virtual_file_name+" ; delete-virtual-files "+virtual_file_name);
	}

protected:
	bool check_mouse_button_use_intercepted(int button, int action, int mods)
	{
		ImGui_ImplGlfwGL3_MouseButtonCallback(window(), button, action, mods);
		return (ImGui::IsMouseHoveringAnyWindow());
	}

	bool check_mouse_cursor_move_intercepted(double xpos, double ypos)
	{
		ImGui_ImplGlfwGL3_CursorPosCallback(window(), xpos, ypos);
		return (ImGui::IsMouseHoveringAnyWindow());
	}

	void on_mouse_cursor_moved(double xpos, double ypos)
	{
		cursor_label_.clear();
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
				if(hovered())
				{
					console_configuration_.need_keyboard_focus=console_configuration_.enabled;
				}
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
		ImGui_ImplGlfwGL3_NewFrame();

		if(console_configuration_.enabled)
		{
			if(ImGui::BeginMainMenuBar())
			{
				if(ImGui::BeginMenu("Settings"))
				{
					if(ImGui::BeginMenu("Background color"))
					{
						if(ImGui::MenuItem("White", ""))
						{
							pending_commands_.push_back(std::string("background white"));
						}
						if(ImGui::MenuItem("Gray", ""))
						{
							pending_commands_.push_back(std::string("background 0xCCCCCC"));
						}
						if(ImGui::MenuItem("Black", ""))
						{
							pending_commands_.push_back(std::string("background black"));
						}
						ImGui::EndMenu();
					}
					if(ImGui::BeginMenu("Grid mode"))
					{
						if(ImGui::MenuItem("Disable", "", false, rendering_mode_is_grid()))
						{
							pending_commands_.push_back(std::string("mono"));
						}
						if(ImGui::MenuItem("Enable grid by object", "", false, !rendering_mode_is_grid()))
						{
							pending_commands_.push_back(std::string("grid-by-object"));
						}
						ImGui::EndMenu();
					}
					if(ImGui::BeginMenu("Projection mode"))
					{
						if(ImGui::MenuItem("Orthographic", "", false, !projection_mode_is_ortho()))
						{
							pending_commands_.push_back(std::string("ortho"));
						}
						if(ImGui::MenuItem("Perspective", "", false, !projection_mode_is_perspective()))
						{
							pending_commands_.push_back(std::string("perspective"));
						}
						ImGui::EndMenu();
					}
					if(ImGui::BeginMenu("Stereo"))
					{
						if(ImGui::MenuItem("Disable", "", false, rendering_mode_is_stereo()))
						{
							pending_commands_.push_back(std::string("mono"));
						}
						if(ImGui::MenuItem("Enable", "", false, !rendering_mode_is_stereo()))
						{
							pending_commands_.push_back(std::string("stereo"));
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
		}

		if(waiting_stage_>0 && waiting_stage_<=2)
		{
			const int label_width=150;
			const int label_height=30;
			const int label_x_pos=(window_width()/2)-(label_width/2);
			const int label_y_pos=(window_height()/2)-(label_height/2);

			{
				static bool waiting_window=false;
				ImGui::SetNextWindowPos(ImVec2(label_x_pos, label_y_pos));
				ImGui::SetNextWindowSize(ImVec2(label_width, label_height));
				ImGui::Begin("Waiting", &waiting_window, ImVec2(0, 0), 0.75f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
				ImGui::Text("Please wait ...");
				ImGui::End();
			}
		}

		if(!cursor_label_.empty())
		{
			static bool cursor_label_window=false;
			ImGui::SetNextWindowPos(ImVec2(mouse_x()+5.0f, std::max(0.0f, mouse_y()-35.0f)), 0);
			ImGui::SetNextWindowSize(ImVec2(3+(cursor_label_.size()*8), 30));
			ImGui::Begin("Label", &cursor_label_window, ImVec2(0, 0), 0.75f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
			ImGui::Text("%s", cursor_label_.c_str());
			ImGui::End();
		}

		if(console_configuration_.enabled)
		{
			const int console_height=30;
			const int console_padding=2;
			const int console_x_pos=console_padding;
			const int console_y_pos=std::max(console_padding, window_height()-(console_height+console_padding));
			const int console_width=window_width()-console_padding*2;

			{
				ImGui::SetNextWindowPos(ImVec2(console_x_pos, console_y_pos));
				ImGui::SetNextWindowSize(ImVec2(console_width, console_height));

				ImGui::Begin("Console input", &console_configuration_.enabled, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);

				{
					ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
					ImVec4 color_background=ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
					ImGui::PushItemWidth(-1);
					if(ImGui::InputText("", command_buffer_.data(), command_buffer_.size(), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackHistory, &on_command_input_data_request, this))
					{
						pending_commands_.push_back(std::string(command_buffer_.data()));
						update_history_of_commands(pending_commands_.back());
						command_buffer_.assign(command_buffer_.size(), 0);
						console_configuration_.need_keyboard_focus=true;
					}
					ImGui::PopItemWidth();
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
		if(ReadingThread::check_data())
		{
			script_execution_manager_.set_output_stream_mode(1);
			script_execution_manager_.execute_script(ReadingThread::extract_data(), false);
			script_execution_manager_.set_output_stream_mode(0);
		}

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
		if(button_code==2)
		{
			std::ostringstream output_label;
			if(script_execution_manager_.generate_click_label(drawing_id, output_label))
			{
				cursor_label_=output_label.str();
			}
		}
		{
			std::ostringstream output_script;
			if(script_execution_manager_.generate_click_script(drawing_id, button_code, mod_ctrl, mod_shift, output_script))
			{
				pending_commands_.push_back(output_script.str());
			}
		}
		waiting_stage_=100;
	}

private:
	struct ConsoleConfiguration
	{
		bool enabled;
		bool need_keyboard_focus;

		ConsoleConfiguration() :
			enabled(false),
			need_keyboard_focus(false)
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
	std::string cursor_label_;
	ScriptExecutionManager script_execution_manager_;
};

}

#endif /* VIEWER_APPLICATION_H_ */
