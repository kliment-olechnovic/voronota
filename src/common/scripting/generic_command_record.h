#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_RECORD_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_RECORD_H_

#include "command_input.h"
#include "heterogeneous_storage.h"

namespace common
{

namespace scripting
{

struct GenericCommandRecord
{
	bool successful;
	CommandInput command_input;
	HeterogeneousStorage heterostorage;

	explicit GenericCommandRecord(const CommandInput& command_input) :
		successful(false),
		command_input(command_input)
	{
	}
};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_RECORD_H_ */

