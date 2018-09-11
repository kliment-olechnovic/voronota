#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_SCRIPT_PARTITIONER_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_SCRIPT_PARTITIONER_H_

#include <fstream>

#include "generic_command_for_script_partitioner.h"

namespace common
{

namespace scripting
{

class CustomCommandsForScriptPartitioner
{
public:
	class set_alias : public GenericCommandForScriptPartitioner
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string>& strings=cargs.input.get_list_of_unnamed_values();

			if(strings.size()!=2)
			{
				throw std::runtime_error(std::string("Not exactly two strings provided."));
			}

			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			cargs.script_partitioner.set_alias(strings[0], strings[1]);

			cargs.output_for_log << "Set alias '" << strings[0] << "' to '" << strings[1] << "'\n";
		}
	};

	class unset_aliases : public GenericCommandForScriptPartitioner
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();

			if(names.empty())
			{
				throw std::runtime_error(std::string("No names provided."));
			}

			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			for(std::size_t i=0;i<names.size();i++)
			{
				if(cargs.script_partitioner.unset_alias(names[i]))
				{
					cargs.output_for_log << "Unset alias '" << names[i] << "'\n";
				}
			}
		}
	};

	class source : public GenericCommandForScriptPartitioner
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string>& files=cargs.input.get_list_of_unnamed_values();

			if(files.empty())
			{
				throw std::runtime_error(std::string("No files provided."));
			}

			if(files.size()!=1)
			{
				throw std::runtime_error(std::string("Not exactly one file provided."));
			}

			const std::string file=files.front();

			std::ifstream finput(file.c_str(), std::ios::in);
			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
			}

			std::istreambuf_iterator<char> eos;
			std::string script(std::istreambuf_iterator<char>(finput), eos);

			if(script.empty())
			{
				throw std::runtime_error(std::string("No script in file '")+file+"'.");
			}

			cargs.script_partitioner.add_pending_sentences_from_string_to_front(script);
		}
	};
};

}

}

#endif /* COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_SCRIPT_PARTITIONER_H_ */
