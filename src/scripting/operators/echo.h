#ifndef SCRIPTING_OPERATORS_ECHO_H_
#define SCRIPTING_OPERATORS_ECHO_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class Echo : public OperatorBase<Echo>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<std::string> strings;

		void store(HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<strings.size();i++)
			{
				heterostorage.variant_object.values_array("lines").push_back(VariantValue(strings[i]));
			}
		}
	};

	std::vector<std::string> strings;

	Echo()
	{
	}

	void initialize(CommandInput& input)
	{
		strings=input.get_value_vector_or_all_unused_unnamed_values("strings");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("strings", CDOD::DATATYPE_STRING_ARRAY, "strings to output", ""));
	}

	Result run(void*) const
	{
		Result result;
		result.strings=strings;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ECHO_H_ */
