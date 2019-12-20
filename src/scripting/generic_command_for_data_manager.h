#ifndef SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_

#include "generic_command.h"
#include "data_manager.h"

namespace scripting
{

class GenericCommandForDataManager : public GenericCommandInterface
{
public:
	GenericCommandForDataManager()
	{
	}

	virtual ~GenericCommandForDataManager()
	{
	}

	bool execute(GenericCommandRecord& record, DataManager& data_manager)
	{
		CommandArguments cargs(record, data_manager);

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

		return record.successful;
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

		explicit CommandArguments(
				GenericCommandRecord& command_record,
				DataManager& data_manager) :
						input(command_record.command_input),
						heterostorage(command_record.heterostorage),
						data_manager(data_manager)
		{
		}
	};

	virtual void run(CommandArguments&) const
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
