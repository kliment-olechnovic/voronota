#ifndef SCRIPTING_OPERATORS_SHOW_CONTACTS_H_
#define SCRIPTING_OPERATORS_SHOW_CONTACTS_H_

#include "../operators_common.h"

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

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			return (*this);
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

	ShowContacts& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		return (*this);
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

		const std::set<std::size_t> ids=data_manager.filter_contacts_drawable_implemented_ids(
				representation_ids,
				data_manager.selection_manager().select_contacts(parameters_for_selecting),
				false);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable contacts selected."));
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

#endif /* SCRIPTING_OPERATORS_SHOW_CONTACTS_H_ */
