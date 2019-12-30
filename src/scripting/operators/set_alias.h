#ifndef SCRIPTING_OPERATORS_SET_ALIAS_H_
#define SCRIPTING_OPERATORS_SET_ALIAS_H_

#include "common.h"

#include "../script_partitioner.h"

namespace scripting
{

namespace operators
{

class SetAlias
{
public:
	struct Result
	{
		std::string alias;
		std::string script;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("alias")=alias;
			heterostorage.variant_object.value("script")=script;
			return (*this);
		}
	};

	std::string name;
	std::string script_template;

	SetAlias()
	{
	}

	SetAlias& init(CommandInput& input)
	{
		const std::vector<std::string>& strings=input.get_list_of_unnamed_values();
		input.mark_all_unnamed_values_as_used();
		if(strings.size()!=2)
		{
			throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
		}
		name=strings[0];
		script_template=strings[1];
		return (*this);
	}

	Result run(ScriptPartitioner& script_partitioner) const
	{
		if(name.empty())
		{
			throw std::runtime_error(std::string("Empty alias name."));
		}

		if(script_template.empty())
		{
			throw std::runtime_error(std::string("Empty script template."));
		}

		if(name==script_template)
		{
			throw std::runtime_error(std::string("Alias name is the same as the script template."));
		}

		script_partitioner.set_alias(name, script_template);

		Result result;
		result.alias=name;
		result.script=script_template;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SET_ALIAS_H_ */
