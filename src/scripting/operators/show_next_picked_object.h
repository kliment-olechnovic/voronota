#ifndef SCRIPTING_OPERATORS_SHOW_NEXT_PICKED_OBJECT_H_
#define SCRIPTING_OPERATORS_SHOW_NEXT_PICKED_OBJECT_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ShowNextPickedObject : public OperatorBase<ShowNextPickedObject>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	ShowNextPickedObject()
	{
	}

	void initialize(CommandInput&)
	{
	}

	void document(CommandDocumentation&) const
	{
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		congregation_of_data_managers.set_next_picked_object_visible(false);

		Result result;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SHOW_NEXT_PICKED_OBJECT_H_ */
