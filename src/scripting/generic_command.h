#ifndef SCRIPTING_GENERIC_COMMAND_H_
#define SCRIPTING_GENERIC_COMMAND_H_

#include "command_input.h"
#include "heterogeneous_storage.h"
#include "command_documentation.h"

namespace scripting
{

class CommonGenericCommandInterface
{
public:
	CommonGenericCommandInterface()
	{
	}

	virtual ~CommonGenericCommandInterface()
	{
	}

	CommandDocumentation document() const
	{
		CommandDocumentation doc;
		document(doc);
		return doc;
	}

protected:
	virtual void document(CommandDocumentation&) const
	{
	}
};

class CommonGenericCommandRecord
{
public:
	bool successful;
	CommandInput command_input;
	HeterogeneousStorage heterostorage;

	explicit CommonGenericCommandRecord(const CommandInput& command_input) :
		successful(false),
		command_input(command_input)
	{
	}

	virtual ~CommonGenericCommandRecord()
	{
	}

	void save_error(const std::exception& e)
	{
		heterostorage.errors.push_back(std::string(e.what()));
	}
};

class GenericCommand : public CommonGenericCommandInterface
{
public:
	struct CommandRecord : public CommonGenericCommandRecord
	{
		explicit CommandRecord(const CommandInput& command_input) :
			CommonGenericCommandRecord(command_input)
		{
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
			record.save_error(e);
		}

		return record;
	}

protected:
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
	};

	virtual void run(CommandArguments&)
	{
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_H_ */

