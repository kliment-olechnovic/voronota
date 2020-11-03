#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_BY_ATOM_ADJUNCTS_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_BY_ATOM_ADJUNCTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctOfContactsByAtomAdjuncts : public OperatorBase<SetAdjunctOfContactsByAtomAdjuncts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	std::string selection_expresion_for_atoms;
	std::string selection_expresion_for_contacts;
	std::string source_name;
	std::string destination_name;
	std::string pooling_mode;

	SetAdjunctOfContactsByAtomAdjuncts()
	{
	}

	void initialize(CommandInput& input)
	{
		selection_expresion_for_atoms=input.get_value<std::string>("atoms");
		selection_expresion_for_contacts=input.get_value<std::string>("contacts");
		source_name=input.get_value<std::string>("source-name");
		destination_name=input.get_value<std::string>("destination-name");
		pooling_mode=input.get_value<std::string>("pooling-mode");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("atoms", CDOD::DATATYPE_STRING, "selection expression for atoms"));
		doc.set_option_decription(CDOD("contacts", CDOD::DATATYPE_STRING, "selection expression for contacts"));
		doc.set_option_decription(CDOD("source-name", CDOD::DATATYPE_STRING, "source adjunct name"));
		doc.set_option_decription(CDOD("destination-name", CDOD::DATATYPE_STRING, "destination adjunct name"));
		doc.set_option_decription(CDOD("pooling-mode", CDOD::DATATYPE_STRING, "pooling mode, possible values: min, max, sum"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(source_name, false);
		assert_adjunct_name_input(destination_name, false);

		if(pooling_mode!="min" && pooling_mode!="max" && pooling_mode!="sum")
		{
			throw std::runtime_error(std::string("Invalid pooling mode, valid options are: 'min', 'max', 'sum'."));
		}

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(SelectionManager::Query(selection_expresion_for_atoms, false));
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(SelectionManager::Query(selection_expresion_for_contacts, false));
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::set<std::size_t> used_atom_ids;

		for(std::set<std::size_t>::const_iterator contact_id_it=contact_ids.begin();contact_id_it!=contact_ids.end();++contact_id_it)
		{
			const std::size_t contact_id=(*contact_id_it);
			const Contact& contact=data_manager.contacts()[contact_id];
			std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(contact_id);
			contact_adjuncts.erase(destination_name);
			bool first_setting=true;
			for(int i=0;i<(contact.solvent() ? 1 : 2);i++)
			{
				const std::size_t atom_id=contact.ids[i];
				if(atom_ids.count(atom_id)>0 && data_manager.atoms()[atom_id].value.props.adjuncts.count(source_name)>0)
				{
					const double source_value=data_manager.atoms()[atom_id].value.props.adjuncts.find(source_name)->second;
					double& destination_value=contact_adjuncts[destination_name];
					if(pooling_mode=="min")
					{
						destination_value=(first_setting ? source_value : std::min(source_value, destination_value));
					}
					else if(pooling_mode=="max")
					{
						destination_value=(first_setting ? source_value : std::max(source_value, destination_value));
					}
					else if(pooling_mode=="sum")
					{
						destination_value+=source_value;
					}
					first_setting=false;
					used_atom_ids.insert(atom_id);
				}
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), contact_ids);
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), used_atom_ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_BY_ATOM_ADJUNCTS_H_ */
