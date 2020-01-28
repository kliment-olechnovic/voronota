#ifndef VIEWER_RUNTIME_PARAMETERS_H_
#define VIEWER_RUNTIME_PARAMETERS_H_

namespace voronota
{

namespace viewer
{

class RuntimeParameters
{
public:
	enum GridVariant
	{
		GRID_VARIANT_BY_OBJECT,
		GRID_VARIANT_BY_CONCEPT
	};

	enum ConsoleModeVariant
	{
		CONSOLE_MODE_VARIANT_NATIVE,
		CONSOLE_MODE_VARIANT_JAVASCRIPT
	};


	GridVariant grid_variant;
	ConsoleModeVariant console_mode_variant;
	bool enabled_menu;
	bool enabled_console;
	bool enabled_info_box;

	static RuntimeParameters& instance()
	{
		static RuntimeParameters rp;
		return rp;
	}

private:
	RuntimeParameters() :
		grid_variant(GRID_VARIANT_BY_OBJECT),
		console_mode_variant(CONSOLE_MODE_VARIANT_NATIVE),
		enabled_menu(false),
		enabled_console(false),
		enabled_info_box(true)
	{
	}

	~RuntimeParameters()
	{
	}
};

}

}

#endif /* VIEWER_RUNTIME_PARAMETERS_H_ */
