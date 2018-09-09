#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_

#include "command_input.h"

namespace common
{

namespace scripting
{

class GenericCommandForExtraActions
{
public:
	struct CommandRecord
	{
		CommandInput command_input;
		bool successful;
		std::string output_log;
		std::string output_error;

		explicit CommandRecord(const CommandInput& command_input) :
			command_input(command_input),
			successful(false)
		{
		}
	};

	GenericCommandForExtraActions()
	{
	}

	virtual ~GenericCommandForExtraActions()
	{
	}

	CommandRecord execute(const CommandInput& command_input)
	{
		CommandRecord record(command_input);

		std::ostringstream output_for_log;
		std::ostringstream output_for_errors;

		CommandArguments cargs(record.command_input, output_for_log);

		try
		{
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			output_for_errors << e.what();
		}

		record.output_log=output_for_log.str();
		record.output_error=output_for_errors.str();

		return record;
	}

protected:
	class CommandArguments
	{
	public:
		CommandInput& input;
		std::ostream& output_for_log;

		CommandArguments(
				CommandInput& input,
				std::ostream& output_for_log) :
					input(input),
					output_for_log(output_for_log)
		{
		}
	};

	virtual void run(CommandArguments& /*cargs*/)
	{
	}
};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_ */

