#ifndef SCRIPTING_OPERATORS_CAD_SCORE_H_
#define SCRIPTING_OPERATORS_CAD_SCORE_H_

#include "../operators_common.h"
#include "../scoring_of_data_managers_using_cad_score.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CADScore : public OperatorBase<CADScore>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string target_name;
		std::string model_name;
		common::ConstructionOfCADScore::CADDescriptor atom_level_result;
		common::ConstructionOfCADScore::CADDescriptor residue_level_result;
		common::ConstructionOfCADScore::CADDescriptor site_residue_level_result;

		void store(HeterogeneousStorage& heterostorage) const
		{
			store(heterostorage.variant_object);
		}

		void store(VariantObject& variant_object) const
		{
			variant_object.value("target_name")=target_name;
			variant_object.value("model_name")=model_name;
			if(atom_level_result.target_area_sum>0.0)
			{
				write_cad_descriptor(atom_level_result, variant_object.object("atom_level_result"));
			}
			if(residue_level_result.target_area_sum>0.0)
			{
				write_cad_descriptor(residue_level_result, variant_object.object("residue_level_result"));
			}
			if(site_residue_level_result.target_area_sum>0.0)
			{
				write_cad_descriptor(site_residue_level_result, variant_object.object("site_residue_level_result"));
			}
		}

		static void write_cad_descriptor(const common::ConstructionOfCADScore::CADDescriptor& cadd, VariantObject& output)
		{
			output.value("score")=cadd.score();
			output.value("target_area_sum")=cadd.target_area_sum;
			output.value("model_area_sum")=cadd.model_area_sum;
			output.value("raw_differences_sum")=cadd.raw_differences_sum;
			output.value("constrained_differences_sum")=cadd.constrained_differences_sum;
			output.value("model_target_area_sum")=cadd.model_target_area_sum;
			output.value("score_F1")=cadd.score_F1();
			output.value("confusion_TP")=cadd.confusion_TP;
			output.value("confusion_FP")=cadd.confusion_FP;
			output.value("confusion_FN")=cadd.confusion_FN;
		}
	};

	std::string target_name;
	std::string model_name;
	ScoringOfDataManagersUsingCADScore::Parameters params;
	std::string target_global_adj_prefix;
	std::string model_global_adj_prefix;

	CADScore()
	{
	}

	void initialize(CommandInput& input, const bool managed)
	{
		target_name=input.get_value<std::string>("target");
		if(!managed)
		{
			model_name=input.get_value<std::string>("model");
		}
		params=ScoringOfDataManagersUsingCADScore::Parameters();
		params.target_selection_expression=input.get_value_or_default<std::string>("t-sel", "[--no-solvent --min-seq-sep 1]");
		params.model_selection_expression=input.get_value_or_default<std::string>("m-sel", params.target_selection_expression);
		if(!managed)
		{
			params.target_adjunct_atom_scores=input.get_value_or_default<std::string>("t-adj-atom", "");
			params.target_adjunct_inter_atom_scores=input.get_value_or_default<std::string>("t-adj-inter-atom", "");
			params.target_adjunct_inter_atom_relevant_areas=input.get_value_or_default<std::string>("t-adj-inter-atom-relevant-areas", "");
			params.target_adjunct_residue_scores=input.get_value_or_default<std::string>("t-adj-residue", "");
			params.target_adjunct_inter_residue_scores=input.get_value_or_default<std::string>("t-adj-inter-residue", "");
			params.target_adjunct_inter_residue_relevant_areas=input.get_value_or_default<std::string>("t-adj-inter-residue-relevant-areas", "");
		}
		params.model_adjunct_atom_scores=input.get_value_or_default<std::string>("m-adj-atom", "");
		params.model_adjunct_inter_atom_scores=input.get_value_or_default<std::string>("m-adj-inter-atom", "");
		params.model_adjunct_inter_atom_relevant_areas=input.get_value_or_default<std::string>("m-adj-inter-atom-relevant-areas", "");
		params.model_adjunct_residue_scores=input.get_value_or_default<std::string>("m-adj-residue", "");
		params.model_adjunct_inter_residue_scores=input.get_value_or_default<std::string>("m-adj-inter-residue", "");
		params.model_adjunct_inter_residue_relevant_areas=input.get_value_or_default<std::string>("m-adj-inter-residue-relevant-areas", "");
		params.depth=input.get_value_or_default<int>("depth", 0);
		params.max_chains_to_fully_permute=input.get_value_or_default<int>("max-chains-to-fully-permute", 6);
		params.smoothing_window=input.get_value_or_default<unsigned int>("smoothing-window", 0);
		params.ignore_residue_names=input.get_flag("ignore-residue-names");
		params.binarize=input.get_flag("binarize");
		params.also_site_based=input.get_flag("also-site-based");
		params.site_selection_expression=input.get_value_or_default<std::string>("site-sel", "[]");
		params.chain_renaming_pairs=input.get_value_vector_or_default<std::string>("m-chain-renaming-pairs", std::vector<std::string>());
		params.remap_chains=input.get_flag("remap-chains");
		params.remap_chains_logging=input.get_flag("remap-chains-logging");
		if(!managed)
		{
			target_global_adj_prefix=input.get_value_or_default<std::string>("t-global-adj-prefix", "");
		}
		model_global_adj_prefix=input.get_value_or_default<std::string>("m-global-adj-prefix", "");
	}

	void initialize(scripting::CommandInput& input)
	{
		initialize(input, false);
	}

	void document(CommandDocumentation& doc, const bool managed) const
	{
		doc.set_option_decription(CDOD("target", CDOD::DATATYPE_STRING, "target object name"));
		if(!managed)
		{
			doc.set_option_decription(CDOD("model", CDOD::DATATYPE_STRING, "model object name"));
		}
		doc.set_option_decription(CDOD("t-sel", CDOD::DATATYPE_STRING, "target selection expression", "[--no-solvent --min-seq-sep 1]"));
		doc.set_option_decription(CDOD("m-sel", CDOD::DATATYPE_STRING, "model selection expression", "[--no-solvent --min-seq-sep 1]"));
		if(!managed)
		{
			doc.set_option_decription(CDOD("t-adj-atom", CDOD::DATATYPE_STRING, "target adjunct name for atom scores", ""));
			doc.set_option_decription(CDOD("t-adj-inter-atom", CDOD::DATATYPE_STRING, "target adjunct name for inter-atom scores", ""));
			doc.set_option_decription(CDOD("t-adj-residue", CDOD::DATATYPE_STRING, "target adjunct name for residue scores", ""));
			doc.set_option_decription(CDOD("t-adj-inter-residue", CDOD::DATATYPE_STRING, "target adjunct name for inter-residue scores", ""));
		}
		doc.set_option_decription(CDOD("m-adj-atom", CDOD::DATATYPE_STRING, "model adjunct name for atom scores", ""));
		doc.set_option_decription(CDOD("m-adj-inter-atom", CDOD::DATATYPE_STRING, "model adjunct name for inter-atom scores", ""));
		doc.set_option_decription(CDOD("m-adj-inter-atom-relevant-areas", CDOD::DATATYPE_STRING, "model adjunct name for inter-atom relevant areas", ""));
		doc.set_option_decription(CDOD("m-adj-residue", CDOD::DATATYPE_STRING, "model adjunct name for residue scores", ""));
		doc.set_option_decription(CDOD("m-adj-inter-residue", CDOD::DATATYPE_STRING, "model adjunct name for inter-residue scores", ""));
		doc.set_option_decription(CDOD("m-adj-inter-residue-relevant-areas", CDOD::DATATYPE_STRING, "model adjunct name for inter-residue relevant areas", ""));
		doc.set_option_decription(CDOD("depth", CDOD::DATATYPE_INT, "local neighborhood depth", 0));
		doc.set_option_decription(CDOD("max-chains-to-fully-permute", CDOD::DATATYPE_INT, "maximum number of chains for switching to exhaustive remapping", 6));
		doc.set_option_decription(CDOD("smoothing-window", CDOD::DATATYPE_INT, "smoothing window size", 0));
		doc.set_option_decription(CDOD("ignore-residue-names", CDOD::DATATYPE_BOOL, "flag to ignore residue names"));
		doc.set_option_decription(CDOD("binarize", CDOD::DATATYPE_BOOL, "flag to use binary contact description"));
		doc.set_option_decription(CDOD("also-site-based", CDOD::DATATYPE_BOOL, "flag to also compute site-based score"));
		doc.set_option_decription(CDOD("site-sel", CDOD::DATATYPE_STRING, "site atoms selection expression for site-based scoring", "[]"));
		doc.set_option_decription(CDOD("m-chain-renaming-pairs", CDOD::DATATYPE_STRING_ARRAY, "source and destination pairs for model chain renaming", ""));
		doc.set_option_decription(CDOD("remap-chains", CDOD::DATATYPE_BOOL, "flag to calculate and use optimal chains remapping"));
		if(!managed)
		{
			doc.set_option_decription(CDOD("t-global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts of target", ""));
		}
		doc.set_option_decription(CDOD("m-global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts of model", ""));
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		document(doc, false);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(target_name.empty())
		{
			throw std::runtime_error(std::string("No target object name provided."));
		}

		if(model_name.empty())
		{
			throw std::runtime_error(std::string("No model object name provided."));
		}

		if(params.chain_renaming_pairs.size()>0 && params.chain_renaming_pairs.size()%2!=0)
		{
			throw std::runtime_error(std::string("Incomplete pair in the chain renaming vector."));
		}

		assert_adjunct_name_input(params.target_adjunct_atom_scores, true);
		assert_adjunct_name_input(params.target_adjunct_inter_atom_scores, true);
		assert_adjunct_name_input(params.target_adjunct_residue_scores, true);
		assert_adjunct_name_input(params.target_adjunct_inter_residue_scores, true);
		assert_adjunct_name_input(params.model_adjunct_atom_scores, true);
		assert_adjunct_name_input(params.model_adjunct_inter_atom_scores, true);
		assert_adjunct_name_input(params.model_adjunct_residue_scores, true);
		assert_adjunct_name_input(params.model_adjunct_inter_residue_scores, true);

		congregation_of_data_managers.assert_object_availability(target_name);
		congregation_of_data_managers.assert_object_availability(model_name);

		DataManager& target_dm=*congregation_of_data_managers.get_object(target_name);
		DataManager& model_dm=*congregation_of_data_managers.get_object(model_name);

		ScoringOfDataManagersUsingCADScore::Result cadscore_result;
		ScoringOfDataManagersUsingCADScore::construct_result(params, target_dm, model_dm, cadscore_result);

		if(!target_global_adj_prefix.empty())
		{
			if(cadscore_result.bundle.parameters_of_construction.atom_level)
			{
				target_dm.global_numeric_adjuncts_mutable()[target_global_adj_prefix+"_atom_level_cad_score"]=cadscore_result.bundle.atom_level_global_descriptor.score();
			}
			if(cadscore_result.bundle.parameters_of_construction.residue_level)
			{
				target_dm.global_numeric_adjuncts_mutable()[target_global_adj_prefix+"_residue_level_cad_score"]=cadscore_result.bundle.residue_level_global_descriptor.score();
			}
		}

		if(!model_global_adj_prefix.empty())
		{
			if(cadscore_result.bundle.parameters_of_construction.atom_level)
			{
				model_dm.global_numeric_adjuncts_mutable()[model_global_adj_prefix+"_atom_level_cad_score"]=cadscore_result.bundle.atom_level_global_descriptor.score();
			}
			if(cadscore_result.bundle.parameters_of_construction.residue_level)
			{
				model_dm.global_numeric_adjuncts_mutable()[model_global_adj_prefix+"_residue_level_cad_score"]=cadscore_result.bundle.residue_level_global_descriptor.score();
			}
		}

		Result result;

		result.target_name=target_name;
		result.model_name=model_name;

		if(cadscore_result.bundle.parameters_of_construction.atom_level)
		{
			result.atom_level_result=cadscore_result.bundle.atom_level_global_descriptor;
		}

		if(cadscore_result.bundle.parameters_of_construction.residue_level)
		{
			result.residue_level_result=cadscore_result.bundle.residue_level_global_descriptor;
		}

		if(cadscore_result.site_bundle.parameters_of_construction.residue_level)
		{
			result.site_residue_level_result=cadscore_result.site_bundle.residue_level_global_descriptor;
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CAD_SCORE_H_ */
