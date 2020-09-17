#ifndef DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_
#define DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"

#include "operators/call_shell.h"
#include "operators/checksum.h"
#include "operators/construct_or_load_contacts.h"
#include "operators/construct_or_load_quality_scores.h"
#include "operators/fetch.h"
#include "operators/import_url.h"
#include "operators/nnport_predict.h"
#include "operators/scwrl.h"
#include "operators/setup_defaults.h"
#include "operators/tmalign_many.h"
#include "operators/tmalign.h"
#include "operators/voromqa_dark_global.h"
#include "operators/voromqa_dark_local.h"
#include "operators/voromqa_dark_split.h"

namespace voronota
{

namespace duktaper
{

class ScriptExecutionManager : public scripting::ScriptExecutionManagerWithVariantOutput
{
public:
	ScriptExecutionManager()
	{
		set_command_for_extra_actions("call-shell", operators::CallShell());
		set_command_for_extra_actions("checksum", operators::Checksum());
		set_command_for_extra_actions("nnport-predict", operators::NNPortPredict());
		set_command_for_extra_actions("setup-defaults", operators::SetupDefaults());

		set_command_for_data_manager("construct-or-load-contacts", operators::ConstructOrLoadContacts(), true);
		set_command_for_data_manager("construct-or-load-quality-scores", operators::ConstructOrLoadQualityScores(), true);
		set_command_for_data_manager("scwrl", operators::Scwrl(), true);
		set_command_for_data_manager("voromqa-dark-global", operators::VoroMQADarkGlobal(), true);
		set_command_for_data_manager("voromqa-dark-local", operators::VoroMQADarkLocal(), true);
		set_command_for_data_manager("voromqa-dark-split", operators::VoroMQADarkSplit(), true);

		set_command_for_congregation_of_data_managers("fetch", operators::Fetch());
		set_command_for_congregation_of_data_managers("import-url", operators::ImportUrl());
		set_command_for_congregation_of_data_managers("tmalign-many", operators::TMalignMany());
		set_command_for_congregation_of_data_managers("tmalign", operators::TMalign());
	}
};

}

}

#endif /* DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_ */
