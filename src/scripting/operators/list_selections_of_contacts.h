#ifndef SCRIPTING_OPERATORS_LIST_SELECTIONS_OF_CONTACTS_H_
#define SCRIPTING_OPERATORS_LIST_SELECTIONS_OF_CONTACTS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class ListSelectionsOfContacts
{
public:
	struct Result
	{
		std::map<std::string, SummaryOfContacts> selections_summaries;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			for(std::map<std::string, SummaryOfContacts>::const_iterator it=selections_summaries.begin();it!=selections_summaries.end();++it)
			{
				VariantObject info;
				info.value("name")=(it->first);
				VariantSerialization::write(it->second, info.object("contacts_summary"));
				heterostorage.variant_object.objects_array("selections").push_back(info);
			}
			return (*this);
		}
	};

		ListSelectionsOfContacts()
	{
	}

	ListSelectionsOfContacts& init(CommandInput&)
	{
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.sync_contacts_selections_with_display_states();
		data_manager.assert_contacts_selections_availability();

		const std::map< std::string, std::set<std::size_t> >& map_of_selections=data_manager.selection_manager().map_of_contacts_selections();

		Result result;

		for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
		{
			result.selections_summaries[it->first]=SummaryOfContacts(data_manager.contacts(), it->second);
		}

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_LIST_SELECTIONS_OF_CONTACTS_H_ */
