#ifndef SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_

#include "generic_command.h"
#include "data_manager.h"

namespace scripting
{

class GenericCommandForDataManager
{
public:
	GenericCommandForDataManager()
	{
	}

	virtual ~GenericCommandForDataManager()
	{
	}

	virtual bool execute(GenericCommandRecord&, DataManager&) const = 0;

	virtual bool multiplicable() const = 0;
};

template<class Operator>
class GenericCommandForDataManagerFromOperator : public GenericCommandForDataManager, public GenericCommandForSubject<DataManager, Operator>
{
public:
	GenericCommandForDataManagerFromOperator(const Operator& op, const bool on_multiple) : GenericCommandForSubject<DataManager, Operator>(op, on_multiple)
	{
	}

	bool execute(GenericCommandRecord& record, DataManager& data_manager) const
	{
		return GenericCommandForSubject<DataManager, Operator>::run(record, data_manager);
	}

	bool multiplicable() const
	{
		return GenericCommandForSubject<DataManager, Operator>::on_multiple();
	}

private:
	void prepare(DataManager& data_manager, CommandInput& input) const
	{
		data_manager.reset_change_indicator();
		data_manager.sync_selections_with_display_states_if_requested_in_string(input.get_canonical_input_command_string());
	}
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_ */
