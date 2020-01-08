#ifndef SCRIPTING_OPERATORS_SET_TEXT_DESCRIPTION_H_
#define SCRIPTING_OPERATORS_SET_TEXT_DESCRIPTION_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class SetTextDescription : public OperatorBase<SetTextDescription>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string text_description;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("text_description")=text_description;
		}
	};

	std::string text;

	SetTextDescription()
	{
	}

	void initialize(CommandInput& input)
	{
		text=input.get_value_or_first_unused_unnamed_value("text");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("text", CDOD::DATATYPE_STRING, "any text"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.set_text_description(text);

		Result result;
		result.text_description=data_manager.text_description();

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SET_TEXT_DESCRIPTION_H_ */
