#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_H_

#include "command_input.h"
#include "heterogeneous_storage.h"

namespace common
{

namespace scripting
{

class GenericCommand
{
public:
	struct CommandRecord
	{
		bool successful;
		CommandInput command_input;
		HeterogeneousStorage heterostorage;

		explicit CommandRecord(const CommandInput& command_input) :
			successful(false),
			command_input(command_input)
		{
		}
	};

	struct CommandArguments
	{
	public:
		CommandInput& input;
		HeterogeneousStorage& heterostorage;

		explicit CommandArguments(CommandRecord& command_record) :
			input(command_record.command_input),
			heterostorage(command_record.heterostorage)
		{
		}

		void save_error(const std::exception& e)
		{
			heterostorage.errors.push_back(std::string(e.what()));
		}
	};

	GenericCommand()
	{
	}

	virtual ~GenericCommand()
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

		return record;
	}

protected:
	virtual void run(CommandArguments& /*cargs*/)
	{
	}
};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_H_ */

