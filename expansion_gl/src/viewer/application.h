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
		static std::string rejection_message("rejected");

		if(need_to_wait_for_asynchronous_downloads_to_finish())
		{
			return rejection_message;
		}

		while(dequeue_job())
		{
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

	void upload_session(const std::string& data)
	{
		std::string virtual_file_name=std::string("_virtual/session.vses");
		scripting::VirtualFileStorage::set_file(virtual_file_name, data);
		enqueue_script(std::string("import-session --file ")+virtual_file_name+" ; delete-virtual-files "+virtual_file_name);
	}

#ifdef FOR_WEB
	void setup_js_bindings_to_all_api_functions()
	{
		std::string script;
		script+="raw_voronota=voronota_viewer_execute_native_script;\n";
		script+="raw_voronota_last_output=voronota_viewer_get_last_script_output;\n";
		script+=duktaper::BindingJavascript::generate_setup_script(script_execution_manager_.collection_of_command_documentations(), true);
		EnscriptenUtilities::execute_javascript(script);
	}
#endif

protected:
	void on_after_init_success()
	{
		if(good())
		{
			set_background_color(0x000000);
			set_margin_color(0x7F7F7F);
			set_margin_top_fixed(200);

			ImGui::CreateContext();

			ImGui_ImplGlfw_InitForOpenGL(window(), false);
			ImGui_ImplOpenGL3_Init();

			{
				ImGuiIO& io=ImGui::GetIO();
				io.IniFilename=0;
			}

			{
				ImGuiStyle& style=ImGui::GetStyle();
				style=GUIStyleWrapper::default_style();
			}

			duktaper::DuktapeManager::set_output_director(DuktaperOutputDirector::instance());
			duktaper::DuktapeManager::set_script_execution_manager(script_execution_manager_);
			duktaper::DuktapeManager::flag_to_print_result_on_eval()=false;
			duktaper::DuktapeManager::eval(duktaper::BindingJavascript::generate_setup_script(script_execution_manager_.collection_of_command_documentations(), false));
			duktaper::DuktapeManager::flag_to_print_result_on_eval()=true;
		}
	}

	bool check_window_scroll_intercepted(double xoffset, double yoffset)
	{
		ImGui_ImplGlfw_ScrollCallback(window(), xoffset, yoffset);
		return (ImGui::GetIO().WantCaptureMouse || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
	}

	bool check_mouse_button_use_intercepted(int button, int action, int mods)
	{
		ImGui_ImplGlfw_MouseButtonCallback(window(), button, action, mods);
		return (ImGui::GetIO().WantCaptureMouse || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
	}

	bool check_mouse_cursor_move_intercepted(double xpos, double ypos)
	{
		ImGui_ImplGlfw_CursorPosCallback(window(), xpos, ypos);
		return (ImGui::GetIO().WantCaptureMouse || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
	}

	void on_mouse_cursor_moved(double xpos, double ypos)
	{
		cursor_label_.clear();
	}

	void on_key_used(int key, int scancode, int action, int mods)
	{
		if(action==GLFW_PRESS)
		{
			if(key==GLFW_KEY_ENTER || key==GLFW_KEY_UP || key==GLFW_KEY_DOWN)
			{
				if(hovered())
				{
					console::Console::instance().text_interface_info().set_need_keyboard_focus_in_command_input(true);
				}
			}
			else if(key==GLFW_KEY_ESCAPE && mods==0)
			{
				GUIConfiguration::instance().enabled_console=!GUIConfiguration::instance().enabled_console;
			}
			else if(key==GLFW_KEY_ESCAPE && mods==GLFW_MOD_SHIFT)
			{
				console::Console::instance().shrink_to_minimal_view();
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
			const std::string console_result=console::Console::instance().execute(0, 0, window_width()/5*3, 200, window_width()/4, window_width(), 40, window_height());
			if(!console_result.empty())
			{
				const ScriptPrefixParsing::Bundle task=ScriptPrefixParsing::parse(console_result);
				enqueue_script(task);
				if(!task.prefix.empty() && task.mode!=ScriptPrefixParsing::MODE_NATIVE_BRIEF)
				{
					console::Console::instance().text_interface_info().set_next_prefix(task.prefix+" ");
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
				widgets::WaitingIndicator::instance().set_activated(pending_jobs_in_queue(false));
			}
			widgets::WaitingIndicator::instance().execute(box_x, box_y, box_w, box_h);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void on_before_rendered_frame()
	{
		if(console::Console::instance().current_heigth()>0 && console::Console::instance().current_heigth()<(window_height()-1))
		{
			set_margin_top_fixed(console::Console::instance().current_heigth());
		}
		else
		{
			set_margin_top_fixed(0);
		}

		if(console::Console::instance().current_width()>0 && console::Console::instance().current_width()<(window_width()-1))
		{
			set_margin_right_fixed(window_width()-console::Console::instance().current_width());
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
			TYPE_OBJECTS,
			TYPE_VIEW,
			TYPE_SESSION,
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
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_OBJECTS, ".vo"));
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_OBJECTS, ".VO"));
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_VIEW, ".vview"));
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_VIEW, ".VVIEW"));
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_SESSION, ".vses"));
			map_of_format_extensions.insert(std::pair<Type, std::string>(TYPE_SESSION, ".VSES"));
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
			return add_briefness_if_needed(parse_with_prefix(script));
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

		static Bundle parse_with_prefix(const std::string& script)
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

		static Bundle add_briefness_if_needed(const Bundle& bundle)
		{
			if(bundle.mode==MODE_NATIVE && bundle.script.find("screenshot")<bundle.script.size())
			{
				return Bundle(MODE_NATIVE_BRIEF, bundle.prefix, bundle.script);
			}
			else if(bundle.mode==MODE_JAVASCRIPT && bundle.script.find("screenshot")<bundle.script.size())
			{
				return Bundle(MODE_JAVASCRIPT_BRIEF, bundle.prefix, bundle.script);
			}
			return bundle;
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
			console::Console::instance().text_interface_info().add_output(str, 1.0f, 1.0f, 1.0f);
		}

		void write_error(const std::string& str) const
		{
			console::Console::instance().text_interface_info().add_output(str, 1.0f, 0.5f, 0.5f);
		}

		void write_log(const std::string& str) const
		{
			console::Console::instance().text_interface_info().add_output(str, 1.0f, 1.0f, 0.5f);
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

	bool enqueue_job(const Job& job, const bool in_front)
	{
		if(job.script.find_first_not_of(" \t\n")!=std::string::npos)
		{
			if(in_front)
			{
				job_queue_.push_front(job);
			}
			else
			{
				job_queue_.push_back(job);
			}
			return true;
		}
		return false;
	}

	bool enqueue_job(const Job& job)
	{
		return enqueue_job(job, false);
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
			else if(job_file.type==JobFile::TYPE_OBJECTS)
			{
				return enqueue_job(Job(std::string("import-objects --file '")+job_file.filename+"'", Job::TYPE_NATIVE));
			}
			else if(job_file.type==JobFile::TYPE_VIEW)
			{
				return enqueue_job(Job(std::string("import-view --file '")+job_file.filename+"'", Job::TYPE_NATIVE));
			}
			else if(job_file.type==JobFile::TYPE_SESSION)
			{
				return enqueue_job(Job(std::string("import-session --file '")+job_file.filename+"'", Job::TYPE_NATIVE));
			}
			else
			{
				return enqueue_job(Job(std::string("import --include-heteroatoms --file '")+job_file.filename+"'", Job::TYPE_NATIVE));
			}
		}
		return false;
	}

	bool pending_jobs_in_queue(const bool including_brief_jobs) const
	{
		return ((!job_queue_.empty() && (including_brief_jobs || !job_queue_.front().brief)) || RemoteImportDownloaderAdaptive::instance().check_if_any_request_downloaded_and_not_fully_processed());
	}

	bool need_to_wait_for_asynchronous_downloads_to_finish() const
	{
		return (!RemoteImportDownloaderAdaptive::instance().is_synchronous() && RemoteImportDownloaderAdaptive::instance().check_if_any_request_not_downloaded());
	}

	bool dequeue_job()
	{
		if(need_to_wait_for_asynchronous_downloads_to_finish())
		{
			return (!job_queue_.empty());
		}

		if(RemoteImportDownloaderAdaptive::instance().check_if_any_request_downloaded_and_not_fully_processed())
		{
			enqueue_job(Job("import-downloaded", Job::TYPE_NATIVE), true);
		}

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
