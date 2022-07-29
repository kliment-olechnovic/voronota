#ifndef VIEWER_APPLICATION_H_
#define VIEWER_APPLICATION_H_

#include "../uv/viewer_application.h"

#include "../dependencies/imgui/imgui_impl_glfw.h"
#include "../dependencies/imgui/imgui_impl_opengl3.h"

#include "../../../expansion_js/src/duktaper/binding_javascript.h"
#include "../../../expansion_js/src/duktaper/duktape_manager.h"

#include "script_execution_manager.h"
#include "widgets/cursor_label.h"
#include "widgets/waiting_indicator.h"

namespace voronota
{

namespace viewer
{

class Application : public uv::ViewerApplication
{
public:
	static Application& instance()
	{
		static Application app;
		return app;
	}

	bool enqueue_script(const std::string& script)
	{
		const ScriptPrefixParsing::Bundle task=ScriptPrefixParsing::parse(script);
		return enqueue_script(task);
	}

	bool enqueue_file(const std::string& filename)
	{
		return enqueue_job(JobFile(filename));
	}

	const std::string& execute_native_script(const std::string& script)
	{
		while(!job_queue_.empty())
		{
			dequeue_job();
		}
		return script_execution_manager_.execute_script_and_return_last_output_string(script, false);
	}

	const std::string& get_last_script_output()
	{
		return script_execution_manager_.last_output_string();
	}

	void upload_file(const std::string& name, const std::string& data, const std::string& parameters)
	{
		std::string object_name=name;
		if(object_name.empty())
		{
			object_name="uploaded.pdb";
		}
		std::string virtual_file_name=std::string("_virtual/")+object_name;
		scripting::VirtualFileStorage::set_file(virtual_file_name, data);
		enqueue_script(std::string("import --file ")+virtual_file_name+" "+parameters+" ; delete-virtual-files "+virtual_file_name);
	}

protected:
	void on_after_init_success()
	{
		if(good())
		{
			set_background_color(0xCCCCCC);
			set_margin_color(0x7F7F7F);
			set_margin_top_fixed(200);

			ImGui::CreateContext();

			ImGui_ImplGlfw_InitForOpenGL(window(), false);
			ImGui_ImplOpenGL3_Init();

			{
				ImGuiStyle& style=ImGui::GetStyle();

				style.WindowRounding                          = 0.0f;
				style.FrameRounding                           = 0.0f;
				style.GrabRounding                            = 0.0f;
				style.ScrollbarRounding                       = 0.0f;

				style.Colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
				style.Colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
				style.Colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
				style.Colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
				style.Colors[ImGuiCol_Border]                 = ImVec4(0.63f, 0.63f, 0.63f, 0.50f);
				style.Colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
				style.Colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
				style.Colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
				style.Colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
				style.Colors[ImGuiCol_TitleBg]                = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
				style.Colors[ImGuiCol_TitleBgActive]          = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
				style.Colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
				style.Colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
				style.Colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
				style.Colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
				style.Colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
				style.Colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
				style.Colors[ImGuiCol_CheckMark]              = ImVec4(0.74f, 0.74f, 0.74f, 1.00f);
				style.Colors[ImGuiCol_SliderGrab]             = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
				style.Colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
				style.Colors[ImGuiCol_Button]                 = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
				style.Colors[ImGuiCol_ButtonHovered]          = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
				style.Colors[ImGuiCol_ButtonActive]           = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
				style.Colors[ImGuiCol_Header]                 = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
				style.Colors[ImGuiCol_HeaderHovered]          = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
				style.Colors[ImGuiCol_HeaderActive]           = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
				style.Colors[ImGuiCol_ResizeGrip]             = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
				style.Colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
				style.Colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
				style.Colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
				style.Colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
				style.Colors[ImGuiCol_PlotHistogram]          = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
				style.Colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
				style.Colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
				style.Colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

				ImGui::GetIO().IniFilename=0;
			}

			duktaper::DuktapeManager::set_output_director(DuktaperOutputDirector::instance());
			duktaper::DuktapeManager::set_script_execution_manager(script_execution_manager_);
			duktaper::DuktapeManager::flag_to_print_result_on_eval()=false;
			duktaper::DuktapeManager::eval(duktaper::BindingJavascript::generate_setup_script(script_execution_manager_.collection_of_command_documentations()));
			duktaper::DuktapeManager::flag_to_print_result_on_eval()=true;
		}
	}

