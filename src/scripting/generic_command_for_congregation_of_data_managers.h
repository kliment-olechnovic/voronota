#ifndef SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command.h"
#include "congregation_of_data_managers.h"

namespace scripting
{

class GenericCommandForCongregationOfDataManagers : public CommonGenericCommandInterface
{
public:
	GenericCommandForCongregationOfDataManagers()
	{
	}

	virtual ~GenericCommandForCongregationOfDataManagers()
	{
	}

	CommonGenericCommandRecord execute(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers)
	{
		CommonGenericCommandRecord record(command_input);

		CommandArguments cargs(record, congregation_of_data_managers);

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

		explicit CommandArguments(
				CommonGenericCommandRecord& command_record,
				CongregationOfDataManagers& congregation_of_data_managers) :
						input(command_record.command_input),
						heterostorage(command_record.heterostorage),
						congregation_of_data_managers(congregation_of_data_managers)
		{
		}
	};

	virtual void run(CommandArguments&)
	{
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
