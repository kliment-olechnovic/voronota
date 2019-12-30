#ifndef SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_

#include "generic_command.h"
#include "script_partitioner.h"

namespace scripting
{

class GenericCommandForScriptPartitioner : public GenericCommandInterface
{
public:
	GenericCommandForScriptPartitioner()
	{
	}

	virtual ~GenericCommandForScriptPartitioner()
	{
	}

	bool execute(GenericCommandRecord& record, ScriptPartitioner& script_partitioner)
	{
		try
		{
			run(record.command_input, script_partitioner, record.heterostorage);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			record.save_error(e);
		}

		return record.successful;
	}

protected:
	virtual void run(CommandInput&, ScriptPartitioner&, HeterogeneousStorage&) const
	{
	}
};

template<class Operator>
class GenericCommandForScriptPartitionerFromOperator : public GenericCommandForScriptPartitioner
{
public:
	GenericCommandForScriptPartitionerFromOperator(const Operator& op) : op_(op)
	{
	}

protected:
	void run(CommandInput& input, ScriptPartitioner& script_partitioner, HeterogeneousStorage& heterostorage) const
	{
		Operator op=op_;
		op.init(input);
		input.assert_nothing_unusable();
		op.run(script_partitioner).write(heterostorage);
	}

private:
	Operator op_;
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_ */
