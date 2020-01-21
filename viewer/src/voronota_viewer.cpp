#include <memory>

#include "viewer/application.h"

int main(const int argc, const char** argv)
{
	voronota::viewer::Application::InitializationParameters app_parameters;
	app_parameters.suggested_window_width=800;
	app_parameters.suggested_window_height=600;
	app_parameters.title="voronota-viewer";
	app_parameters.shader_vertex="_shader_vertex_simple";
	app_parameters.shader_vertex_with_instancing="_shader_vertex_with_instancing";
	app_parameters.shader_fragment="_shader_fragment_simple";

	std::string raw_arguments;

	{
		int i=1;
		while(i<argc)
		{
			const std::string argv_i=argv[i];
			if(argv_i=="--title" && (i+1)<argc)
			{
				app_parameters.title=argv[++i];
			}
			else if(argv_i=="--shader-vertex" && (i+1)<argc)
			{
				app_parameters.shader_vertex=argv[++i];
			}
			else if(argv_i=="--shader-vertex-with-instancing" && (i+1)<argc)
			{
				app_parameters.shader_vertex_with_instancing=argv[++i];
			}
			else if(argv_i=="--shader-fragment" && (i+1)<argc)
			{
				app_parameters.shader_fragment=argv[++i];
			}
			else if(argv_i=="--input-pdb" && (i+1)<argc)
			{
				raw_arguments+="import --format pdb --include-heteroatoms --file '";
				raw_arguments+=argv[++i];
				raw_arguments+="'\n";
			}
			else if(argv_i=="--script-text" && (i+1)<argc)
			{
				raw_arguments+=argv[++i];
				raw_arguments+="\n";
			}
			else if(argv_i=="--script-file" && (i+1)<argc)
			{
				raw_arguments+="source '";
				raw_arguments+=argv[++i];
				raw_arguments+="'\n";
			}
			else if(argv_i=="--window-size" && (i+2)<argc)
			{
				{
					std::istringstream local_input((std::string(argv[++i])));
					local_input >> app_parameters.suggested_window_width;
				}
				{
					std::istringstream local_input((std::string(argv[++i])));
					local_input >> app_parameters.suggested_window_height;
				}
			}
			else
			{
				raw_arguments+=argv_i;
				raw_arguments+="\n";
			}
			i++;
		}
	}

	voronota::viewer::Application& app=voronota::viewer::Application::instance();

	if(!app.init(app_parameters))
	{
		std::cerr << "Error: failed to init application." << std::endl;
		return 1;
	}

	ImGui_ImplGlfwGL3_Init(app.window(), false);

	{
		ImGuiIO& io=ImGui::GetIO();
		io.IniFilename=0;
	}

	app.enqueue_script(raw_arguments);

#ifdef FOR_WEB
	emscripten_set_main_loop_arg(voronota::viewer::Application::render, &app, 0, 1);
#else
	app.run_loop();
#endif

	ImGui_ImplGlfwGL3_Shutdown();

	return 0;
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

