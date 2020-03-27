#include <memory>

#include "viewer/application.h"
#include "duktaper/stocked_data_resources.h"

namespace
{

inline std::string generate_startup_script()
{
	voronota::scripting::VirtualFileStorage::set_file("_virtual/radii", voronota::duktaper::resources::data_radii());
	voronota::scripting::VirtualFileStorage::set_file("_virtual/voromqa_v1_energy_means_and_sds", voronota::duktaper::resources::data_voromqa_v1_energy_means_and_sds());
	voronota::scripting::VirtualFileStorage::set_file("_virtual/voromqa_v1_energy_potential", voronota::duktaper::resources::data_voromqa_v1_energy_potential());

	std::ostringstream output;
	output << "setup-loading --radii-file _virtual/radii" << "\n";
	output << "setup-voromqa --potential _virtual/voromqa_v1_energy_potential --means-and-sds _virtual/voromqa_v1_energy_means_and_sds" << "\n";
	output << "delete-virtual-files" << "\n";

	const std::string result_filename="_virtual/startup.vs";
	voronota::scripting::VirtualFileStorage::set_file(result_filename, output.str());

	return result_filename;
}

}

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
		const std::vector<std::string> files=command_args_input.get_value_vector_or_all_unnamed_values("files");
		const std::vector<std::string> scripts=command_args_input.get_value_vector_or_default<std::string>("scripts", std::vector<std::string>());

		command_args_input.assert_nothing_unusable();

		if(!voronota::viewer::Application::instance().init(app_init_parameters))
		{
			throw std::runtime_error(std::string("Failed to init application."));
		}

		voronota::viewer::Application::instance().enqueue_file(generate_startup_script());

		for(std::size_t i=0;i<files.size();i++)
		{
			voronota::viewer::Application::instance().enqueue_file(files[i]);
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

EMSCRIPTEN_KEEPALIVE void voronota_viewer_upload_file(const char* name, const char* data)
{
	voronota::viewer::Application::instance().upload_file(name, data);
}

}

#endif

