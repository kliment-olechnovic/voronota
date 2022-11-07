#ifndef DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_
#define DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"

#include "operators/call_shell.h"
#include "operators/checksum.h"
#include "operators/congruence_score.h"
#include "operators/congruence_score_many.h"
#include "operators/construct_or_load_contacts.h"
#include "operators/construct_or_load_quality_scores.h"
#include "operators/convert_bff_obj_to_svg.h"
#include "operators/faspr.h"
#include "operators/fetch.h"
#include "operators/fetch_afdb.h"
#include "operators/import_cod_cif.h"
#include "operators/import_url.h"
#include "operators/music_background.h"
#include "operators/nnport_predict.h"
#include "operators/plot_contacts_map.h"
#include "operators/run_bff.h"
#include "operators/run_nolb.h"
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
		set_command_for_extra_actions("convert-bff-obj-to-svg", operators::ConvertBFFObjToSVG());
		set_command_for_extra_actions("nnport-predict", operators::NNPortPredict());
		set_command_for_extra_actions("setup-defaults", operators::SetupDefaults());
		set_command_for_extra_actions("music-background", operators::MusicBackground());

		set_command_for_data_manager("construct-or-load-contacts", operators::ConstructOrLoadContacts(), true);
		set_command_for_data_manager("construct-or-load-quality-scores", operators::ConstructOrLoadQualityScores(), true);
		set_command_for_data_manager("faspr", operators::FASPR(), true);
		set_command_for_data_manager("plot-contacts-map", operators::PlotContactsMap(), true);
		set_command_for_data_manager("run-bff", operators::RunBFF(), true);
		set_command_for_data_manager("voromqa-dark-global", operators::VoroMQADarkGlobal(), true);
		set_command_for_data_manager("voromqa-dark-local", operators::VoroMQADarkLocal(), true);
		set_command_for_data_manager("voromqa-dark-split", operators::VoroMQADarkSplit(), true);

		set_command_for_congregation_of_data_managers("congruence-score", operators::CongruenceScore());
		set_command_for_congregation_of_data_managers("congruence-score-many", operators::CongruenceScoreMany());
		set_command_for_congregation_of_data_managers("fetch", operators::Fetch(RemoteImportDownloaderSimple::instance()));
		set_command_for_congregation_of_data_managers("fetch-afdb", operators::FetchAFDB(RemoteImportDownloaderSimple::instance()));
		set_command_for_congregation_of_data_managers("import-cod-cif", operators::ImportCODCIF());
		set_command_for_congregation_of_data_managers("import-url", operators::ImportUrl(RemoteImportDownloaderSimple::instance()));
		set_command_for_congregation_of_data_managers("run-nolb", operators::RunNolb());
		set_command_for_congregation_of_data_managers("tmalign-many", operators::TMalignMany());
		set_command_for_congregation_of_data_managers("tmalign", operators::TMalign());
	}
};

}

}

#endif /* DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_ */
