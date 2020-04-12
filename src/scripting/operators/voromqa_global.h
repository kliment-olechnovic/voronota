#ifndef SCRIPTING_OPERATORS_VOROMQA_GLOBAL_H_
#define SCRIPTING_OPERATORS_VOROMQA_GLOBAL_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_using_voromqa.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class VoroMQAGlobal : public OperatorBase<VoroMQAGlobal>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		VariantObject voromqa_result;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object=voromqa_result;
		}
	};

	ScoringOfDataManagerUsingVoroMQA::Parameters parameters_to_score_using_voromqa;
	std::string global_adj_prefix;

	VoroMQAGlobal()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_to_score_using_voromqa=ScoringOfDataManagerUsingVoroMQA::Parameters();
		parameters_to_score_using_voromqa.adjunct_inter_atom_energy_scores_raw=input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
		parameters_to_score_using_voromqa.adjunct_inter_atom_energy_scores_normalized=input.get_value_or_default<std::string>("adj-contact-energy-normalized", "");
		parameters_to_score_using_voromqa.adjunct_atom_depth_weights=input.get_value_or_default<std::string>("adj-atom-depth", "voromqa_depth");
		parameters_to_score_using_voromqa.adjunct_atom_quality_scores=input.get_value_or_default<std::string>("adj-atom-quality", "voromqa_score_a");
		parameters_to_score_using_voromqa.adjunct_residue_quality_scores_raw=input.get_value_or_default<std::string>("adj-residue-quality-raw", "");
		parameters_to_score_using_voromqa.adjunct_residue_quality_scores_smoothed=input.get_value_or_default<std::string>("adj-residue-quality", "voromqa_score_r");
		parameters_to_score_using_voromqa.smoothing_window=input.get_value_or_default<unsigned int>("smoothing-window", parameters_to_score_using_voromqa.smoothing_window);
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "voromqa_global");
	}

	void document(CommandDocumentation& doc) const
	{
		ScoringOfDataManagerUsingVoroMQA::Parameters params;
		doc.set_option_decription(CDOD("adj-contact-energy", CDOD::DATATYPE_STRING, "name of output adjunct for raw energy values", "voromqa_energy"));
		doc.set_option_decription(CDOD("adj-contact-energy-normalized", CDOD::DATATYPE_STRING, "name of output adjunct for normalized energy values", ""));
		doc.set_option_decription(CDOD("adj-atom-depth", CDOD::DATATYPE_STRING, "name of output adjunct for atom values", "voromqa_depth"));
		doc.set_option_decription(CDOD("adj-atom-quality", CDOD::DATATYPE_STRING, "name of output adjunct for atom quality scores", "voromqa_score_a"));
		doc.set_option_decription(CDOD("adj-residue-quality-raw", CDOD::DATATYPE_STRING, "name of output adjunct for raw residue quality scores", ""));
		doc.set_option_decription(CDOD("adj-residue-quality", CDOD::DATATYPE_STRING, "name of output adjunct for smoothed residue quality scores", "voromqa_score_r"));
		doc.set_option_decription(CDOD("smoothing-window", CDOD::DATATYPE_INT, "smoothing window size", params.smoothing_window));
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", "voromqa_global"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(parameters_to_score_using_voromqa.adjunct_inter_atom_energy_scores_raw, true);
		assert_adjunct_name_input(parameters_to_score_using_voromqa.adjunct_inter_atom_energy_scores_normalized, true);
		assert_adjunct_name_input(parameters_to_score_using_voromqa.adjunct_atom_depth_weights, true);
		assert_adjunct_name_input(parameters_to_score_using_voromqa.adjunct_atom_quality_scores, true);
		assert_adjunct_name_input(parameters_to_score_using_voromqa.adjunct_residue_quality_scores_raw, true);
		assert_adjunct_name_input(parameters_to_score_using_voromqa.adjunct_residue_quality_scores_smoothed, true);

		ScoringOfDataManagerUsingVoroMQA::Result voromqa_result;
		ScoringOfDataManagerUsingVoroMQA::construct_result(parameters_to_score_using_voromqa, data_manager, voromqa_result);

		if(!global_adj_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_quality_score"]=voromqa_result.global_quality_score;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_atoms_count"]=voromqa_result.bundle_of_quality.atom_quality_scores.size();
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_residues_count"]=voromqa_result.bundle_of_quality.raw_residue_quality_scores.size();
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_contacts_count"]=voromqa_result.bundle_of_energy.global_energy_descriptor.contacts_count;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_total_area"]=voromqa_result.bundle_of_energy.global_energy_descriptor.total_area;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_strange_area"]=voromqa_result.bundle_of_energy.global_energy_descriptor.strange_area;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_pseudo_energy"]=voromqa_result.bundle_of_energy.global_energy_descriptor.energy;
			if(voromqa_result.bundle_of_energy.global_energy_descriptor.total_area>0.0)
			{
				data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_pseudo_energy_norm"]=voromqa_result.bundle_of_energy.global_energy_descriptor.energy/voromqa_result.bundle_of_energy.global_energy_descriptor.total_area;
			}
			else
			{
				data_manager.global_numeric_adjuncts_mutable().erase(global_adj_prefix+"_pseudo_energy_norm");
			}
		}

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

}

#endif /* SCRIPTING_OPERATORS_VOROMQA_GLOBAL_H_ */
