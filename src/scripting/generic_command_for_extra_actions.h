#ifndef SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_
#define SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_

#include "generic_command.h"

namespace scripting
{

class GenericCommandForExtraActions : public GenericCommandInterface
{
public:
	GenericCommandForExtraActions()
	{
	}

	virtual ~GenericCommandForExtraActions()
	{
	}

	bool execute(GenericCommandRecord& record)
	{
		try
		{
			run(record.command_input, record.heterostorage);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			record.save_error(e);
		}

		return record.successful;
	}

protected:
	virtual void run(CommandInput&, HeterogeneousStorage&) const
	{
	}
};

template<class Operator>
class GenericCommandForExtraActionsFromOperator : public GenericCommandForExtraActions
{
public:
	explicit GenericCommandForExtraActionsFromOperator(const Operator& op) : op_(op)
	{
	}

protected:
	void run(CommandInput& input, HeterogeneousStorage& heterostorage) const
	{
		Operator op=op_;
		op.init(input);
		input.assert_nothing_unusable();
		op.run().write(heterostorage);
	}

private:
	Operator op_;
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_FOR_EXTRA_ACTIONS_H_ */
