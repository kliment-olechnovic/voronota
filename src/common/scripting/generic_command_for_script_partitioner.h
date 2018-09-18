#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_SCRIPT_PARTITIONER_H_

#include "command_input.h"
#include "script_partitioner.h"
#include "basic_types.h"
#include "heterogeneous_storage.h"

namespace common
{

namespace scripting
{

class GenericCommandForScriptPartitioner
{
public:
	struct CommandRecord
	{
		CommandInput command_input;
		ScriptPartitioner* script_partitioner_ptr;
		bool successful;
		HeterogeneousStorage heterostorage;

		explicit CommandRecord(const CommandInput& command_input, ScriptPartitioner& script_partitioner) :
			command_input(command_input),
			script_partitioner_ptr(&script_partitioner),
			successful(false)
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

		std::ostringstream output_for_log;
		std::ostringstream output_for_errors;

		CommandArguments cargs(record.command_input, script_partitioner, output_for_log, record.heterostorage);

		try
		{
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			output_for_errors << e.what();
		}

		record.heterostorage.log+=output_for_log.str();
		record.heterostorage.error+=output_for_errors.str();

		return record;
	}

protected:
	class CommandArguments
	{
	public:
		CommandInput& input;
		ScriptPartitioner& script_partitioner;
		std::ostream& output_for_log;
		HeterogeneousStorage& heterostorage;

		CommandArguments(
				CommandInput& input,
				ScriptPartitioner& script_partitioner,
				std::ostream& output_for_log,
				HeterogeneousStorage& heterostorage) :
					input(input),
					script_partitioner(script_partitioner),
					output_for_log(output_for_log),
					heterostorage(heterostorage)
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
