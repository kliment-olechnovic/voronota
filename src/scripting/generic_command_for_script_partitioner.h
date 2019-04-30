#ifndef SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_

#include "generic_command.h"
#include "script_partitioner.h"

namespace scripting
{

class GenericCommandForScriptPartitioner
{
public:
	struct CommandRecord : public GenericCommand::CommandRecord
	{
		ScriptPartitioner* script_partitioner_ptr;

		CommandRecord(const CommandInput& command_input, ScriptPartitioner& script_partitioner) :
			GenericCommand::CommandRecord(command_input),
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

		return record;
	}

	CommandDocumentation document() const
	{
		CommandDocumentation doc;
		document(doc);
		return doc;
	}

protected:
	struct CommandArguments : public GenericCommand::CommandArguments
	{
		ScriptPartitioner& script_partitioner;

		explicit CommandArguments(CommandRecord& command_record) :
			GenericCommand::CommandArguments(command_record),
			script_partitioner(*command_record.script_partitioner_ptr)
		{
		}
	};

	virtual void run(CommandArguments&)
	{
	}

	virtual void document(CommandDocumentation&) const
	{
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_ */
