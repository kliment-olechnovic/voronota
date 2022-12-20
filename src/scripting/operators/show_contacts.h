#ifndef SCRIPTING_OPERATORS_SHOW_CONTACTS_H_
#define SCRIPTING_OPERATORS_SHOW_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ShowContacts : public OperatorBase<ShowContacts>
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
	std::vector<std::string> representation_names;

	ShowContacts()
	{
	}

	virtual ~ShowContacts()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("rep", CDOD::DATATYPE_STRING_ARRAY, "representation names", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_representations_availability();

		std::set<std::size_t> representation_ids=data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

		if(positive() && representation_ids.empty() && data_manager.contacts_representation_descriptor().names.size()>1)
		{
			representation_ids.insert(0);
		}

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		if(positive())
		{
			common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts;
			parameters_to_draw_contacts.probe=data_manager.history_of_actions_on_contacts().probe();
			parameters_to_draw_contacts.enable_alt=(representation_ids.count(data_manager.contacts_representation_descriptor().id_by_name("skin-shape"))>0);
			data_manager.reset_contacts_graphics_by_creating(parameters_to_draw_contacts, ids, true);
		}

		data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_show(positive()).set_hide(!positive()), ids);

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}

protected:
	virtual bool positive() const
	{
		return true;
	}
};

class HideContacts : public ShowContacts
{
protected:
	bool positive() const
	{
		return false;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SHOW_CONTACTS_H_ */
