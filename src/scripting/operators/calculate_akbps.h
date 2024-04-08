#ifndef SCRIPTING_OPERATORS_CALCULATE_AKBPS_H_
#define SCRIPTING_OPERATORS_CALCULATE_AKBPS_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_interfaces_using_area_kbps.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CalculateAKBPs : public OperatorBase<CalculateAKBPs>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		ScoringOfDataManagerInterfacesUsingAreaKBPs::Result scoring_result;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(scoring_result.contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			VariantObject& info=heterostorage.variant_object.object("scoring_summary");
			info.value("known_area")=scoring_result.known_area;
			info.value("unknown_area")=scoring_result.unknown_area;
			for(std::size_t i=0;i<scoring_result.score_names.size() && i<scoring_result.score_values.size();i++)
			{
				info.value(std::string("score_")+scoring_result.score_names[i])=scoring_result.score_values[i];
				info.value(std::string("score_")+scoring_result.score_names[i]+"_solvated")=scoring_result.score_values_solvated[i];
			}
		}
	};

	ScoringOfDataManagerInterfacesUsingAreaKBPs::Parameters params;

	CalculateAKBPs()
	{
	}

	void initialize(CommandInput& input)
	{
		params.parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query("", "[--inter-chain]", input);
		params.adjunct_prefix=input.get_value_or_default<std::string>("adj-prefix", "");
		params.global_adjunct_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query("", "[--inter-chain]", doc);
		doc.set_option_decription(CDOD("adj-prefix", CDOD::DATATYPE_STRING, "adjunct names prefix write KBP-based scores", ""));
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "global adjunct names prefix write KBP-based scores", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(params.adjunct_prefix+"_name", true);

		Result result;

		ScoringOfDataManagerInterfacesUsingAreaKBPs::construct_result(params, data_manager, result.scoring_result);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CALCULATE_AKBPS_H_ */
