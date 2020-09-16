#ifndef SCRIPTING_OPERATORS_PICK_OBJECTS_H_
#define SCRIPTING_OPERATORS_PICK_OBJECTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class PickObjects : public OperatorBase<PickObjects>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	CongregationOfDataManagers::ObjectQuery query;

	PickObjects()
	{
	}

	virtual ~PickObjects()
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

		if(positive() && !add())
		{
			congregation_of_data_managers.set_all_objects_picked(false);
		}

		for(std::size_t i=0;i<objects.size();i++)
		{
			congregation_of_data_managers.set_object_picked(objects[i], positive());
		}

		Result result;

		return result;
	}

protected:
	virtual bool positive() const
	{
		return true;
	}

	virtual bool add() const
	{
		return false;
	}
};

class PickMoreObjects : public PickObjects
{
protected:
	bool add() const
	{
		return true;
	}
};

class UnpickObjects : public PickObjects
{
protected:
	bool positive() const
	{
		return false;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_PICK_OBJECTS_H_ */
