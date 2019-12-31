#ifndef SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_

#include "generic_command.h"

namespace scripting
{

class GenericCommandForExtraActions
{
public:
	GenericCommandForExtraActions()
	{
	}

	virtual ~GenericCommandForExtraActions()
	{
	}

	virtual bool execute(GenericCommandRecord&) = 0;
};

template<class Operator>
class GenericCommandForExtraActionsFromOperator : public GenericCommandForExtraActions, public GenericCommandWithoutSubject<Operator>
{
public:
	explicit GenericCommandForExtraActionsFromOperator(const Operator& op) : GenericCommandWithoutSubject<Operator>(op)
	{
	}

	bool execute(GenericCommandRecord& record)
	{
		return GenericCommandWithoutSubject<Operator>::run(record);
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_ */
