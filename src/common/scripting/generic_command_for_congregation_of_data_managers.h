#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command_description.h"
#include "congregation_of_data_managers.h"

namespace common
{

namespace scripting
{

class GenericCommandForCongregationOfDataManagers
{
public:
	struct CommandRecord : public GenericCommandDescription::CommandRecord
	{
		CongregationOfDataManagers* congregation_of_data_managers_ptr;
		CongregationOfDataManagers::ChangeIndicator change_indicator;

		CommandRecord(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers) :
			GenericCommandDescription::CommandRecord(command_input),
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
			cargs.output_for_errors << e.what();
		}

		cargs.save_output_streams_data();

		record.change_indicator.ensure_correctness();

		return record;
	}

protected:
	struct CommandArguments : public GenericCommandDescription::CommandArguments
	{
		CongregationOfDataManagers& congregation_of_data_managers;
		CongregationOfDataManagers::ChangeIndicator& change_indicator;

		explicit CommandArguments(CommandRecord& command_record) :
			GenericCommandDescription::CommandArguments(command_record),
			congregation_of_data_managers(*command_record.congregation_of_data_managers_ptr),
			change_indicator(command_record.change_indicator)
		{
		}
	};

	virtual void run(CommandArguments& /*cargs*/)
	{
	}
};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
