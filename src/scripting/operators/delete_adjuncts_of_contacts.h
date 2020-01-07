#ifndef SCRIPTING_OPERATORS_DELETE_ADJUNCTS_OF_CONTACTS_H_
#define SCRIPTING_OPERATORS_DELETE_ADJUNCTS_OF_CONTACTS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class DeleteAdjunctsOfContacts : public OperatorBase<DeleteAdjunctsOfContacts>
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
	bool all;
	std::vector<std::string> adjuncts;

	DeleteAdjunctsOfContacts() : all(false)
	{
	}

	DeleteAdjunctsOfContacts& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		all=input.get_flag("all");
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		if(!all && adjuncts.empty())
		{
			throw std::runtime_error(std::string("No adjuncts specified."));
		}

		if(all && !adjuncts.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of adjuncts."));
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(*it);
			if(all)
			{
				contact_adjuncts.clear();
			}
			else
			{
				for(std::size_t i=0;i<adjuncts.size();i++)
				{
					contact_adjuncts.erase(adjuncts[i]);
				}
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_ADJUNCTS_OF_CONTACTS_H_ */
