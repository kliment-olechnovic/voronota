#ifndef IMGUI_ADDONS_SIMPLE_SPLITTER_H_
#define IMGUI_ADDONS_SIMPLE_SPLITTER_H_

class ImGuiAddonSimpleSplitter
{
public:
	static void set_splitter(const char* id_str, float* inside_size, float* outside_size, const float min_inside_size, const float min_outside_size, const float gui_scale_factor);
};

#endif /* IMGUI_ADDONS_SIMPLE_SPLITTER_H_ */
