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
		bool changed_objects;
		bool changed_objects_names;
		bool changed_objects_picks;
		bool changed_objects_visibilities;
		std::string output_log;
		std::string output_error;
		std::vector<DataManager*> set_of_added_objects;
		std::vector<DataManager*> set_of_deleted_objects;
		SummaryOfAtoms summary_of_atoms;
		std::map<std::string, VariantValue> extra_values;

		explicit CommandRecord(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers) :
			command_input(command_input),
			congregation_of_data_managers(&congregation_of_data_managers),
			successful(false),
			changed_objects(false),
			changed_objects_names(false),
			changed_objects_picks(false),
			changed_objects_visibilities(false)
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

		record.summary_of_atoms=cargs.summary_of_atoms;

		record.extra_values.swap(cargs.extra_values);

		record.changed_objects=(cargs.changed_objects || !record.set_of_added_objects.empty() || !record.set_of_deleted_objects.empty());
		record.changed_objects_names=(cargs.changed_objects_names || record.changed_objects);
		record.changed_objects_picks=(cargs.changed_objects_picks || record.changed_objects);
		record.changed_objects_visibilities=(cargs.changed_objects_visibilities || record.changed_objects);

		return record;
	}

protected:
	class CommandArguments
	{
	public:
		CommandInput& input;
		CongregationOfDataManagers& congregation_of_data_managers;
		std::ostream& output_for_log;
		bool changed_objects;
		bool changed_objects_names;
		bool changed_objects_picks;
		bool changed_objects_visibilities;
		std::set<DataManager*> set_of_added_objects;
		std::set<DataManager*> set_of_deleted_objects;
		SummaryOfAtoms summary_of_atoms;
		std::map<std::string, VariantValue> extra_values;

		CommandArguments(
				CommandInput& input,
				CongregationOfDataManagers& congregation_of_data_managers,
				std::ostream& output_for_log) :
					input(input),
					congregation_of_data_managers(congregation_of_data_managers),
					output_for_log(output_for_log),
					changed_objects(false),
					changed_objects_names(false),
					changed_objects_picks(false),
					changed_objects_visibilities(false)
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
