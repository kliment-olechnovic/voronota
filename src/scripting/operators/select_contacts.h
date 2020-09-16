#ifndef SCRIPTING_OPERATORS_SELECT_CONTACTS_H_
#define SCRIPTING_OPERATORS_SELECT_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SelectContacts : public OperatorBase<SelectContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;
		std::string selection_name;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			if(selection_name.empty())
			{
				heterostorage.variant_object.value("selection_name").set_null();
			}
			else
			{
				heterostorage.variant_object.value("selection_name")=selection_name;
			}
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string name;
	bool mark;

	SelectContacts() : mark(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		name=input.get_value_or_first_unused_unnamed_value_or_default("name", "");
		mark=input.get_flag("mark");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "contact selection name", ""));
		doc.set_option_decription(CDOD("mark", CDOD::DATATYPE_BOOL, "flag to mark selected contacts"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_selection_name_input(name, true);

		std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		if(!name.empty())
		{
			data_manager.selection_manager().set_contacts_selection(name, ids);
		}

		if(mark)
		{
			data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_unmark(true));
			data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_mark(true), ids);
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);
		result.selection_name=name;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SELECT_CONTACTS_H_ */
