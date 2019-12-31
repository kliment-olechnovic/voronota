#ifndef SCRIPTING_OPERATORS_PICK_OBJECTS_H_
#define SCRIPTING_OPERATORS_PICK_OBJECTS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

template<bool positive, bool add>
class PickObjectsTemplate
{
public:
	struct Result
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	CongregationOfDataManagers::ObjectQuery query;

	PickObjectsTemplate()
	{
	}

	PickObjectsTemplate& init(CommandInput& input)
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

		if(positive && !add)
		{
			congregation_of_data_managers.set_all_objects_picked(false);
		}

		for(std::size_t i=0;i<objects.size();i++)
		{
			congregation_of_data_managers.set_object_picked(objects[i], positive);
		}

		Result result;

		return result;
	}
};

typedef PickObjectsTemplate<true, false> PickObjects;
typedef PickObjectsTemplate<true, true> PickMoreObjects;
typedef PickObjectsTemplate<false, false> UnpickObjects;

}

}

#endif /* SCRIPTING_OPERATORS_PICK_OBJECTS_H_ */
