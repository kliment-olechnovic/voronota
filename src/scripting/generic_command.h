#ifndef SCRIPTING_GENERIC_COMMAND_H_
#define SCRIPTING_GENERIC_COMMAND_H_

#include "command_input.h"
#include "heterogeneous_storage.h"

namespace scripting
{

class GenericCommandRecord
{
public:
	bool successful;
	CommandInput command_input;
	HeterogeneousStorage heterostorage;

	explicit GenericCommandRecord(const CommandInput& command_input) :
		successful(false),
		command_input(command_input)
	{
	}

	virtual ~GenericCommandRecord()
	{
	}

	void save_error(const std::exception& e)
	{
		heterostorage.errors.push_back(std::string(e.what()));
	}
};

template<class Operator>
class GenericCommandWithoutSubject
{
public:
	explicit GenericCommandWithoutSubject(const Operator& op) : op_(op)
	{
	}

	virtual ~GenericCommandWithoutSubject()
	{
	}

	bool run(GenericCommandRecord& record) const
	{
		try
		{
			{
				Operator op=op_;
				op.init(record.command_input);
				record.command_input.assert_nothing_unusable();
				op.run().write(record.heterostorage);
			}
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			record.save_error(e);
		}

		return record.successful;
	}

private:
	Operator op_;
};

template<class Subject, class Operator>
class GenericCommandForSubject
{
public:
	GenericCommandForSubject(const Operator& op) : op_(op),  on_multiple_(true)
	{
	}

	GenericCommandForSubject(const Operator& op, const bool on_multiple) : op_(op),  on_multiple_(on_multiple)
	{
	}

	virtual ~GenericCommandForSubject()
	{
	}

	bool on_multiple() const
	{
		return on_multiple_;
	}

	bool run(GenericCommandRecord& record, Subject& subject) const
	{
		try
		{
			prepare(subject, record.command_input);
			{
				Operator op=op_;
				op.init(record.command_input);
				record.command_input.assert_nothing_unusable();
				op.run(subject).write(record.heterostorage);
			}
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			record.save_error(e);
		}

		return record.successful;
	}

protected:
	virtual void prepare(Subject&, CommandInput&) const
	{
	}

private:
	Operator op_;
	bool on_multiple_;
};

}

#endif /* SCRIPTING_GENERIC_COMMAND_H_ */
