#include <memory>

#include "viewer/application.h"
#include "viewer/emscripten_utilities.h"

int main(const int argc, const char** argv)
{
	try
	{
		voronota::scripting::CommandInput command_args_input(argc, argv);

		voronota::uv::ViewerApplication::InitializationParameters app_init_parameters;
		app_init_parameters.suggested_window_width=command_args_input.get_value_or_default<int>("window-width", 800);
		app_init_parameters.suggested_window_height=command_args_input.get_value_or_default<int>("window-height", 600);
		app_init_parameters.title=command_args_input.get_value_or_default<std::string>("title", "voronota-viewer");
		app_init_parameters.shader_vertex=command_args_input.get_value_or_default<std::string>("shader-vertex", "_shader_vertex_simple");
		app_init_parameters.shader_vertex_with_instancing=command_args_input.get_value_or_default<std::string>("shader-vertex-with-instancing", "_shader_vertex_with_instancing");
		app_init_parameters.shader_fragment=command_args_input.get_value_or_default<std::string>("shader-fragment", "_shader_fragment_simple");
		const std::vector<std::string> files=command_args_input.get_value_vector_or_all_unused_unnamed_values("files");
		const std::vector<std::string> scripts=command_args_input.get_value_vector_or_default<std::string>("scripts", std::vector<std::string>());

		command_args_input.assert_nothing_unusable();

		if(!voronota::viewer::Application::instance().init(app_init_parameters))
		{
			throw std::runtime_error(std::string("Failed to init application."));
		}

		voronota::viewer::Application::instance().enqueue_script("clear");
		voronota::viewer::Application::instance().enqueue_script("setup-defaults ; clear");

		for(std::size_t i=0;i<files.size();i++)
		{
			voronota::viewer::Application::instance().enqueue_file(files[i]);
		}

		if(!files.empty())
		{
			voronota::viewer::Application::instance().enqueue_script("pick-objects");
			voronota::viewer::Application::instance().enqueue_script("clear-last");
		}

		for(std::size_t i=0;i<scripts.size();i++)
		{
			voronota::viewer::Application::instance().enqueue_script(scripts[i]);
		}

#ifdef FOR_WEB
		emscripten_set_main_loop(voronota::viewer::Application::instance_render_frame, 0, 1);
#else
		voronota::viewer::Application::instance().run_loop();
#endif
		return 0;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown exception caught." << std::endl;
	}

	return 1;
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

