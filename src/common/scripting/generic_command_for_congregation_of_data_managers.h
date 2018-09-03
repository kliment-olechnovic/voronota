#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "command_input.h"
#include "congregation_of_data_managers.h"

namespace common
{

namespace scripting
{

class GenericCommandForCongregationOfDataManagers
{
public:
	struct CommandRecord
	{
		CommandInput command_input;
		CongregationOfDataManagers* congregation_of_data_managers;
		bool successful;
		std::string output_log;
		std::string output_error;

		explicit CommandRecord(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers) :
			command_input(command_input),
			congregation_of_data_managers(&congregation_of_data_managers),
			successful(false)
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

		std::ostringstream output_for_log;
		std::ostringstream output_for_errors;

		CommandArguments cargs(record.command_input, congregation_of_data_managers, output_for_log);

		try
		{
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			output_for_errors << e.what();
		}

		record.output_log=output_for_log.str();
		record.output_error=output_for_errors.str();

		return record;
	}

protected:
	class CommandArguments
	{
	public:
		CommandInput& input;
		CongregationOfDataManagers& congregation_of_data_managers;
		std::ostream& output_for_log;

		CommandArguments(
				CommandInput& input,
				CongregationOfDataManagers& congregation_of_data_managers,
				std::ostream& output_for_log) :
					input(input),
					congregation_of_data_managers(congregation_of_data_managers),
					output_for_log(output_for_log)
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
