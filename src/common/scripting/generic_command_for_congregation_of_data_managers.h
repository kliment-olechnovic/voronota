#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command.h"
#include "congregation_of_data_managers.h"

namespace common
{

namespace scripting
{

class GenericCommandForCongregationOfDataManagers
{
public:
	struct CommandRecord : public GenericCommand::CommandRecord
	{
		CongregationOfDataManagers* congregation_of_data_managers_ptr;
		CongregationOfDataManagers::ChangeIndicator change_indicator;

		CommandRecord(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers) :
			GenericCommand::CommandRecord(command_input),
			congregation_of_data_managers_ptr(&congregation_of_data_managers)
		{
		}
	};

	GenericCommandForCongregationOfDataManagers()
	{
	}

	virtual ~GenericCommandForCongregationOfDataManagers()
	{
	}

	CommandRecord execute(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers)
	{
		CommandRecord record(command_input, congregation_of_data_managers);

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

		record.change_indicator.ensure_correctness();

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
		CongregationOfDataManagers& congregation_of_data_managers;
		CongregationOfDataManagers::ChangeIndicator& change_indicator;

		explicit CommandArguments(CommandRecord& command_record) :
			GenericCommand::CommandArguments(command_record),
			congregation_of_data_managers(*command_record.congregation_of_data_managers_ptr),
			change_indicator(command_record.change_indicator)
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

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
