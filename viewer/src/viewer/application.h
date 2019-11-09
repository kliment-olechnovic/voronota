#ifndef VIEWER_APPLICATION_H_
#define VIEWER_APPLICATION_H_

#include "../uv/viewer_application.h"

#include "../widgets/console.h"
#include "../widgets/waiting_indicator.h"

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
		script_execution_manager_(*this),
		menu_enabled_(false)
	{
		self_ptr=this;
		set_background_color(0xCCCCCC);
	}

	~Application()
	{
		self_ptr=0;
	}

	void add_command(const std::string& command)
	{
		if(!command.empty())
		{
			pending_commands_.push_back(command);
		}
	}

	void add_command(const char* str)
	{
		add_command(std::string(str));
	}

	const char* execute_command(const std::string& command)
	{
		static std::string last_output_string;

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

	const char* execute_command(const char* str)
	{
		return execute_command(std::string(str));
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
		add_command(std::string("import --include-heteroatoms --file ")+virtual_file_name+" ; delete-virtual-files "+virtual_file_name);
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
				console_.set_enabled(!console_.enabled());
				menu_enabled_=!menu_enabled_;
			}
			else if(key==GLFW_KEY_ENTER || key==GLFW_KEY_SPACE || key==GLFW_KEY_UP || key==GLFW_KEY_DOWN)
			{
				if(hovered())
				{
					console_.set_focused(true);
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

		execute_menu();

		waiting_indicator_.execute(window_width(), window_height());

		if(!cursor_label_.empty())
		{
			static bool cursor_label_window=false;
			ImGui::SetNextWindowPos(ImVec2(mouse_x()+5.0f, std::max(0.0f, mouse_y()-35.0f)), 0);
			ImGui::SetNextWindowSize(ImVec2(3+(cursor_label_.size()*8), 30));
			ImGui::Begin("Label", &cursor_label_window, ImVec2(0, 0), 0.75f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
			ImGui::Text("%s", cursor_label_.c_str());
			ImGui::End();
		}

		add_command(console_.execute_on_bottom(window_width(), window_height(), 2));

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
			if(!waiting_indicator_.check_waiting())
			{
				script_execution_manager_.execute_script(pending_commands_.front(), false);
				pending_commands_.pop_front();
				waiting_indicator_.keep_waiting(!pending_commands_.empty());
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
				add_command(output_script.str());
			}
		}
		waiting_indicator_.disable();
	}

private:
	void execute_menu()
	{
		if(!menu_enabled_)
		{
			return;
		}

		if(ImGui::BeginMainMenuBar())
		{
			if(ImGui::BeginMenu("Settings"))
			{
				if(ImGui::BeginMenu("Background color"))
				{
					if(ImGui::MenuItem("White", ""))
					{
						add_command("background white");
					}
					if(ImGui::MenuItem("Gray", ""))
					{
						add_command("background 0xCCCCCC");
					}
					if(ImGui::MenuItem("Black", ""))
					{
						add_command("background black");
					}
					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Grid mode"))
				{
					if(ImGui::MenuItem("Disable", "", false, rendering_mode_is_grid()))
					{
						add_command("mono");
					}
					if(ImGui::MenuItem("Enable grid by object", "", false, !rendering_mode_is_grid()))
					{
						add_command("grid-by-object");
					}
					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Projection mode"))
				{
					if(ImGui::MenuItem("Orthographic", "", false, !projection_mode_is_ortho()))
					{
						add_command("ortho");
					}
					if(ImGui::MenuItem("Perspective", "", false, !projection_mode_is_perspective()))
					{
						add_command("perspective");
					}
					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Stereo"))
				{
					if(ImGui::MenuItem("Disable", "", false, rendering_mode_is_stereo()))
					{
						add_command("mono");
					}
					if(ImGui::MenuItem("Enable", "", false, !rendering_mode_is_stereo()))
					{
						add_command("stereo");
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	ScriptExecutionManager script_execution_manager_;
	std::list<std::string> pending_commands_;
	widgets::Console console_;
	widgets::WaitingIndicator waiting_indicator_;
	std::string cursor_label_;
	bool menu_enabled_;
};

}

#endif /* VIEWER_APPLICATION_H_ */
