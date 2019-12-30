#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class SetAdjunctOfContacts
{
public:
	struct Result
	{
		SummaryOfContacts contacts_summary;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string name;
	bool value_present;
	double value;
	bool remove;

	SetAdjunctOfContacts() : value_present(false), value(0.0), remove(false)
	{
	}

	SetAdjunctOfContacts& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		name=input.get_value<std::string>("name");
		value_present=input.is_option("value");
		value=input.get_value_or_default<double>("value", 0.0);
		remove=input.get_flag("remove");
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		if(value_present && remove)
		{
			throw std::runtime_error(std::string("Value setting and removing options used together."));
		}

		assert_adjunct_name_input(name, false);

		std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(*it);
			if(remove)
			{
				contact_adjuncts.erase(name);
			}
			else
			{
				contact_adjuncts[name]=value;
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_H_ */
