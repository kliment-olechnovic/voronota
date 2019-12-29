#ifndef SCRIPTING_OPERATORS_PRINT_TEXT_DESCRIPTION_H_
#define SCRIPTING_OPERATORS_PRINT_TEXT_DESCRIPTION_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class PrintTextDescription
{
public:
	struct Result
	{
		std::string text_description;

		Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("text_description")=text_description;
			return (*this);
		}
	};

	PrintTextDescription()
	{
	}

	PrintTextDescription& init(CommandInput&)
	{
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		Result result;
		result.text_description=data_manager.text_description();

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_PRINT_TEXT_DESCRIPTION_H_ */
