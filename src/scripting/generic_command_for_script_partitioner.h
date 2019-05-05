#ifndef SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_

#include "generic_command.h"
#include "script_partitioner.h"

namespace scripting
{

class GenericCommandForScriptPartitioner : public CommonGenericCommandInterface
{
public:
	struct CommandRecord : public CommonGenericCommandRecord
	{
		ScriptPartitioner* script_partitioner_ptr;

		CommandRecord(const CommandInput& command_input, ScriptPartitioner& script_partitioner) :
			CommonGenericCommandRecord(command_input),
			script_partitioner_ptr(&script_partitioner)
		{
		}
	};

	GenericCommandForScriptPartitioner()
	{
	}

	virtual ~GenericCommandForScriptPartitioner()
	{
	}

	CommandRecord execute(const CommandInput& command_input, ScriptPartitioner& script_partitioner)
	{
		CommandRecord record(command_input, script_partitioner);

		CommandArguments cargs(record);

		try
		{
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			record.save_error(e);
		}

		return record;
	}

protected:
	struct CommandArguments
	{
		CommandInput& input;
		HeterogeneousStorage& heterostorage;
		ScriptPartitioner& script_partitioner;

		explicit CommandArguments(CommandRecord& command_record) :
			input(command_record.command_input),
			heterostorage(command_record.heterostorage),
			script_partitioner(*command_record.script_partitioner_ptr)
		{
		}
	};

	virtual void run(CommandArguments&)
	{
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_ */
