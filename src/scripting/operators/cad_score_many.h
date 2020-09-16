#ifndef SCRIPTING_OPERATORS_CAD_SCORE_MANY_H_
#define SCRIPTING_OPERATORS_CAD_SCORE_MANY_H_

#include "cad_score.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CADScoreMany : public OperatorBase<CADScoreMany>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<CADScore::Result> cad_score_results;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<cad_score_results.size();i++)
			{
				std::vector<scripting::VariantObject>& voa=heterostorage.variant_object.objects_array("cad_score_results");
				voa.push_back(scripting::VariantObject());
				cad_score_results[i].store(voa.back());
			}
		}
	};

	scripting::CongregationOfDataManagers::ObjectQuery query;
	CADScore cad_score_operator;

	CADScoreMany()
	{
	}

	void initialize(CommandInput& input)
	{
		query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		cad_score_operator.initialize(input, true);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		cad_score_operator.document(doc, true);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(congregation_of_data_managers.get_objects().size()<2)
		{
			throw std::runtime_error(std::string("Less than two objects overall available."));
		}

		congregation_of_data_managers.assert_object_availability(cad_score_operator.target_name);

		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects(query);

		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		CADScore cad_score_operator_to_use=cad_score_operator;

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
			if(attributes.name!=cad_score_operator_to_use.target_name)
			{
				cad_score_operator_to_use.model_name=attributes.name;
				result.cad_score_results.push_back(cad_score_operator_to_use.run(congregation_of_data_managers));
			}
		}

		if(result.cad_score_results.empty())
		{
			throw std::runtime_error(std::string("No valid pairs of objects."));
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CAD_SCORE_MANY_H_ */
