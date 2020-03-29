#ifndef VIEWER_OPERATORS_VOROMQA_DARK_GLOBAL_H_
#define VIEWER_OPERATORS_VOROMQA_DARK_GLOBAL_H_

#include "../../../src/scripting/operators_all.h"

#include "nnport_predict.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class VoroMQADarkGlobal : public scripting::operators::OperatorBase<VoroMQADarkGlobal>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		double global_score;

		Result() : global_score(0.0)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("global_score")=global_score;
		}
	};

	VoroMQADarkGlobal()
	{
	}

	void initialize(scripting::CommandInput&)
	{
	}

	void document(scripting::CommandDocumentation&) const
	{
	}

	Result run(scripting::DataManager& data_manager) const
	{
		scripting::operators::GenerateResidueVoroMQAEnergyProfile().init("-file _virtual/tmp/residue_voromqa_energy_profile").run(data_manager);

		{
			void* void_subject=0;
			operators::NNPortPredict().init(""
					"-input-value-column-names-file _virtual/voromqa_dark_nnport_input_header "
					"-input-statistics-file _virtual/voromqa_dark_nnport_input_statistics "
					"-input-model-files _virtual/voromqa_dark_nnport_input_fdeep_model_json "
					"-output-value-column-names vd1 vd2 vd3 vd4 vd5 vd6 "
					"-input-data-file _virtual/tmp/residue_voromqa_energy_profile "
					"-output-data-file _virtual/tmp/voromqa_dark_scores").run(void_subject);
		}

		scripting::operators::ImportAdjunctsOfAtoms().init("-file _virtual/tmp/voromqa_dark_scores").run(data_manager);

		Result result;

		result.global_score=scripting::operators::SpectrumAtoms().init("-use [-aname CA] -adjunct vd1 -only-summarize").run(data_manager).mean_of_values;

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_VOROMQA_DARK_GLOBAL_H_ */
