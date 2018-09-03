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
		std::vector<DataManager*> set_of_added_objects;
		std::vector<DataManager*> set_of_deleted_objects;

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

		record.set_of_added_objects=std::vector<DataManager*>(cargs.set_of_added_objects.begin(), cargs.set_of_added_objects.end());
		record.set_of_deleted_objects=std::vector<DataManager*>(cargs.set_of_deleted_objects.begin(), cargs.set_of_deleted_objects.end());

		return record;
	}

protected:
	class CommandArguments
	{
	public:
		CommandInput& input;
		CongregationOfDataManagers& congregation_of_data_managers;
		std::ostream& output_for_log;
		std::set<DataManager*> set_of_added_objects;
		std::set<DataManager*> set_of_deleted_objects;

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
