#ifndef SCRIPTING_OPERATORS_RENAME_OBJECT_H_
#define SCRIPTING_OPERATORS_RENAME_OBJECT_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class RenameObject : public OperatorBase<RenameObject>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string name_original;
	std::string name_new;

	RenameObject()
	{
	}

	void initialize(CommandInput& input)
	{
		name_original=input.get_value_or_first_unused_unnamed_value("original");
		name_new=input.get_value_or_first_unused_unnamed_value("new");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("original", CDOD::DATATYPE_STRING, "original object name"));
		doc.set_option_decription(CDOD("new", CDOD::DATATYPE_STRING, "new object name"));
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

		OperatorsUtilities::assert_new_object_name_input(name_new);

		congregation_of_data_managers.assert_object_availability(name_original);

		congregation_of_data_managers.rename_object(name_original, name_new);

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_RENAME_OBJECT_H_ */
