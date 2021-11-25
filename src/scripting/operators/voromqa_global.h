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
		double quality_score;
		double weight_of_quality_score;
		std::size_t atoms_count;
		std::size_t residues_count;
		std::size_t contacts_count;
		double pseudo_energy;
		double total_area;
		double strange_area;
		double total_volume;

		Result() :
			quality_score(0.0),
			weight_of_quality_score(0.0),
			atoms_count(0),
			residues_count(0),
			contacts_count(0),
			pseudo_energy(0.0),
			total_area(0.0),
			strange_area(0.0),
			total_volume(0.0)
		{
		}

		void add(const Result& r)
		{
			const double sum_of_quality_scores=(quality_score*weight_of_quality_score+r.quality_score*r.weight_of_quality_score);
			weight_of_quality_score+=r.weight_of_quality_score;
			quality_score=(weight_of_quality_score>0.0 ? (sum_of_quality_scores/weight_of_quality_score) : 0.0);
			atoms_count+=r.atoms_count;
			residues_count+=r.residues_count;
			contacts_count+=r.contacts_count;
			pseudo_energy+=r.pseudo_energy;
			total_area+=r.total_area;
			strange_area+=r.strange_area;
			total_volume+=r.total_volume;
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			write_to_variant_object(heterostorage.variant_object);
		}

		void write_to_variant_object(scripting::VariantObject& variant_object) const
		{
			variant_object.value("quality_score")=quality_score;
			variant_object.value("weight_of_quality_score")=weight_of_quality_score;
			variant_object.value("atoms_count")=atoms_count;
			variant_object.value("residues_count")=residues_count;
			variant_object.value("contacts_count")=contacts_count;
			variant_object.value("pseudo_energy")=pseudo_energy;
			variant_object.value("total_area")=total_area;
			variant_object.value("strange_area")=strange_area;
			variant_object.value("total_volume")=total_volume;
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
		parameters_to_score_using_voromqa.adjunct_area_scale=input.get_value_or_default<std::string>("adj-area-scale", "");
		parameters_to_score_using_voromqa.adjunct_area_alt_part=input.get_value_or_default<std::string>("adj-area-alt-part", "");
		parameters_to_score_using_voromqa.adjunct_inter_atom_energy_scores_raw=input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
		parameters_to_score_using_voromqa.adjunct_inter_atom_energy_scores_normalized=input.get_value_or_default<std::string>("adj-contact-energy-normalized", "");
		parameters_to_score_using_voromqa.adjunct_inter_atom_split_alt_sas_energy_scores=input.get_value_or_default<std::string>("adj-contact-energy-split-to-sas", "");
		parameters_to_score_using_voromqa.adjunct_atom_depth_weights=input.get_value_or_default<std::string>("adj-atom-depth", "voromqa_depth");
		parameters_to_score_using_voromqa.adjunct_atom_quality_scores=input.get_value_or_default<std::string>("adj-atom-quality", "voromqa_score_a");
		parameters_to_score_using_voromqa.adjunct_atom_sas_potential_values=input.get_value_or_default<std::string>("adj-atom-sas-potential", "");
		parameters_to_score_using_voromqa.adjunct_residue_quality_scores_raw=input.get_value_or_default<std::string>("adj-residue-quality-raw", "");
		parameters_to_score_using_voromqa.adjunct_residue_quality_scores_smoothed=input.get_value_or_default<std::string>("adj-residue-quality", "voromqa_score_r");
		parameters_to_score_using_voromqa.smoothing_window=input.get_value_or_default<unsigned int>("smoothing-window", parameters_to_score_using_voromqa.smoothing_window);
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "voromqa_global");
	}

	void document(CommandDocumentation& doc) const
	{
		ScoringOfDataManagerUsingVoroMQA::Parameters params;
		doc.set_option_decription(CDOD("adj-area-scale", CDOD::DATATYPE_STRING, "name of input adjunct for contact area scaling", ""));
		doc.set_option_decription(CDOD("adj-area-alt-part", CDOD::DATATYPE_STRING, "name of input adjunct for contact area alternative fraction", ""));
		doc.set_option_decription(CDOD("adj-contact-energy", CDOD::DATATYPE_STRING, "name of output adjunct for raw energy values", "voromqa_energy"));
		doc.set_option_decription(CDOD("adj-contact-energy-normalized", CDOD::DATATYPE_STRING, "name of output adjunct for normalized energy values", ""));
		doc.set_option_decription(CDOD("adj-contact-energy-split-to-sas", CDOD::DATATYPE_STRING, "name prefix of output adjuncts for split-to-sas energy coefficients", ""));
		doc.set_option_decription(CDOD("adj-atom-depth", CDOD::DATATYPE_STRING, "name of output adjunct for atom values", "voromqa_depth"));
		doc.set_option_decription(CDOD("adj-atom-quality", CDOD::DATATYPE_STRING, "name of output adjunct for atom quality scores", "voromqa_score_a"));
		doc.set_option_decription(CDOD("adj-atom-sas-potential", CDOD::DATATYPE_STRING, "name of output adjunct for atom solvent interaction energy coefficients", ""));
		doc.set_option_decription(CDOD("adj-residue-quality-raw", CDOD::DATATYPE_STRING, "name of output adjunct for raw residue quality scores", ""));
		doc.set_option_decription(CDOD("adj-residue-quality", CDOD::DATATYPE_STRING, "name of output adjunct for smoothed residue quality scores", "voromqa_score_r"));
		doc.set_option_decription(CDOD("smoothing-window", CDOD::DATATYPE_INT, "smoothing window size", params.smoothing_window));
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", "voromqa_global"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(parameters_to_score_using_voromqa.adjunct_area_scale, true);
		assert_adjunct_name_input(parameters_to_score_using_voromqa.adjunct_area_alt_part, true);
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
		result.quality_score=voromqa_result.global_quality_score;
		result.weight_of_quality_score=voromqa_result.weight_of_global_quality_score;
		result.atoms_count=voromqa_result.bundle_of_quality.atom_quality_scores.size();
		result.residues_count=voromqa_result.bundle_of_quality.raw_residue_quality_scores.size();
		result.contacts_count=voromqa_result.bundle_of_energy.global_energy_descriptor.contacts_count;
		result.pseudo_energy=voromqa_result.bundle_of_energy.global_energy_descriptor.energy;
		result.total_area=voromqa_result.bundle_of_energy.global_energy_descriptor.total_area;
		result.strange_area=voromqa_result.bundle_of_energy.global_energy_descriptor.strange_area;
		result.total_volume=voromqa_result.total_volume_of_atoms;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_VOROMQA_GLOBAL_H_ */
