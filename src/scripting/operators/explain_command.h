#ifndef SCRIPTING_OPERATORS_EXPLAIN_COMMAND_H_
#define SCRIPTING_OPERATORS_EXPLAIN_COMMAND_H_

#include "../operators_common.h"
#include "../command_documentation.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExplainCommand : public OperatorBase<ExplainCommand>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		VariantObject info;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object=info;
		}
	};

	std::string name;

	explicit ExplainCommand(CollectionOfCommandDocumentations& collection_of_command_documentations) : collection_of_command_documentations_ptr_(&collection_of_command_documentations)
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value_or_first_unused_unnamed_value("name");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "name of a command"));
	}

	Result run(void*) const
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
				obj.value("type")=od.data_type_string();
				obj.value("description")=od.description;
				if(!od.default_value.empty())
				{
					obj.value("default")=od.default_value;
				}
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

}

#endif /* SCRIPTING_OPERATORS_EXPLAIN_COMMAND_H_ */
