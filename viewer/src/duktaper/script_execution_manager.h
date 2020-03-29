#ifndef DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_
#define DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"

#include "operators_all.h"

namespace voronota
{

namespace duktaper
{

class ScriptExecutionManager : public scripting::ScriptExecutionManagerWithVariantOutput
{
public:
	ScriptExecutionManager()
	{
		set_command_for_extra_actions("nnport-predict", operators::NNPortPredict());
	}
};

}

}

#endif /* DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_ */
