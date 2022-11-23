#include <memory>

#include "viewer/application.h"
#include "viewer/emscripten_utilities.h"

int main(const int argc, const char** argv)
{
	int return_status=1;

	try
	{
		voronota::scripting::CommandInput command_args_input(argc, argv);

		voronota::uv::ViewerApplication::InitializationParameters app_init_parameters;
		app_init_parameters.suggested_window_width=command_args_input.get_value_or_default<int>("window-width", 1024);
		app_init_parameters.suggested_window_height=command_args_input.get_value_or_default<int>("window-height", 768);
		app_init_parameters.no_fps_limit=command_args_input.get_flag("no-fps-limit");
		app_init_parameters.title=command_args_input.get_value_or_default<std::string>("title", "Voronota-GL");
		app_init_parameters.shader_vertex_screen=command_args_input.get_value_or_default<std::string>("shader-vertex-screen", "_shader_vertex_screen");
		app_init_parameters.shader_vertex=command_args_input.get_value_or_default<std::string>("shader-vertex", "_shader_vertex_simple");
		app_init_parameters.shader_vertex_with_instancing=command_args_input.get_value_or_default<std::string>("shader-vertex-with-instancing", "_shader_vertex_with_instancing");
		app_init_parameters.shader_vertex_with_impostoring=command_args_input.get_value_or_default<std::string>("shader-vertex-with-impostoring", "_shader_vertex_with_impostoring");
		app_init_parameters.shader_fragment_screen=command_args_input.get_value_or_default<std::string>("shader-fragment-screen", "_shader_fragment_screen");
		app_init_parameters.shader_fragment=command_args_input.get_value_or_default<std::string>("shader-fragment", "_shader_fragment_simple");
		app_init_parameters.shader_fragment_with_instancing=command_args_input.get_value_or_default<std::string>("shader-fragment-with-instancing", "_shader_fragment_simple");
		app_init_parameters.shader_fragment_with_impostoring=command_args_input.get_value_or_default<std::string>("shader-fragment-with-impostoring", "_shader_fragment_with_impostoring");
		const float gui_scaling=command_args_input.get_value_or_default<float>("gui-scaling", 1.0f);
		const std::string custom_font_file=command_args_input.get_value_or_default<std::string>("custom-font-file", "");
		const std::vector<std::string> files=command_args_input.get_value_vector_or_all_unused_unnamed_values("files");
		const std::vector<std::string> scripts=command_args_input.get_value_vector_or_default<std::string>("scripts", std::vector<std::string>());
		const bool musical=command_args_input.get_flag("musical");

		command_args_input.assert_nothing_unusable();

		if(!voronota::viewer::Application::instance().init(app_init_parameters))
		{
			throw std::runtime_error(std::string("Failed to init application."));
		}

		if(!custom_font_file.empty())
		{
			ImGuiIO& io=ImGui::GetIO();
			io.Fonts->AddFontFromFileTTF(custom_font_file.c_str(), 13.0f*gui_scaling);
		}

		voronota::viewer::GUIStyleWrapper::initialized()=true;
		voronota::viewer::GUIStyleWrapper::set_scale_factor(gui_scaling, custom_font_file.empty());

		voronota::viewer::Application::instance().enqueue_script("clear");
		voronota::viewer::Application::instance().enqueue_script("setup-defaults");
		voronota::viewer::Application::instance().enqueue_script("perspective");
		voronota::viewer::Application::instance().enqueue_script("impostoring-none");
		voronota::viewer::Application::instance().enqueue_script("antialiasing-none");
		voronota::viewer::Application::instance().enqueue_script("occlusion-none");
		voronota::viewer::Application::instance().enqueue_script("multisampling-basic");
		voronota::viewer::Application::instance().enqueue_script("background 0xCCCCCC");

		const bool faster_loading=(files.size()>5);
		const bool show_cartoons_after_faster_loading=(faster_loading && files.size()<=30);
		const bool with_music_background_for_loading=(musical && files.size()>10);

		if(musical)
		{
			voronota::viewer::Application::instance().enqueue_script("music-background enable");
		}

		if(with_music_background_for_loading)
		{
			voronota::viewer::Application::instance().enqueue_script("music-background waiting");
		}

		if(faster_loading)
		{
			voronota::viewer::Application::instance().enqueue_script("set-initial-atom-representation-to-trace");
		}

		for(std::size_t i=0;i<files.size();i++)
		{
			voronota::viewer::Application::instance().enqueue_file(files[i]);
		}

		if(!files.empty())
		{
			voronota::viewer::Application::instance().enqueue_script("pick-objects");
		}

		if(show_cartoons_after_faster_loading)
		{
			voronota::viewer::Application::instance().enqueue_script("show-atoms -rep cartoon");
			voronota::viewer::Application::instance().enqueue_script("hide-atoms -rep trace");
		}

		if(faster_loading)
		{
			voronota::viewer::Application::instance().enqueue_script("set-initial-atom-representation-to-cartoon");
		}

		for(std::size_t i=0;i<scripts.size();i++)
		{
			voronota::viewer::Application::instance().enqueue_script(scripts[i]);
		}

		if(with_music_background_for_loading)
		{
			voronota::viewer::Application::instance().enqueue_script("music-background stop");
		}

#ifdef FOR_WEB
		emscripten_set_main_loop(voronota::viewer::Application::instance_render_frame, 0, 1);
#else
		voronota::viewer::Application::instance().run_loop();
#endif

		return_status=0;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown exception caught." << std::endl;
	}

	try
	{
		voronota::duktaper::operators::MusicBackground::stop_if_was_used();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught when finalizing: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown exception caught when finalizing." << std::endl;
	}

	return return_status;
}

#ifdef FOR_WEB

extern "C"
{

EMSCRIPTEN_KEEPALIVE void voronota_viewer_enqueue_script(const char* command)
{
	voronota::viewer::Application::instance().enqueue_script(command);
}

EMSCRIPTEN_KEEPALIVE const char* voronota_viewer_execute_native_script(const char* command)
{
	return voronota::viewer::Application::instance().execute_native_script(command).c_str();
}

EMSCRIPTEN_KEEPALIVE const char* voronota_viewer_get_last_script_output()
{
	return voronota::viewer::Application::instance().get_last_script_output().c_str();
}

EMSCRIPTEN_KEEPALIVE void voronota_viewer_upload_file(const char* name, const char* data, const char* parameters)
{
	voronota::viewer::Application::instance().upload_file(name, data, parameters);
}

EMSCRIPTEN_KEEPALIVE void voronota_viewer_resize_window(const int width, const int height)
{
	voronota::viewer::Application::instance().set_window_size(width, height);
}

}

#endif

