#include "simple_splitter.h"
#include "../imgui_internal.h"

void ImGuiAddonSimpleSplitter::set_splitter(const char* id_str, float* inside_size, float* outside_size, const float min_inside_size, const float min_outside_size, const float gui_scale_factor)
{
	ImGuiWindow* window=ImGui::GetCurrentContext()->CurrentWindow;
	ImGuiID id=window->GetID(id_str);
	ImRect region_rect;
	region_rect.Min=window->DC.CursorPos;
	region_rect.Min.y+=(*inside_size);
	region_rect.Max=region_rect.Min;
	ImVec2 gui_size=ImGui::CalcItemSize(ImVec2(-1.0f, 4.0f*gui_scale_factor), 0.0f, 0.0f);
	region_rect.Max.x+=gui_size.x;
	region_rect.Max.y+=gui_size.y;
	ImGui::SplitterBehavior(region_rect, id, ImGuiAxis_Y, inside_size, outside_size, min_inside_size, min_outside_size, 1.0f);
}
