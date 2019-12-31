#ifndef SCRIPTING_OPERATORS_DELETE_OBJECTS_H_
#define SCRIPTING_OPERATORS_DELETE_OBJECTS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class DeleteObjects
{
public:
	struct Result
	{
		std::vector<std::string> deleted_objects;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<deleted_objects.size();i++)
			{
				heterostorage.variant_object.values_array("deleted_objects").push_back(VariantValue(deleted_objects[i]));
			}
			return (*this);
		}
	};

	CongregationOfDataManagers::ObjectQuery query;

	DeleteObjects()
	{
	}

	DeleteObjects& init(CommandInput& input)
	{
		query=Utilities::read_congregation_of_data_managers_object_query(input);
		return (*this);
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
			result.deleted_objects.push_back(congregation_of_data_managers.get_object_attributes(objects[i]).name);
			congregation_of_data_managers.delete_object(objects[i]);
		}

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_OBJECTS_H_ */