	bool check_window_scroll_intercepted(double xoffset, double yoffset)
	{
		ImGui_ImplGlfw_ScrollCallback(window(), xoffset, yoffset);
		return (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
	}

	bool check_mouse_button_use_intercepted(int button, int action, int mods)
	{
		ImGui_ImplGlfw_MouseButtonCallback(window(), button, action, mods);
		return (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
	}

	bool check_mouse_cursor_move_intercepted(double xpos, double ypos)
	{
		ImGui_ImplGlfw_CursorPosCallback(window(), xpos, ypos);
		return (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
	}

	void on_mouse_cursor_moved(double xpos, double ypos)
	{
		cursor_label_.clear();
	}

	void on_key_used(int key, int scancode, int action, int mods)
	{
		if(action==GLFW_PRESS)
		{
			if(key==GLFW_KEY_ENTER || key==GLFW_KEY_ESCAPE || key==GLFW_KEY_UP || key==GLFW_KEY_DOWN)
			{
				if(hovered())
				{
					Console::instance().set_need_keyboard_focus_in_command_input(true);
				}
			}
		}

		ImGui_ImplGlfw_KeyCallback(window(), key, scancode, action, mods);
	}

	void on_character_used(unsigned int codepoint)
	{
		ImGui_ImplGlfw_CharCallback(window(), codepoint);
	}

	void on_draw(const uv::ShadingMode::Mode shading_mode, const int grid_id)
	{
		script_execution_manager_.draw(shading_mode, grid_id);
	}

	void on_draw_overlay_start(const int box_x, const int box_y, const int box_w, const int box_h)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if(GUIConfiguration::instance().enabled_cursor_label)
		{
			cursor_label_.execute(mouse_x(),  mouse_y());
		}

		{
			const std::string console_result=Console::instance().execute(0, 0, window_width()/5*3, 200, window_width()/4, window_width(), 40, window_height());
			if(!console_result.empty())
			{
				const ScriptPrefixParsing::Bundle task=ScriptPrefixParsing::parse(console_result);
				enqueue_script(task);
				if(!task.prefix.empty() && task.mode!=ScriptPrefixParsing::MODE_NATIVE_BRIEF)
				{
					Console::instance().set_next_prefix(task.prefix+" ");
				}
			}
		}
	}

	void on_draw_overlay_middle(const int box_x, const int box_y, const int box_w, const int box_h, const bool stereo, const bool grid, const int id)
	{
	}

	void on_draw_overlay_end(const int box_x, const int box_y, const int box_w, const int box_h)
	{
		if(GUIConfiguration::instance().enabled_waiting_indicator)
		{
			if(!widgets::WaitingIndicator::instance().decided())
			{
				widgets::WaitingIndicator::instance().set_activated(!job_queue_.empty() && !job_queue_.front().brief);
			}
			widgets::WaitingIndicator::instance().execute(box_x, box_y, box_w, box_h);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void on_before_rendered_frame()
	{
		if(Console::instance().current_heigth()>0 && Console::instance().current_heigth()<(window_height()-1))
		{
			set_margin_top_fixed(Console::instance().current_heigth());
		}
		else
		{
			set_margin_top_fixed(0);
		}

		if(Console::instance().current_width()>0 && Console::instance().current_width()<(window_width()-1))
		{
			set_margin_right_fixed(window_width()-Console::instance().current_width());
		}
		else
		{
			set_margin_right_fixed(0);
		}

		script_execution_manager_.setup_grid_parameters();
		script_execution_manager_.setup_animation();
	}

	void on_after_rendered_frame()
	{
		if(GUIConfiguration::instance().enabled_waiting_indicator)
		{
			if(widgets::WaitingIndicator::instance().activated() && !widgets::WaitingIndicator::instance().executed())
			{
				return;
			}
		}

		dequeue_job();

		widgets::WaitingIndicator::instance().reset();

		if(script_execution_manager_.exit_requested())
		{
			close();
		}
	}

	void on_selection(const unsigned int drawing_id, const int button_code, const bool mod_ctrl_left, const bool mod_shift_left, const bool mod_ctrl_right, const bool mod_shift_right)
	{
		if(button_code==2)
		{
			std::ostringstream output_label;
			if(script_execution_manager_.generate_click_label(drawing_id, output_label))
			{
				cursor_label_.set(output_label.str());
			}
		}
		{
			std::ostringstream output_script;
			if(script_execution_manager_.generate_click_script(drawing_id, button_code, mod_ctrl_left, mod_shift_left, mod_ctrl_right, mod_shift_right, output_script))
			{
				enqueue_script(output_script.str());
			}
		}
	}

private:
	class Job
	{
	public:
		enum Type
		{
			TYPE_NATIVE,
			TYPE_JAVASCRIPT
		};

		std::string script;
		Type type;
		bool brief;

		Job(const std::string& script, const Type type) : script(script), type(type), brief(false)
		{
		}

		Job(const std::string& script, const Type type, const bool alt) : script(script), type(type), brief(alt)
		{
		}
	};

	class JobFile
	{
	public:
		enum Type
		{
			TYPE_NATIVE,
			TYPE_JAVASCRIPT,
			TYPE_DATA
		};

		std::string filename;
		Type type;

		JobFile(const std::string& filename, const Type type) : filename(filename), type(type)
		{
		}

		JobFile(const std::string& filename) : filename(filename), type(get_format_from_script_file_name(filename))
		{
		}

	private:
		static Type get_format_from_script_file_name(const std::string& filename)
		{
			static const std::multimap<Type, std::string> map_of_format_extensions=generate_map_of_script_file_format_extensions();
			for(std::multimap<Type, std::string>::const_iterator it=map_of_format_extensions.begin();it!=map_of_format_extensions.end();++it)
			{
				const Type format=it->first;
				const std::string& extension=it->second;
				const std::size_t pos=filename.rfind(extension);
				if(pos<filename.size() && (pos+extension.size())==filename.size())
				{
					return format;
				}
			}
			return TYPE_DATA;
		}

		static std::multimap<Type, std::string> generate_map_of_script_file_format_extensions()
		{
			std::multimap<Type, std::string> map_of_format_extensions;
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_NATIVE, ".vs"));
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_NATIVE, ".VS"));
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_JAVASCRIPT, ".js"));
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_JAVASCRIPT, ".JS"));
			return map_of_format_extensions;
		}
	};

	class ScriptPrefixParsing
	{
	public:
		enum Mode
		{
			MODE_NATIVE,
			MODE_NATIVE_BRIEF,
			MODE_JAVASCRIPT,
			MODE_JAVASCRIPT_BRIEF,
			MODE_FILES
		};

		struct Bundle
		{
			Mode mode;
			std::string prefix;
			std::string script;

			Bundle(const Mode mode, const std::string& prefix, const std::string& script) : mode(mode), prefix(prefix), script(script)
			{
			}
		};

		static Bundle parse(const std::string& script)
		{
			static const std::map<Mode, std::string> map_of_mode_prefixes=generate_map_of_mode_prefixes();
			for(std::map<Mode, std::string>::const_iterator it=map_of_mode_prefixes.begin();it!=map_of_mode_prefixes.end();++it)
			{
				const Mode mode=it->first;
				const std::string& prefix=it->second;
				if(script.rfind(prefix, 0)==0)
				{
					return Bundle(mode, prefix, script.substr(prefix.size()));
				}
			}
			return Bundle(MODE_NATIVE, "", script);
		}

	private:
		static std::map<Mode, std::string> generate_map_of_mode_prefixes()
		{
			std::map<Mode, std::string> map_of_mode_prefixes;
			map_of_mode_prefixes[MODE_NATIVE]="vs:";
			map_of_mode_prefixes[MODE_NATIVE_BRIEF]="vsb:";
			map_of_mode_prefixes[MODE_JAVASCRIPT]="js:";
			map_of_mode_prefixes[MODE_JAVASCRIPT_BRIEF]="jsb:";
			map_of_mode_prefixes[MODE_FILES]="files:";
			return map_of_mode_prefixes;
		}
	};

	class DuktaperOutputDirector : public duktaper::DuktapeManager::OutputDirector
	{
	public:
		DuktaperOutputDirector()
		{
		}

		static const DuktaperOutputDirector& instance()
		{
			static DuktaperOutputDirector output_director;
			return output_director;
		}

		void write_text(const std::string& str) const
		{
			Console::instance().add_output(str, 1.0f, 1.0f, 1.0f);
		}

		void write_error(const std::string& str) const
		{
			Console::instance().add_output(str, 1.0f, 0.5f, 0.5f);
		}

		void write_log(const std::string& str) const
		{
			Console::instance().add_output(str, 1.0f, 1.0f, 0.5f);
		}
	};

	Application()
	{
	}

	~Application()
	{
		if(good())
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}
	}

	bool enqueue_script(const ScriptPrefixParsing::Bundle& task)
	{
		if(task.mode==ScriptPrefixParsing::MODE_NATIVE || task.mode==ScriptPrefixParsing::MODE_NATIVE_BRIEF)
		{
			return enqueue_job(Job(task.script, Job::TYPE_NATIVE, task.mode==ScriptPrefixParsing::MODE_NATIVE_BRIEF));
		}
		else if(task.mode==ScriptPrefixParsing::MODE_JAVASCRIPT || task.mode==ScriptPrefixParsing::MODE_JAVASCRIPT_BRIEF)
		{
			return enqueue_job(Job(task.script, Job::TYPE_JAVASCRIPT, task.mode==ScriptPrefixParsing::MODE_JAVASCRIPT_BRIEF));
		}
		else if(task.mode==ScriptPrefixParsing::MODE_FILES)
		{
			std::istringstream input(task.script);
			bool status=false;
			while(input.good())
			{
				std::string filename;
				input >> filename;
				status=enqueue_file(filename) || status;
			}
			return status;
		}
		return false;
	}

	bool enqueue_job(const Job& job)
	{
		if(job.script.find_first_not_of(" \t\n")!=std::string::npos)
		{
			job_queue_.push_back(job);
			return true;
		}
		return false;
	}

	bool enqueue_job(const JobFile& job_file)
	{
		if(!job_file.filename.empty())
		{
			if(job_file.type==JobFile::TYPE_NATIVE)
			{
				return enqueue_job(Job(std::string("source '")+job_file.filename+"'", Job::TYPE_NATIVE));
			}
			else if(job_file.type==JobFile::TYPE_JAVASCRIPT)
			{
				return enqueue_job(Job(std::string("source('")+job_file.filename+"');", Job::TYPE_JAVASCRIPT));
			}
			else
			{
				return enqueue_job(Job(std::string("import --include-heteroatoms --file '")+job_file.filename+"'", Job::TYPE_NATIVE));
			}
		}
		return false;
	}

	bool dequeue_job()
	{
		if(!job_queue_.empty())
		{
			const Job job=job_queue_.front();
			job_queue_.pop_front();
			if(job.type==Job::TYPE_NATIVE)
			{
				script_execution_manager_.execute_script(job.script, false);
			}
			else if(job.type==Job::TYPE_JAVASCRIPT)
			{
				duktaper::DuktapeManager::eval(job.script);
			}
		}
		return (!job_queue_.empty());
	}

	ScriptExecutionManager script_execution_manager_;
	std::list<Job> job_queue_;
	widgets::CursorLabel cursor_label_;
};

}

}

#endif /* VIEWER_APPLICATION_H_ */
