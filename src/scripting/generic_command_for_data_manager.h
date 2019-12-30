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
		try
		{
			data_manager.reset_change_indicator();
			data_manager.sync_selections_with_display_states_if_requested_in_string(record.command_input.get_canonical_input_command_string());
			run(record.command_input, data_manager, record.heterostorage);
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
	virtual void run(CommandInput&, DataManager&, HeterogeneousStorage&) const
	{
	}
};

template<class Operator, bool on_multiple>
class GenericCommandForDataManagerFromOperator : public GenericCommandForDataManager
{
public:
	virtual bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
	{
		return on_multiple;
	}

protected:
	void run(CommandInput& input, DataManager& data_manager, HeterogeneousStorage& heterostorage) const
	{
		Operator op;
		op.init(input);
		input.assert_nothing_unusable();
		op.run(data_manager).write(heterostorage);
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_ */
