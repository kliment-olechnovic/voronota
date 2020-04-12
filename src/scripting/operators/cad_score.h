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
		common::ConstructionOfCADScore::CADDescriptor atom_level_result;
		common::ConstructionOfCADScore::CADDescriptor residue_level_result;

		void store(HeterogeneousStorage& heterostorage) const
		{
			if(atom_level_result.target_area_sum>0.0)
			{
				write_cad_descriptor(atom_level_result, heterostorage.variant_object.object("atom_level_result"));
			}
			if(residue_level_result.target_area_sum>0.0)
			{
				write_cad_descriptor(residue_level_result, heterostorage.variant_object.object("residue_level_result"));
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

	void initialize(CommandInput& input)
	{
		target_name=input.get_value<std::string>("target");
		model_name=input.get_value<std::string>("model");
		params=ScoringOfDataManagersUsingCADScore::Parameters();
		params.target_selection_expression=input.get_value_or_default<std::string>("t-sel", "[--no-solvent --min-seq-sep 1]");
		params.model_selection_expression=input.get_value_or_default<std::string>("m-sel", params.target_selection_expression);
		params.target_adjunct_atom_scores=input.get_value_or_default<std::string>("t-adj-atom", "");
		params.target_adjunct_inter_atom_scores=input.get_value_or_default<std::string>("t-adj-inter-atom", "");
		params.target_adjunct_residue_scores=input.get_value_or_default<std::string>("t-adj-residue", "");
		params.target_adjunct_inter_residue_scores=input.get_value_or_default<std::string>("t-adj-inter-residue", "");
		params.model_adjunct_atom_scores=input.get_value_or_default<std::string>("m-adj-atom", "");
		params.model_adjunct_inter_atom_scores=input.get_value_or_default<std::string>("m-adj-inter-atom", "");
		params.model_adjunct_residue_scores=input.get_value_or_default<std::string>("m-adj-residue", "");
		params.model_adjunct_inter_residue_scores=input.get_value_or_default<std::string>("m-adj-inter-residue", "");
		params.depth=input.get_value_or_default<int>("depth", 0);
		params.smoothing_window=input.get_value_or_default<unsigned int>("smoothing-window", 0);
		params.ignore_residue_names=input.get_flag("ignore-residue-names");
		params.binarize=input.get_flag("binarize");
		target_global_adj_prefix=input.get_value_or_default<std::string>("t-global-adj-prefix", "");
		model_global_adj_prefix=input.get_value_or_default<std::string>("m-global-adj-prefix", "");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("target", CDOD::DATATYPE_STRING, "target object name"));
		doc.set_option_decription(CDOD("model", CDOD::DATATYPE_STRING, "model object name"));
		doc.set_option_decription(CDOD("t-sel", CDOD::DATATYPE_STRING, "target selection expression", "[--no-solvent --min-seq-sep 1]"));
		doc.set_option_decription(CDOD("m-sel", CDOD::DATATYPE_STRING, "model selection expression", "[--no-solvent --min-seq-sep 1]"));
		doc.set_option_decription(CDOD("t-adj-atom", CDOD::DATATYPE_STRING, "target adjunct name for atom scores", ""));
		doc.set_option_decription(CDOD("t-adj-inter-atom", CDOD::DATATYPE_STRING, "target adjunct name for inter-atom scores", ""));
		doc.set_option_decription(CDOD("t-adj-residue", CDOD::DATATYPE_STRING, "target adjunct name for residue scores", ""));
		doc.set_option_decription(CDOD("t-adj-inter-residue", CDOD::DATATYPE_STRING, "target adjunct name for inter-residue scores", ""));
		doc.set_option_decription(CDOD("m-adj-atom", CDOD::DATATYPE_STRING, "model adjunct name for atom scores", ""));
		doc.set_option_decription(CDOD("m-adj-inter-atom", CDOD::DATATYPE_STRING, "model adjunct name for inter-atom scores", ""));
		doc.set_option_decription(CDOD("m-adj-residue", CDOD::DATATYPE_STRING, "model adjunct name for residue scores", ""));
		doc.set_option_decription(CDOD("m-adj-inter-residue", CDOD::DATATYPE_STRING, "model adjunct name for inter-residue scores", ""));
		doc.set_option_decription(CDOD("depth", CDOD::DATATYPE_INT, "local neighborhood depth", 0));
		doc.set_option_decription(CDOD("smoothing-window", CDOD::DATATYPE_INT, "smoothing window size", 0));
		doc.set_option_decription(CDOD("ignore-residue-names", CDOD::DATATYPE_BOOL, "flag to ignore residue names"));
		doc.set_option_decription(CDOD("binarize", CDOD::DATATYPE_BOOL, "flag to use binary contact description"));
		doc.set_option_decription(CDOD("t-global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts of target", ""));
		doc.set_option_decription(CDOD("m-global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts of model", ""));
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(target_name==model_name)
		{
			throw std::runtime_error(std::string("Target and model are the same."));
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

		if(cadscore_result.bundle.parameters_of_construction.atom_level)
		{
			result.atom_level_result=cadscore_result.bundle.atom_level_global_descriptor;
		}

		if(cadscore_result.bundle.parameters_of_construction.residue_level)
		{
			result.residue_level_result=cadscore_result.bundle.residue_level_global_descriptor;
		}

		return result;
	}
};

}

}




}

#endif /* SCRIPTING_OPERATORS_CAD_SCORE_H_ */
