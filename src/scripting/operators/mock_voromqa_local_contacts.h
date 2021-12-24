#ifndef SCRIPTING_OPERATORS_MOCK_VOROMQA_LOCAL_CONTACTS_H_
#define SCRIPTING_OPERATORS_MOCK_VOROMQA_LOCAL_CONTACTS_H_

#include "../operators_common.h"
#include "../scoring_of_data_manager_using_mock_voromqa.h"
#include "../primitive_chemistry_annotation.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class MockVoroMQALocalContacts : public OperatorBase<MockVoroMQALocalContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::size_t contacts_selected;
		double area;
		double pseudo_energy;

		Result() :
			contacts_selected(0),
			area(0.0),
			pseudo_energy(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("contacts_selected")=contacts_selected;
			heterostorage.variant_object.value("area")=area;
			heterostorage.variant_object.value("pseudo_energy")=pseudo_energy;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string adjunct_contact_energy;

	MockVoroMQALocalContacts()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		adjunct_contact_energy=input.get_value<std::string>("adj-contact-energy");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("adj-contact-energy", CDOD::DATATYPE_STRING, "name of output adjunct for raw energy values"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(adjunct_contact_energy, false);

		if(!ScoringOfDataManagerUsingMockVoroMQA::Configuration::get_default_configuration().valid())
		{
			throw std::runtime_error(std::string("Invalid mock VoroMQA configuration."));
		}

		const std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		Result result;
		result.contacts_selected=contact_ids.size();

		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const std::size_t contact_id=(*it);
			const Contact& contact=data_manager.contacts()[contact_id];
			const double potential_value=ScoringOfDataManagerUsingMockVoroMQA::get_potential_value(data_manager, contact);
			const double pseudo_energy_value=(contact.value.area*potential_value);
			std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(contact_id);
			contact_adjuncts[adjunct_contact_energy]=pseudo_energy_value;
			result.area+=contact.value.area;
			result.pseudo_energy+=pseudo_energy_value;
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_MOCK_VOROMQA_LOCAL_CONTACTS_H_ */
