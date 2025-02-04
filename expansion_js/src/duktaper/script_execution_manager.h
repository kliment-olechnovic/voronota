#ifndef DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_
#define DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_

#include "../../../src/scripting/script_execution_manager_with_variant_output.h"

#include "operators/call_shell.h"
#include "operators/checksum.h"
#include "operators/congruence_score.h"
#include "operators/congruence_score_many.h"
#include "operators/construct_and_collect_inter_atom_contact_area_ranges.h"
#include "operators/construct_contacts_radically_fast.h"
#include "operators/construct_or_load_contacts.h"
#include "operators/construct_or_load_quality_scores.h"
#include "operators/convert_bff_obj_to_svg.h"
#include "operators/export_atoms_to_mmcif.h"
#include "operators/export_atoms_to_mmcif_multimodel.h"
#include "operators/faspr.h"
#include "operators/fetch.h"
#include "operators/fetch_afdb.h"
#include "operators/fetch_mmcif.h"
#include "operators/import_cod_cif.h"
#include "operators/import_mmcif.h"
#include "operators/import_url.h"
#include "operators/nnport_predict.h"
#include "operators/plot_contacts_map.h"
#include "operators/pulchra.h"
#include "operators/qcprot_many.h"
#include "operators/qcprot.h"
#include "operators/run_bff.h"
#include "operators/run_hbplus.h"
#include "operators/run_nolb.h"
#include "operators/run_pulchra.h"
#include "operators/set_adjuncts_of_atoms_by_pca.h"
#include "operators/setup_defaults.h"
#include "operators/summarize_two_state_motion.h"
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

		set_command_for_data_manager("construct-contacts-radically-fast", operators::ConstructContactsRadicallyFast(), true);
		set_command_for_data_manager("construct-or-load-contacts", operators::ConstructOrLoadContacts(), true);
		set_command_for_data_manager("construct-or-load-quality-scores", operators::ConstructOrLoadQualityScores(), true);
		set_command_for_data_manager("export-atoms-to-mmcif", operators::ExportAtomsToMMCIF(), false);
		set_command_for_data_manager("faspr", operators::FASPR(), true);
		set_command_for_data_manager("plot-contacts-map", operators::PlotContactsMap(), true);
		set_command_for_data_manager("pulchra", operators::PULCHRA(), true);
		set_command_for_data_manager("run-bff", operators::RunBFF(), true);
		set_command_for_data_manager("run-hbplus", operators::RunHBPlus(), true);
		set_command_for_data_manager("run-pulchra", operators::RunPULCHRA(), true);
		set_command_for_data_manager("set-adjuncts-of-atoms-by-pca", operators::SetAdjunctsOfAtomsByPCA(), true);
		set_command_for_data_manager("voromqa-dark-global", operators::VoroMQADarkGlobal(), true);
		set_command_for_data_manager("voromqa-dark-local", operators::VoroMQADarkLocal(), true);
		set_command_for_data_manager("voromqa-dark-split", operators::VoroMQADarkSplit(), true);

		set_command_for_congregation_of_data_managers("construct-and-collect-inter-atom-contact-area-ranges", operators::ConstructAndCollectInterAtomContactAreaRanges());
		set_command_for_congregation_of_data_managers("congruence-score", operators::CongruenceScore());
		set_command_for_congregation_of_data_managers("congruence-score-many", operators::CongruenceScoreMany());
		set_command_for_congregation_of_data_managers("export-atoms-to-mmcif-multimodel", operators::ExportAtomsToMMCIFMultimodel());
		set_command_for_congregation_of_data_managers("fetch", operators::Fetch(RemoteImportDownloaderSimple< RemoteImportRequest<scripting::operators::ImportMany> >::instance()));
		set_command_for_congregation_of_data_managers("fetch-afdb", operators::FetchAFDB(RemoteImportDownloaderSimple< RemoteImportRequest<scripting::operators::ImportMany> >::instance()));
		set_command_for_congregation_of_data_managers("fetch-mmcif", operators::FetchMMCIF(RemoteImportDownloaderSimple< RemoteImportRequest<operators::ImportMMCIF> >::instance()));
		set_command_for_congregation_of_data_managers("import-cod-cif", operators::ImportCODCIF());
		set_command_for_congregation_of_data_managers("import-mmcif", operators::ImportMMCIF());
		set_command_for_congregation_of_data_managers("import-mmcif-url", operators::ImportUrl< RemoteImportDownloaderSimple< RemoteImportRequest<operators::ImportMMCIF> > >());
		set_command_for_congregation_of_data_managers("import-url", operators::ImportUrl< RemoteImportDownloaderSimple< RemoteImportRequest<scripting::operators::ImportMany> > >());
		set_command_for_congregation_of_data_managers("qcprot-many", operators::QCProtMany());
		set_command_for_congregation_of_data_managers("qcprot", operators::QCProt());
		set_command_for_congregation_of_data_managers("run-nolb", operators::RunNolb());
		set_command_for_congregation_of_data_managers("summarize-two-state-motion", operators::SummarizeTwoStateMotion());
		set_command_for_congregation_of_data_managers("tmalign-many", operators::TMalignMany());
		set_command_for_congregation_of_data_managers("tmalign", operators::TMalign());
	}
};

}

}

#endif /* DUKTAPER_SCRIPT_EXECUTION_MANAGER_H_ */
