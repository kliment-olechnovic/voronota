#ifndef SCRIPTING_OPERATORS_RENAME_OBJECT_H_
#define SCRIPTING_OPERATORS_RENAME_OBJECT_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class RenameObject
{
public:
	struct Result
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	std::string name_original;
	std::string name_new;

	RenameObject()
	{
	}

	RenameObject& init(CommandInput& input)
	{
		const std::vector<std::string>& names=input.get_list_of_unnamed_values();
		input.mark_all_unnamed_values_as_used();
		if(names.size()!=2)
		{
			throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
		}
		name_original=names[0];
		name_new=names[1];
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

		congregation_of_data_managers.rename_object(name_original, name_new);

		Result result;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_RENAME_OBJECT_H_ */
