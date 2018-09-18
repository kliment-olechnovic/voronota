#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_

#include "command_input.h"
#include "data_manager.h"
#include "change_indicator_for_data_manager.h"
#include "heterogeneous_storage.h"

namespace common
{

namespace scripting
{

class GenericCommandForDataManager
{
public:
	struct CommandRecord
	{
		CommandInput command_input;
		DataManager* data_manager_ptr;
		bool successful;
		ChangeIndicatorForDataManager change_indicator;
		HeterogeneousStorage heterostorage;

		explicit CommandRecord(const CommandInput& command_input, DataManager& data_manager) :
			command_input(command_input),
			data_manager_ptr(&data_manager),
			successful(false)
		{
		}
	};

	GenericCommandForDataManager()
	{
	}

	virtual ~GenericCommandForDataManager()
	{
	}

	CommandRecord execute(const CommandInput& command_input, DataManager& data_manager)
	{
		CommandRecord record(command_input, data_manager);

		std::ostringstream output_for_log;
		std::ostringstream output_for_errors;
		std::ostringstream output_for_text;

		CommandArguments cargs(record.command_input, data_manager, record.change_indicator, output_for_log, output_for_text, record.heterostorage);

		try
		{
			data_manager.sync_selections_with_display_states_if_requested_in_string(record.command_input.get_canonical_input_command_string());
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			output_for_errors << e.what();
		}

		record.heterostorage.log+=output_for_log.str();
		record.heterostorage.error+=output_for_errors.str();
		record.heterostorage.text+=output_for_text.str();

		record.change_indicator.ensure_correctness();

		return record;
	}

	virtual bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
	{
		return false;
	}

protected:
	class CommandArguments
	{
	public:
		CommandInput& input;
		DataManager& data_manager;
		ChangeIndicatorForDataManager& change_indicator;
		std::ostream& output_for_log;
		std::ostream& output_for_text;
		HeterogeneousStorage& heterostorage;

		CommandArguments(
				CommandInput& input,
				DataManager& data_manager,
				ChangeIndicatorForDataManager& change_indicator,
				std::ostream& output_for_log,
				std::ostream& output_for_text,
				HeterogeneousStorage& heterostorage) :
					input(input),
					data_manager(data_manager),
					change_indicator(change_indicator),
					output_for_log(output_for_log),
					output_for_text(output_for_text),
					heterostorage(heterostorage)
		{
		}
	};

	virtual void run(CommandArguments& /*cargs*/)
	{
	}
};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_ */
