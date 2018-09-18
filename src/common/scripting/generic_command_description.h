#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_DESCRIPTION_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_DESCRIPTION_H_

#include "command_input.h"
#include "heterogeneous_storage.h"

namespace common
{

namespace scripting
{

class GenericCommandDescription
{
public:
	struct CommandRecord
	{
		bool successful;
		CommandInput command_input;
		HeterogeneousStorage heterostorage;

		explicit CommandRecord(const CommandInput& command_input) :
			successful(false),
			command_input(command_input)
		{
		}
	};

	struct CommandArguments
	{
	public:
		CommandInput& input;
		HeterogeneousStorage& heterostorage;
		std::ostringstream output_for_log;
		std::ostringstream output_for_errors;
		std::ostringstream output_for_text;

		explicit CommandArguments(CommandRecord& command_record) :
			input(command_record.command_input),
			heterostorage(command_record.heterostorage)
		{
		}

		void save_output_streams_data()
		{
			heterostorage.log+=output_for_log.str();
			heterostorage.error+=output_for_errors.str();
			heterostorage.text+=output_for_text.str();
		}
	};

};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_DESCRIPTION_H_ */

