#ifndef SCRIPTING_OPERATORS_SET_TAG_OF_CONTACTS_H_
#define SCRIPTING_OPERATORS_SET_TAG_OF_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetTagOfContacts : public OperatorBase<SetTagOfContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string tag;

	SetTagOfContacts()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		tag=input.get_value_or_first_unused_unnamed_value("tag");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("tag", CDOD::DATATYPE_STRING, "tag name"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_tag_input(tag, false);

		std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			data_manager.contact_tags_mutable(*it).insert(tag);
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_TAG_OF_CONTACTS_H_ */
