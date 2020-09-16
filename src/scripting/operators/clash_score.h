#ifndef SCRIPTING_OPERATORS_CLASH_SCORE_H_
#define SCRIPTING_OPERATORS_CLASH_SCORE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ClashScore : public OperatorBase<ClashScore>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;
		std::size_t relevant_atoms;
		std::size_t clashed_atoms;
		double clash_score;

		Result() : relevant_atoms(0), clashed_atoms(0), clash_score(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			heterostorage.variant_object.value("relevant_atoms")=relevant_atoms;
			heterostorage.variant_object.value("clashed_atoms")=clashed_atoms;
			heterostorage.variant_object.value("clash_score")=clash_score;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	double distance;
	std::string global_adj_prefix;

	ClashScore() : distance(2.5)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		distance=input.get_value_or_default<double>("distance", 2.5);
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "clash");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("distance", CDOD::DATATYPE_FLOAT, "threshold of distance between atomic centers", ""));
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", "clash"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::set<std::size_t> relevant_atom_ids;
		std::set<std::size_t> clashed_atom_ids;

		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			if(!contact.solvent())
			{
				relevant_atom_ids.insert(contact.ids[0]);
				relevant_atom_ids.insert(contact.ids[1]);
				if(contact.value.dist<=distance)
				{
					clashed_atom_ids.insert(contact.ids[0]);
					clashed_atom_ids.insert(contact.ids[1]);
				}
			}
		}

		const double clash_score=static_cast<double>(clashed_atom_ids.size())/static_cast<double>(relevant_atom_ids.size());

		if(!global_adj_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_score"]=clash_score;
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), contact_ids);
		result.relevant_atoms=relevant_atom_ids.size();
		result.clashed_atoms=clashed_atom_ids.size();
		result.clash_score=clash_score;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CLASH_SCORE_H_ */
