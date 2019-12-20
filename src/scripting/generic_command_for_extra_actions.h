#ifndef SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_

#include "generic_command.h"

namespace scripting
{

class GenericCommandForExtraActions : public GenericCommandInterface
{
public:
	GenericCommandForExtraActions()
	{
	}

	virtual ~GenericCommandForExtraActions()
	{
	}

	bool execute(GenericCommandRecord& record)
	{
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

		return record.successful;
	}

protected:
	struct CommandArguments
	{
	public:
		CommandInput& input;
		HeterogeneousStorage& heterostorage;

		explicit CommandArguments(GenericCommandRecord& command_record) :
			input(command_record.command_input),
			heterostorage(command_record.heterostorage)
		{
		}
	};

	virtual void run(CommandArguments&) const
	{
	}
};
}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_ */
