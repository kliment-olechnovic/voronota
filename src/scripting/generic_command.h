#ifndef SCRIPTING_GENERIC_COMMAND_H_
#define SCRIPTING_GENERIC_COMMAND_H_

#include "command_input.h"
#include "heterogeneous_storage.h"
#include "command_documentation.h"

namespace scripting
{

class GenericCommandInterface
{
public:
	GenericCommandInterface()
	{
	}

	virtual ~GenericCommandInterface()
	{
	}

	CommandDocumentation document() const
	{
		CommandDocumentation doc;
		document(doc);
		return doc;
	}

protected:
	virtual void document(CommandDocumentation&) const
	{
	}
};

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

}

#endif /* SCRIPTING_GENERIC_COMMAND_H_ */

