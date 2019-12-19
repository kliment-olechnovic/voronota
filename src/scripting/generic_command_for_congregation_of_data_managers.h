#ifndef SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command.h"
#include "congregation_of_data_managers.h"

namespace scripting
{

class GenericCommandForCongregationOfDataManagers : public CommonGenericCommandInterface
{
public:
	class CommandRecord : public CommonGenericCommandRecord
	{
	public:
		CommandRecord(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers) :
			CommonGenericCommandRecord(command_input),
			congregation_of_data_managers_ptr_(&congregation_of_data_managers)
		{
		}

		CongregationOfDataManagers* congregation_of_data_managers_ptr() const
		{
			return congregation_of_data_managers_ptr_;
		}

	private:
		CongregationOfDataManagers* congregation_of_data_managers_ptr_;
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
			congregation_of_data_managers(*command_record.congregation_of_data_managers_ptr())
		{
		}
	};

	virtual void run(CommandArguments&)
	{
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
