#ifndef VIEWER_CONSOLE_SHADING_CONTROL_TOOLBAR_STATE_H_
#define VIEWER_CONSOLE_SHADING_CONTROL_TOOLBAR_STATE_H_

#include "common_for_gui.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class ShadingControlToolbarPanel
{
public:
	bool visible;

	ShadingControlToolbarPanel() :
		visible(true)
	{
	}

	void execute(std::string& result)
	{
		{
			static bool antialiasing=false;

			antialiasing=uv::ViewerApplication::instance().antialiasing_mode_is_fast();

			if(ImGui::Checkbox("Antialiasing", &antialiasing))
			{
				if(antialiasing)
				{
					result="vsb: antialiasing-fast ; multisampling-none";
				}
				else
				{
					result="vsb: antialiasing-none";
				}
			}
		}
		ImGui::SameLine();
		{
			static bool multisampling=false;

			multisampling=uv::ViewerApplication::instance().multisampling_mode_is_basic();

			if(ImGui::Checkbox("Multisampling", &multisampling))
			{
				if(multisampling)
				{
					result="vsb: multisampling-basic ; antialiasing-none";
				}
				else
				{
					result="vsb: multisampling-none";
				}
			}
		}

		{
			static bool occlusion_smooth=false;

			occlusion_smooth=uv::ViewerApplication::instance().occlusion_mode_is_smooth();

			if(ImGui::Checkbox("Occlusion (smooth)", &occlusion_smooth))
			{
				if(occlusion_smooth)
				{
					result="vsb: occlusion-smooth";
				}
				else
				{
					result="vsb: occlusion-none";
				}
			}
		}
		ImGui::SameLine();
		{
			static bool occlusion_noisy=false;

			occlusion_noisy=uv::ViewerApplication::instance().occlusion_mode_is_noisy();

			if(ImGui::Checkbox("Occlusion (noisy)", &occlusion_noisy))
			{
				if(occlusion_noisy)
				{
					result="vsb: occlusion-noisy";
				}
				else
				{
					result="vsb: occlusion-none";
				}
			}
		}

		{
			static bool impostoring=false;

			impostoring=(GUIConfiguration::instance().impostoring_variant==GUIConfiguration::IMPOSTORING_VARIANT_SIMPLE);

			if(ImGui::Checkbox("Impostors", &impostoring))
			{
				if(impostoring)
				{
					result="vsb: impostoring-simple";
				}
				else
				{
					result="vsb: impostoring-none";
				}
			}
		}
		ImGui::SameLine();
		{
			static bool show_fps=false;

			ImGui::Checkbox(show_fps ? "Show stats:" : "Show stats", &show_fps);

			if(show_fps)
			{
				ImGui::SameLine();
				ImGui::Text("FPS = %.1f", static_cast<double>(ImGui::GetIO().Framerate));
			}
		}
	}
};

}

}

}

#endif /* VIEWER_CONSOLE_SHADING_CONTROL_TOOLBAR_STATE_H_ */
