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
		std::ostringstream output_for_text;

		explicit CommandArguments(CommandRecord& command_record) :
			input(command_record.command_input),
			heterostorage(command_record.heterostorage)
		{
		}

		void save_error(const std::exception& e)
		{
			heterostorage.errors.push_back(std::string(e.what()));
		}

		void save_text()
		{
			heterostorage.texts.push_back(output_for_text.str());
		}
	};

};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_DESCRIPTION_H_ */

