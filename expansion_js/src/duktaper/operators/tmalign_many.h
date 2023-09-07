#ifndef DUKTAPER_OPERATORS_TMALIGN_MANY_H_
#define DUKTAPER_OPERATORS_TMALIGN_MANY_H_

#include "tmalign.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class TMalignMany : public scripting::OperatorBase<TMalignMany>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::vector<TMalign::Result> tmalign_results;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<tmalign_results.size();i++)
			{
				std::vector<scripting::VariantObject>& voa=heterostorage.variant_object.objects_array("tmalign_results");
				voa.push_back(scripting::VariantObject());
				tmalign_results[i].store(voa.back());
			}
		}
	};

	scripting::CongregationOfDataManagers::ObjectQuery query;
	TMalign tmalign_operator;

	TMalignMany()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		query=scripting::OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		tmalign_operator.initialize(input, true);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		tmalign_operator.document(doc, true);
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		const std::vector<scripting::DataManager*> all_objects=congregation_of_data_managers.get_objects();

		if(all_objects.size()<2)
		{
			throw std::runtime_error(std::string("Less than two objects overall available."));
		}

		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects(query);

		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		TMalign tmalign_operator_to_use=tmalign_operator;

		tmalign_operator_to_use.target_name=(tmalign_operator.target_name.empty() ? congregation_of_data_managers.get_object_attributes(all_objects[0]).name : tmalign_operator.target_name);

		congregation_of_data_managers.assert_object_availability(tmalign_operator_to_use.target_name);

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
			if(attributes.name!=tmalign_operator_to_use.target_name)
			{
				tmalign_operator_to_use.model_name=attributes.name;
				result.tmalign_results.push_back(tmalign_operator_to_use.run(congregation_of_data_managers));
			}
		}

		if(result.tmalign_results.empty())
		{
			throw std::runtime_error(std::string("No valid pairs of objects."));
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_TMALIGN_MANY_H_ */
