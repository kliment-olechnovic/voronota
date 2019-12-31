#ifndef SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_

#include "generic_command.h"
#include "script_partitioner.h"

namespace scripting
{

class GenericCommandForScriptPartitioner
{
public:
	GenericCommandForScriptPartitioner()
	{
	}

	virtual ~GenericCommandForScriptPartitioner()
	{
	}

	virtual bool execute(GenericCommandRecord&, ScriptPartitioner&) const = 0;
};

template<class Operator>
class GenericCommandForScriptPartitionerFromOperator : public GenericCommandForScriptPartitioner, public GenericCommandForSubject<ScriptPartitioner, Operator>
{
public:
	explicit GenericCommandForScriptPartitionerFromOperator(const Operator& op) : GenericCommandForSubject<ScriptPartitioner, Operator>(op)
	{
	}

	bool execute(GenericCommandRecord& record, ScriptPartitioner& script_partitioner) const
	{
		return GenericCommandForSubject<ScriptPartitioner, Operator>::run(record, script_partitioner);
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_ */
