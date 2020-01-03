#ifndef SCRIPTING_OPERATORS_VOROMQA_GLOBAL_H_
#define SCRIPTING_OPERATORS_VOROMQA_GLOBAL_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_using_voromqa.h"

namespace scripting
{

namespace operators
{

class VoroMQAGlobal
{
public:
	struct Result
	{
		VariantObject voromqa_result;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object=voromqa_result;
			return (*this);
		}
	};

	ScoringOfDataManagerUsingVoroMQA::Parameters params;

	VoroMQAGlobal()
	{
	}

	VoroMQAGlobal& init(CommandInput& input)
	{
		params=ScoringOfDataManagerUsingVoroMQA::Parameters();
		params.adjunct_inter_atom_energy_scores_raw=input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
		params.adjunct_inter_atom_energy_scores_normalized=input.get_value_or_default<std::string>("adj-contact-energy-normalized", "");
		params.adjunct_atom_depth_weights=input.get_value_or_default<std::string>("adj-atom-depth", "voromqa_depth");
		params.adjunct_atom_quality_scores=input.get_value_or_default<std::string>("adj-atom-quality", "voromqa_score_a");
		params.adjunct_residue_quality_scores_raw=input.get_value_or_default<std::string>("adj-residue-quality-raw", "");
		params.adjunct_residue_quality_scores_smoothed=input.get_value_or_default<std::string>("adj-residue-quality", "voromqa_score_r");
		params.smoothing_window=input.get_value_or_default<unsigned int>("smoothing-window", params.smoothing_window);
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(params.adjunct_inter_atom_energy_scores_raw, true);
		assert_adjunct_name_input(params.adjunct_inter_atom_energy_scores_normalized, true);
		assert_adjunct_name_input(params.adjunct_atom_depth_weights, true);
		assert_adjunct_name_input(params.adjunct_atom_quality_scores, true);
		assert_adjunct_name_input(params.adjunct_residue_quality_scores_raw, true);
		assert_adjunct_name_input(params.adjunct_residue_quality_scores_smoothed, true);

		ScoringOfDataManagerUsingVoroMQA::Result voromqa_result;
		ScoringOfDataManagerUsingVoroMQA::construct_result(params, data_manager, voromqa_result);

		Result result;
		result.voromqa_result.value("quality_score")=voromqa_result.global_quality_score;
		result.voromqa_result.value("atoms_count")=voromqa_result.bundle_of_quality.atom_quality_scores.size();
		result.voromqa_result.value("residues_count")=voromqa_result.bundle_of_quality.raw_residue_quality_scores.size();
		result.voromqa_result.value("contacts_count")=voromqa_result.bundle_of_energy.global_energy_descriptor.contacts_count;
		result.voromqa_result.value("pseudo_energy")=voromqa_result.bundle_of_energy.global_energy_descriptor.energy;
		result.voromqa_result.value("total_area")=voromqa_result.bundle_of_energy.global_energy_descriptor.total_area;
		result.voromqa_result.value("strange_area")=voromqa_result.bundle_of_energy.global_energy_descriptor.strange_area;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_VOROMQA_GLOBAL_H_ */
