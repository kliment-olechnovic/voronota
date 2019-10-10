#include <memory>

#include "viewer/application.h"

viewer::Application* viewer::Application::self_ptr=0;

int main(const int argc, const char** argv)
{
	viewer::Application::InitializationParameters app_parameters;
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
				raw_arguments+="load --format pdb --include-heteroatoms --file '";
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

	viewer::Application app;

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

	{
		ImGuiStyle& style=ImGui::GetStyle();

		for(int i=0;i<ImGuiCol_COUNT;i++)
		{
			ImVec4& col=style.Colors[i];
			float H=0.0f;
			float S=0.0f;
			float V=0.0f;
			ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);
			V=1.0-V;
			ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
		}

		style.WindowRounding=0.0f;
		style.FrameRounding=0.0f;
		style.GrabRounding=0.0f;
		style.ScrollbarRounding=0.0f;
	}

	app.set_console_enabled(false);

	app.add_command(raw_arguments.c_str());

#ifdef FOR_WEB
	emscripten_set_main_loop_arg(viewer::Application::render, &app, 0, 1);
#else
	app.run_loop();
#endif

	ImGui_ImplGlfwGL3_Shutdown();

	return 0;
}

#ifdef FOR_WEB

extern "C"
{

EMSCRIPTEN_KEEPALIVE void application_add_command(const char* command)
{
	viewer::Application* app=viewer::Application::self_ptr;
	app->add_command(command);
}

EMSCRIPTEN_KEEPALIVE void application_upload_file(const char* name, const char* data)
{
	viewer::Application* app=viewer::Application::self_ptr;
	app->upload_file(name, data);
}

}

#endif

