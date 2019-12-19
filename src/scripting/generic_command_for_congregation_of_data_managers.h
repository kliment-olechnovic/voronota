#ifndef SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command.h"
#include "congregation_of_data_managers.h"

namespace scripting
{

class GenericCommandForCongregationOfDataManagers : public CommonGenericCommandInterface
{
public:
	struct CommandRecord : public CommonGenericCommandRecord
	{
		CongregationOfDataManagers* congregation_of_data_managers_ptr;
		CongregationOfDataManagers::ChangeIndicator change_indicator;

		CommandRecord(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers) :
			CommonGenericCommandRecord(command_input),
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
			congregation_of_data_managers.reset_change_indicator();
			congregation_of_data_managers.reset_change_indicators_of_all_objects();
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			record.save_error(e);
		}

		record.change_indicator=congregation_of_data_managers.change_indicator();

		return record;
	}

protected:
	struct CommandArguments
	{
		CommandInput& input;
		HeterogeneousStorage& heterostorage;
		CongregationOfDataManagers& congregation_of_data_managers;

		explicit CommandArguments(CommandRecord& command_record) :
			input(command_record.command_input),
			heterostorage(command_record.heterostorage),
			congregation_of_data_managers(*command_record.congregation_of_data_managers_ptr)
		{
		}
	};

	virtual void run(CommandArguments&)
	{
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
