#ifndef SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_

#include "generic_command.h"
#include "data_manager.h"

namespace scripting
{

class GenericCommandForDataManager : public CommonGenericCommandInterface
{
public:
	class CommandRecord : public CommonGenericCommandRecord
	{
	public:
		CommandRecord(const CommandInput& command_input, DataManager& data_manager) :
			CommonGenericCommandRecord(command_input),
			data_manager_ptr_(&data_manager)
		{
		}

		DataManager* data_manager_ptr() const
		{
			return data_manager_ptr_;
		}

	private:
		DataManager* data_manager_ptr_;
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
			data_manager.reset_change_indicator();
			data_manager.sync_selections_with_display_states_if_requested_in_string(record.command_input.get_canonical_input_command_string());
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			record.save_error(e);
		}

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

		explicit CommandArguments(CommandRecord& command_record) :
			input(command_record.command_input),
			heterostorage(command_record.heterostorage),
			data_manager(*command_record.data_manager_ptr())
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
