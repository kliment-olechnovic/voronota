#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_

#include "generic_command_record.h"

namespace common
{

namespace scripting
{

class GenericCommandForExtraActions
{
public:
	typedef GenericCommandRecord CommandRecord;

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

		CommandArguments cargs(record.command_input, output_for_log, record.heterostorage);

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
		std::ostream& output_for_log;
		HeterogeneousStorage& heterostorage;

		CommandArguments(
				CommandInput& input,
				std::ostream& output_for_log,
				HeterogeneousStorage& heterostorage) :
					input(input),
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

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_ */

