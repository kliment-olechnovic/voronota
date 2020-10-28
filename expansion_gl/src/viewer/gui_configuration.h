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

	enum AnimationVariant
	{
		ANIMATION_VARIANT_NONE,
		ANIMATION_VARIANT_LOOP_PICKED_OBJECTS,
		ANIMATION_VARIANT_SPIN_LEFT,
		ANIMATION_VARIANT_SPIN_RIGHT
	};


	GridVariant grid_variant;
	AnimationVariant animation_variant;
	bool enabled_cursor_label;
	bool enabled_waiting_indicator;
	int json_writing_level;
	double animation_step_miliseconds;

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
		animation_variant(ANIMATION_VARIANT_NONE),
		enabled_cursor_label(true),
		enabled_waiting_indicator(true),
		json_writing_level(1),
		animation_step_miliseconds(1000.0/30.0)
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
