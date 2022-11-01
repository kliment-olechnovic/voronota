#ifndef DUKTAPER_OPERATORS_CONGRUENCE_SCORE_MANY_H_
#define DUKTAPER_OPERATORS_CONGRUENCE_SCORE_MANY_H_

#include "congruence_score.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class CongruenceScoreMany : public scripting::OperatorBase<CongruenceScoreMany>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::vector<CongruenceScore::Result> congruence_score_results;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<congruence_score_results.size();i++)
			{
				std::vector<scripting::VariantObject>& voa=heterostorage.variant_object.objects_array("congruence_score_results");
				voa.push_back(scripting::VariantObject());
				congruence_score_results[i].store(voa.back());
			}
		}
	};

	scripting::CongregationOfDataManagers::ObjectQuery query;
	CongruenceScore congruence_score_operator;

	CongruenceScoreMany()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		query=scripting::OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		congruence_score_operator.initialize(input, true);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		congruence_score_operator.document(doc, true);
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(congregation_of_data_managers.get_objects().size()<2)
		{
			throw std::runtime_error(std::string("Less than two objects overall available."));
		}

		congregation_of_data_managers.assert_object_availability(congruence_score_operator.target_name);

		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects(query);

		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		CongruenceScore congruence_score_operator_to_use=congruence_score_operator;

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
			if(attributes.name!=congruence_score_operator_to_use.target_name)
			{
				congruence_score_operator_to_use.model_name=attributes.name;
				result.congruence_score_results.push_back(congruence_score_operator_to_use.run(congregation_of_data_managers));
			}
		}

		if(result.congruence_score_results.empty())
		{
			throw std::runtime_error(std::string("No valid pairs of objects."));
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CONGRUENCE_SCORE_MANY_H_ */
