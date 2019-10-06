#ifndef SCRIPTING_CUSTOM_COMMANDS_FOR_SCRIPT_PARTITIONER_H_
#define SCRIPTING_CUSTOM_COMMANDS_FOR_SCRIPT_PARTITIONER_H_

#include "generic_command_for_script_partitioner.h"
#include "io_selectors.h"

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

			VariantObject& info=cargs.heterostorage.variant_object;
			info.value("alias")=strings[0];
			info.value("script")=strings[1];
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
					cargs.heterostorage.variant_object.values_array("unset_aliases").push_back(VariantValue(names[i]));
				}
			}
		}
	};

	class source : public GenericCommandForScriptPartitioner
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");

			if(file.empty())
			{
				throw std::runtime_error(std::string("File not provided."));
			}

			cargs.input.assert_nothing_unusable();

			InputSelector finput_selector(file);
			std::istream& finput=finput_selector.stream();

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

#endif /* SCRIPTING_CUSTOM_COMMANDS_FOR_SCRIPT_PARTITIONER_H_ */
