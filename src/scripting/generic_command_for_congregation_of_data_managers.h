#ifndef SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command.h"
#include "congregation_of_data_managers.h"

namespace scripting
{

class GenericCommandForCongregationOfDataManagers : public GenericCommandInterface
{
public:
	GenericCommandForCongregationOfDataManagers()
	{
	}

	virtual ~GenericCommandForCongregationOfDataManagers()
	{
	}

	bool execute(GenericCommandRecord& record, CongregationOfDataManagers& congregation_of_data_managers)
	{
		try
		{
			run(record.command_input, congregation_of_data_managers, record.heterostorage);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			record.save_error(e);
		}

		return record.successful;
	}

protected:
	virtual void run(CommandInput&, CongregationOfDataManagers&, HeterogeneousStorage&) const
	{
	}
};

template<class Operator>
class GenericCommandForCongregationOfDataManagersFromOperator : public GenericCommandForCongregationOfDataManagers
{
public:
	explicit GenericCommandForCongregationOfDataManagersFromOperator(const Operator& op) : op_(op)
	{
	}

protected:
	void run(CommandInput& input, CongregationOfDataManagers& congregation_of_data_managers, HeterogeneousStorage& heterostorage) const
	{
		congregation_of_data_managers.reset_change_indicator();
		congregation_of_data_managers.reset_change_indicators_of_all_objects();
		Operator op=op_;
		op.init(input);
		input.assert_nothing_unusable();
		op.run(congregation_of_data_managers).write(heterostorage);
	}

private:
	Operator op_;
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
