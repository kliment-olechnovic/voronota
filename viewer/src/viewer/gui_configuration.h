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

	GridVariant grid_variant;
	bool enabled_menu;
	bool enabled_cursor_label;
	bool enabled_console;
	bool enabled_info_box;
	bool enabled_waiting_indicator;

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
		return (enabled_menu || enabled_console || enabled_waiting_indicator);
	}

	void set_enabled_widgets(const bool status)
	{
		enabled_waiting_indicator=status;
		enabled_console=status;
		enabled_menu=status;
	}

private:
	GUIConfiguration() :
		grid_variant(GRID_VARIANT_BY_OBJECT),
		enabled_menu(false),
		enabled_cursor_label(true),
		enabled_console(false),
		enabled_info_box(true),
		enabled_waiting_indicator(true)
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
