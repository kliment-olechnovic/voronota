#ifndef VIEWER_CONSOLE_DISPLAY_CONTROL_TOOLBAR_STATE_H_
#define VIEWER_CONSOLE_DISPLAY_CONTROL_TOOLBAR_STATE_H_

#include "common_for_gui.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class DisplayControlToolbarPanel
{
public:
	bool visible;

	DisplayControlToolbarPanel() :
		visible(true)
	{
	}

	void execute(std::string& result)
	{
		{
			static bool sequence=false;

			sequence=GUIConfiguration::instance().enabled_sequence_view;

			if(ImGui::Checkbox("Sequence", &sequence))
			{
				if(sequence)
				{
					result="vsb: configure-gui-enable-sequence-view";
				}
				else
				{
					result="vsb: configure-gui-disable-sequence-view";
				}
			}
		}
		ImGui::SameLine();
		{
			static bool grid=false;

			grid=uv::ViewerApplication::instance().rendering_mode_is_grid();

			if(ImGui::Checkbox("Grid", &grid))
			{
				if(grid)
				{
					result="vsb: grid-by-object";
				}
				else
				{
					result="vsb: mono";
				}
			}
		}
		ImGui::SameLine();
		{
			static bool stereo=false;

			stereo=uv::ViewerApplication::instance().rendering_mode_is_stereo();

			if(ImGui::Checkbox("Stereo", &stereo))
			{
				if(stereo)
				{
					result="vsb: stereo";
				}
				else
				{
					result="vsb: mono";
				}
			}
		}

		{
			static bool perspective=false;

			perspective=uv::ViewerApplication::instance().projection_mode_is_perspective();

			if(ImGui::Checkbox("Perspective", &perspective))
			{
				if(perspective)
				{
					result="vsb: perspective";
				}
				else
				{
					result="vsb: ortho";
				}
			}
		}
		ImGui::SameLine();
		{
			static bool compact_printing=false;

			compact_printing=(GUIConfiguration::instance().json_writing_level<6);

			if(ImGui::Checkbox("Compact printing", &compact_printing))
			{
				if(compact_printing)
				{
					result="vsb: configure-gui-json-write-level-0";
				}
				else
				{
					result="vsb: configure-gui-json-write-level-6";
				}
			}
		}

		{
			ImGui::TextUnformatted("Background:");

			ImGui::SameLine();

			if(ImGui::Button("white##display_control_background"))
			{
				result="background white";
			}

			ImGui::SameLine();

			if(ImGui::Button("gray##display_control_background"))
			{
				result="background 0xCCCCCC";
			}

			ImGui::SameLine();

			if(ImGui::Button("black##display_control_background"))
			{
				result="background black";
			}
		}
	}
};

}

}

}

#endif /* VIEWER_CONSOLE_DISPLAY_CONTROL_TOOLBAR_STATE_H_ */
