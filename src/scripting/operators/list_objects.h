#ifndef SCRIPTING_OPERATORS_LIST_OBJECTS_H_
#define SCRIPTING_OPERATORS_LIST_OBJECTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ListObjects : public OperatorBase<ListObjects>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<VariantObject> objects;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.objects_array("objects")=objects;
		}
	};

	CongregationOfDataManagers::ObjectQuery query;

	ListObjects()
	{
	}

	void initialize(CommandInput& input)
	{
		query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		const std::vector<DataManager*> objects=congregation_of_data_managers.get_objects(query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
			VariantObject info;
			info.value("name")=attributes.name;
			info.value("picked")=attributes.picked;
			info.value("visible")=attributes.visible;
			result.objects.push_back(info);
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_LIST_OBJECTS_H_ */
