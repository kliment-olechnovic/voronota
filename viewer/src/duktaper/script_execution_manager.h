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
		set_command_for_extra_actions("setup-defaults", operators::SetupDefaults());
		set_command_for_extra_actions("nnport-predict", operators::NNPortPredict());

		set_command_for_data_manager("voromqa-dark-global", operators::VoroMQADarkGlobal(), true);

		set_command_for_congregation_of_data_managers("tmalign", operators::TMalign());
		set_command_for_congregation_of_data_managers("tmalign-all", operators::TMalignAll());
	}
};

}

}

#endif /* DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_ */