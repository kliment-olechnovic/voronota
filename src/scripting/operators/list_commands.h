#ifndef SCRIPTING_OPERATORS_LIST_COMMANDS_H_
#define SCRIPTING_OPERATORS_LIST_COMMANDS_H_

#include "common.h"
#include "../command_documentation.h"

namespace scripting
{

namespace operators
{

class ListCommands
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

	bool compact;

	explicit ListCommands(CollectionOfCommandDocumentations& collection_of_command_documentations) : compact(false), collection_of_command_documentations_ptr_(&collection_of_command_documentations)
	{
	}

	ListCommands& init(CommandInput& input)
	{
		compact=input.get_flag("compact");
		return (*this);
	}

	Result run() const
	{
		const std::vector<std::string> names=collection_of_command_documentations_ptr_->get_all_names();

		if(names.empty())
		{
			throw std::runtime_error(std::string("No commands documented."));
		}

		Result result;

		if(compact)
		{
			std::vector<VariantValue>& values_array=result.info.values_array("names");

			for(std::size_t i=0;i<names.size();i++)
			{
				values_array.push_back(VariantValue(names[i]));
			}
		}
		else
		{
			std::vector<VariantObject>& output_array=result.info.objects_array("outlines");

			for(std::size_t i=0;i<names.size();i++)
			{
				const CommandDocumentation doc=collection_of_command_documentations_ptr_->get_documentation(names[i]);
				VariantObject obj;
				obj.value("name")=names[i];
				obj.value("short_description")=doc.get_short_description();
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

#endif /* SCRIPTING_OPERATORS_LIST_COMMANDS_H_ */
