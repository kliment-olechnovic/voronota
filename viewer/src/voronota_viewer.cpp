#include <memory>

#include "viewer/application.h"

int main(const int argc, const char** argv)
{
	try
	{
		voronota::scripting::CommandInput input(argc, argv);

		voronota::viewer::Application::InitializationParameters app_parameters;
		app_parameters.suggested_window_width=input.get_value_or_default<int>("window-width", 800);
		app_parameters.suggested_window_height=input.get_value_or_default<int>("window-height", 600);
		app_parameters.title=input.get_value_or_default<std::string>("title", "voronota-viewer");
		app_parameters.shader_vertex=input.get_value_or_default<std::string>("shader-vertex", "_shader_vertex_simple");
		app_parameters.shader_vertex_with_instancing=input.get_value_or_default<std::string>("shader-vertex-with-instancing", "_shader_vertex_with_instancing");
		app_parameters.shader_fragment=input.get_value_or_default<std::string>("shader-fragment", "_shader_fragment_simple");
		const std::string input_pdb=input.get_value_or_default<std::string>("input-pdb", "");
		const std::string script_file=input.get_value_or_default<std::string>("script-file", "");

		input.assert_nothing_unusable();

		std::ostringstream starting_script;

		if(!input_pdb.empty())
		{
			starting_script << "import --format pdb --include-heteroatoms --file '" << input_pdb << "'\n";
		}

		if(!script_file.empty())
		{
			starting_script << "source '" << script_file << "'\n";
		}

		if(!voronota::viewer::Application::instance().init(app_parameters))
		{
			throw std::runtime_error(std::string("Failed to init application."));
		}

		voronota::viewer::Application::instance().enqueue_script(starting_script.str());

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

EMSCRIPTEN_KEEPALIVE void application_enqueue_script(const char* command)
{
	voronota::viewer::Application::instance().enqueue_script(command);
}

EMSCRIPTEN_KEEPALIVE const char* application_execute_script(const char* command)
{
	return voronota::viewer::Application::instance().execute_script(command).c_str();
}

EMSCRIPTEN_KEEPALIVE void application_upload_file(const char* name, const char* data)
{
	voronota::viewer::Application::instance().upload_file(name, data);
}

}

#endif

