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

	GridVariant grid_variant;

	static RuntimeParameters& instance()
	{
		static RuntimeParameters rp;
		return rp;
	}

private:
	RuntimeParameters() : grid_variant(GRID_VARIANT_BY_OBJECT)
	{
	}

	~RuntimeParameters()
	{
	}
};

}

}

#endif /* VIEWER_RUNTIME_PARAMETERS_H_ */
