#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_

#include "generic_command_description.h"

namespace common
{

namespace scripting
{

class GenericCommandForExtraActions
{
public:
	typedef GenericCommandDescription::CommandRecord CommandRecord;

	GenericCommandForExtraActions()
	{
	}

	virtual ~GenericCommandForExtraActions()
	{
	}

	CommandRecord execute(const CommandInput& command_input)
	{
		CommandRecord record(command_input);

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
	typedef GenericCommandDescription::CommandArguments CommandArguments;

	virtual void run(CommandArguments& /*cargs*/)
	{
	}
};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_ */

