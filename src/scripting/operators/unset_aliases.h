#ifndef SCRIPTING_OPERATORS_UNSET_ALIASES_H_
#define SCRIPTING_OPERATORS_UNSET_ALIASES_H_

#include "../operators_common.h"

#include "../script_partitioner.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class UnsetAliases : public OperatorBase<UnsetAliases>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<std::string> unset_aliases;

		void store(HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<unset_aliases.size();i++)
			{
				heterostorage.variant_object.values_array("unset_aliases").push_back(VariantValue(unset_aliases[i]));
			}
		}
	};

	std::vector<std::string> names;

	UnsetAliases()
	{
	}

	void initialize(CommandInput& input)
	{
		names=input.get_value_vector_or_all_unused_unnamed_values("names");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("names", CDOD::DATATYPE_STRING_ARRAY, "names of aliases", ""));
	}

	Result run(ScriptPartitioner& script_partitioner) const
	{
		if(names.empty())
		{
			throw std::runtime_error(std::string("No names provided."));
		}

		Result result;

		for(std::size_t i=0;i<names.size();i++)
		{
			if(script_partitioner.unset_alias(names[i]))
			{
				result.unset_aliases.push_back(names[i]);
			}
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_UNSET_ALIASES_H_ */
