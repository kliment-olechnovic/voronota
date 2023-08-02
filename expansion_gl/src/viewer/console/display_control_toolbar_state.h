#ifndef VIEWER_CONSOLE_DISPLAY_CONTROL_TOOLBAR_STATE_H_
#define VIEWER_CONSOLE_DISPLAY_CONTROL_TOOLBAR_STATE_H_

#include "common_for_gui.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class DisplayControlToolbarState
{
public:
	bool visible;

	DisplayControlToolbarState() :
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
			const std::string button_id=std::string("Set background##background_color_button");
			const std::string menu_id=std::string("Background##background_color_button_menu");
			ImGui::Button(button_id.c_str(), ImVec2(130*GUIStyleWrapper::scale_factor(),0));
			if(ImGui::BeginPopupContextItem(menu_id.c_str(), 0))
			{
				if(ImGui::Selectable("white"))
				{
					result="background white";
				}
				if(ImGui::Selectable("black"))
				{
					result="background black";
				}
				if(ImGui::Selectable("gray"))
				{
					result="background 0xCCCCCC";
				}
				ImGui::EndPopup();
			}
		}
	}
};

}

}

}

#endif /* VIEWER_CONSOLE_DISPLAY_CONTROL_TOOLBAR_STATE_H_ */
