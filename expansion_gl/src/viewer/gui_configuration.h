#ifndef VIEWER_GUI_CONFIGURATION_H_
#define VIEWER_GUI_CONFIGURATION_H_

#include <vector>

namespace voronota
{

namespace viewer
{

class GUIConfiguration
{
public:
	enum GridVariant
	{
		GRID_VARIANT_BY_OBJECT,
		GRID_VARIANT_BY_CONCEPT
	};

	enum OcclusionVariant
	{
		OCCLUSION_VARIANT_NONE,
		OCCLUSION_VARIANT_NOISY,
		OCCLUSION_VARIANT_SMOOTH
	};

	enum AntialiasingVariant
	{
		ANTIALIASING_VARIANT_NONE,
		ANTIALIASING_VARIANT_FAST
	};

	enum MultisamplingVariant
	{
		MULTISAMPLING_VARIANT_NONE,
		MULTISAMPLING_VARIANT_BASIC
	};

	enum ImpostoringVariant
	{
		IMPOSTORING_VARIANT_NONE,
		IMPOSTORING_VARIANT_SIMPLE
	};

	enum AnimationVariant
	{
		ANIMATION_VARIANT_NONE,
		ANIMATION_VARIANT_LOOP_PICKED_OBJECTS,
		ANIMATION_VARIANT_SPIN_LEFT,
		ANIMATION_VARIANT_SPIN_RIGHT
	};

	enum InitialRepresentationVariant
	{
		INITIAL_REPRESENTATION_VARIANT_CARTOON,
		INITIAL_REPRESENTATION_VARIANT_TRACE
	};

	GridVariant grid_variant;
	OcclusionVariant occlusion_variant;
	AntialiasingVariant antialiasing_variant;
	MultisamplingVariant multisampling_variant;
	ImpostoringVariant impostoring_variant;
	AnimationVariant animation_variant;
	InitialRepresentationVariant initial_main_representation_variant;
	bool enabled_cursor_label;
	bool enabled_waiting_indicator;
	int json_writing_level;
	double animation_step_miliseconds;
	bool enabled_sequence_view;
	bool enabled_console;

	static GUIConfiguration& instance()
	{
		static GUIConfiguration rp;
		return rp;
	}

	static void push()
	{
		stack().push_back(instance());
	}

	static bool pop()
	{
		if(!stack().empty())
		{
			instance()=stack().back();
			stack().pop_back();
			return true;
		}
		return false;
	}

	bool enabled_widgets() const
	{
		return (enabled_waiting_indicator);
	}

	void set_enabled_widgets(const bool status)
	{
		enabled_waiting_indicator=status;
	}

private:
	GUIConfiguration() :
		grid_variant(GRID_VARIANT_BY_OBJECT),
		occlusion_variant(OCCLUSION_VARIANT_NONE),
		antialiasing_variant(ANTIALIASING_VARIANT_NONE),
		multisampling_variant(MULTISAMPLING_VARIANT_NONE),
		impostoring_variant(IMPOSTORING_VARIANT_NONE),
		animation_variant(ANIMATION_VARIANT_NONE),
		initial_main_representation_variant(INITIAL_REPRESENTATION_VARIANT_CARTOON),
		enabled_cursor_label(true),
		enabled_waiting_indicator(true),
		json_writing_level(1),
		animation_step_miliseconds(1000.0/30.0),
		enabled_sequence_view(false),
		enabled_console(true)
	{
	}

	static std::vector<GUIConfiguration>& stack()
	{
		static std::vector<GUIConfiguration> obj;
		return obj;
	}
};

}

}

#endif /* VIEWER_GUI_CONFIGURATION_H_ */
