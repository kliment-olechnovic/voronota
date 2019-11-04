#ifndef SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_

#include "generic_command.h"
#include "data_manager.h"

namespace scripting
{

class GenericCommandForDataManager : public CommonGenericCommandInterface
{
public:
	struct CommandRecord : public CommonGenericCommandRecord
	{
		DataManager* data_manager_ptr;
		DataManager::ChangeIndicator change_indicator;

		CommandRecord(const CommandInput& command_input, DataManager& data_manager) :
			CommonGenericCommandRecord(command_input),
			data_manager_ptr(&data_manager)
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

		CommandArguments cargs(record);

		try
		{
			data_manager.sync_selections_with_display_states_if_requested_in_string(record.command_input.get_canonical_input_command_string());
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			record.save_error(e);
		}

		record.change_indicator.ensure_correctness();

		return record;
	}

	virtual bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
	{
		return false;
	}

protected:
	struct CommandArguments
	{
		CommandInput& input;
		HeterogeneousStorage& heterostorage;
		DataManager& data_manager;
		DataManager::ChangeIndicator& change_indicator;

		explicit CommandArguments(CommandRecord& command_record) :
			input(command_record.command_input),
			heterostorage(command_record.heterostorage),
			data_manager(*command_record.data_manager_ptr),
			change_indicator(command_record.change_indicator)
		{
		}
	};

	virtual void run(CommandArguments&)
	{
	}
};

class GenericCommandForDataManagerScaled : public GenericCommandForDataManager
{
public:
	virtual bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
	{
		return true;
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_ */
