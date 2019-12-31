#ifndef SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command.h"
#include "congregation_of_data_managers.h"

namespace scripting
{

class GenericCommandForCongregationOfDataManagers
{
public:
	GenericCommandForCongregationOfDataManagers()
	{
	}

	virtual ~GenericCommandForCongregationOfDataManagers()
	{
	}

	virtual bool execute(GenericCommandRecord&, CongregationOfDataManagers&) const = 0;
};

template<class Operator>
class GenericCommandForCongregationOfDataManagersFromOperator : public GenericCommandForCongregationOfDataManagers, public GenericCommandForSubject<CongregationOfDataManagers, Operator>
{
public:
	explicit GenericCommandForCongregationOfDataManagersFromOperator(const Operator& op) : GenericCommandForSubject<CongregationOfDataManagers, Operator>(op)
	{
	}

	bool execute(GenericCommandRecord& record, CongregationOfDataManagers& congregation_of_data_managers) const
	{
		return GenericCommandForSubject<CongregationOfDataManagers, Operator>::run(record, congregation_of_data_managers);
	}

protected:
	void prepare(CongregationOfDataManagers& congregation_of_data_managers, CommandInput&) const
	{
		congregation_of_data_managers.reset_change_indicator();
		congregation_of_data_managers.reset_change_indicators_of_all_objects();
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
