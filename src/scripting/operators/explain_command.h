#ifndef SCRIPTING_OPERATORS_EXPLAIN_COMMAND_H_
#define SCRIPTING_OPERATORS_EXPLAIN_COMMAND_H_

#include "common.h"
#include "../collection_of_command_documentations.h"

namespace scripting
{

namespace operators
{

class ExplainCommand
{
public:
	struct Result
	{
		VariantObject info;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object=info;
			return (*this);
		}
	};

	std::string name;

	ExplainCommand(CollectionOfCommandDocumentations& collection_of_command_documentations) : collection_of_command_documentations_ptr_(&collection_of_command_documentations)
	{
	}

	ExplainCommand& init(CommandInput& input)
	{
		name=input.get_value_or_first_unused_unnamed_value("name");
		return (*this);
	}

	Result run() const
	{
		if(collection_of_command_documentations_ptr_->map_of_documentations().count(name)==0)
		{
			throw std::runtime_error(std::string("Invalid command name '")+name+"'.");
		}

		const CommandDocumentation doc=collection_of_command_documentations_ptr_->get_documentation(name);

		Result result;

		result.info.value("command")=name;

		result.info.value("description")=doc.get_full_description();

		if(!doc.get_option_descriptions().empty())
		{
			std::vector<VariantObject>& output_array=result.info.objects_array("options");
			output_array.reserve(doc.get_option_descriptions().size());
			for(std::size_t i=0;i<doc.get_option_descriptions().size();i++)
			{
				const CommandDocumentation::OptionDescription& od=doc.get_option_descriptions()[i];
				VariantObject obj;
				obj.value("name")=od.name;
				obj.value("required")=od.required;
				obj.value("type")=od.value_type;
				obj.value("description")=od.description;
				output_array.push_back(obj);
			}
		}

		return result;
	}

private:
	const CollectionOfCommandDocumentations* collection_of_command_documentations_ptr_;
};

}

}

#endif /* SCRIPTING_OPERATORS_EXPLAIN_COMMAND_H_ */
