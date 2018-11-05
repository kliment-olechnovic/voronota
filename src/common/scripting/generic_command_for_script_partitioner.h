#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_

#include "generic_command_description.h"
#include "script_partitioner.h"

namespace common
{

namespace scripting
{

class GenericCommandForScriptPartitioner
{
public:
	struct CommandRecord : public GenericCommandDescription::CommandRecord
	{
		ScriptPartitioner* script_partitioner_ptr;

		CommandRecord(const CommandInput& command_input, ScriptPartitioner& script_partitioner) :
			GenericCommandDescription::CommandRecord(command_input),
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
			cargs.save_error(e);
		}

		cargs.save_text();

		return record;
	}

protected:
	struct CommandArguments : public GenericCommandDescription::CommandArguments
	{
		ScriptPartitioner& script_partitioner;

		explicit CommandArguments(CommandRecord& command_record) :
			GenericCommandDescription::CommandArguments(command_record),
			script_partitioner(*command_record.script_partitioner_ptr)
		{
		}
	};

	virtual void run(CommandArguments& /*cargs*/)
	{
	}
};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_ */
