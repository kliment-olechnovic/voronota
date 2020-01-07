#ifndef SCRIPTING_OPERATORS_COPY_OBJECT_H_
#define SCRIPTING_OPERATORS_COPY_OBJECT_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class CopyObject : public OperatorBase<CopyObject>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string object_name;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("object_name")=object_name;
			return (*this);
		}
	};

	std::string name_original;
	std::string name_new;

	CopyObject()
	{
	}

	CopyObject& init(CommandInput& input)
	{
		name_original=input.get_value_or_first_unused_unnamed_value("original");
		name_new=input.get_value_or_first_unused_unnamed_value("new");
		return (*this);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(name_original.empty())
		{
			throw std::runtime_error(std::string("Empty first name provided for renaming."));
		}

		if(name_new.empty())
		{
			throw std::runtime_error(std::string("Empty second name provided for renaming."));
		}

		if(name_original==name_new)
		{
			throw std::runtime_error(std::string("Equal object names provided."));
		}

		Utilities::assert_new_object_name_input(name_new);

		congregation_of_data_managers.assert_object_availability(name_original);

		DataManager* object_original=congregation_of_data_managers.get_object(name_original);
		DataManager* object_new=congregation_of_data_managers.add_object(*object_original, name_new);

		Result result;
		result.object_name=congregation_of_data_managers.get_object_attributes(object_new).name;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_COPY_OBJECT_H_ */
