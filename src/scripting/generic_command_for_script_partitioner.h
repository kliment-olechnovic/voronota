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

	GenericCommandRecord execute(const CommandInput& command_input, ScriptPartitioner& script_partitioner)
	{
		GenericCommandRecord record(command_input);

		CommandArguments cargs(record, script_partitioner);

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

		explicit CommandArguments(
				GenericCommandRecord& command_record,
				ScriptPartitioner& script_partitioner) :
						input(command_record.command_input),
						heterostorage(command_record.heterostorage),
						script_partitioner(script_partitioner)
		{
		}
	};

	virtual void run(CommandArguments&) const
	{
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_ */
