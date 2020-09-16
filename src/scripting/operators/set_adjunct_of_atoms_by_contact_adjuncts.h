#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_CONTACT_ADJUNCTS_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_CONTACT_ADJUNCTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctOfAtomsByContactAdjuncts : public OperatorBase<SetAdjunctOfAtomsByContactAdjuncts>
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

	SelectionManager::Query parameters_for_selecting;
	std::string source_name;
	std::string destination_name;
	std::string pooling_mode;

	SetAdjunctOfAtomsByContactAdjuncts()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		source_name=input.get_value<std::string>("source-name");
		destination_name=input.get_value<std::string>("destination-name");
		pooling_mode=input.get_value<std::string>("pooling-mode");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("source-name", CDOD::DATATYPE_STRING, "source adjunct name"));
		doc.set_option_decription(CDOD("destination-name", CDOD::DATATYPE_STRING, "destination adjunct name"));
		doc.set_option_decription(CDOD("pooling-mode", CDOD::DATATYPE_STRING, "pooling mode, possible values: sum, min, max"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(source_name, false);
		assert_adjunct_name_input(destination_name, false);

		if(pooling_mode!="sum" && pooling_mode!="min" && pooling_mode!="max")
		{
			throw std::runtime_error(std::string("Invalid pooling mode, valid options are: 'sum', 'min', 'max'."));
		}

		const std::set<std::size_t> contacts_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(contacts_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			data_manager.atom_adjuncts_mutable(i).erase(destination_name);
		}

		std::set<std::size_t> atom_ids;

		for(std::set<std::size_t>::const_iterator it=contacts_ids.begin();it!=contacts_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];

			if(contact.value.props.adjuncts.count(source_name)>0)
			{
				const double source_value=(contact.value.props.adjuncts.find(source_name)->second);
				for(int i=0;i<(contact.solvent() ? 1 : 2);i++)
				{
					std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(contact.ids[i]);
					const bool first_setting=(atom_adjuncts.count(destination_name)==0);
					double& destination_value=atom_adjuncts[destination_name];
					if(pooling_mode=="sum")
					{
						destination_value+=source_value;
					}
					else if(pooling_mode=="min")
					{
						destination_value=(first_setting ? source_value : std::min(source_value, destination_value));
					}
					else if(pooling_mode=="max")
					{
						destination_value=(first_setting ? source_value : std::max(source_value, destination_value));
					}
					atom_ids.insert(contact.ids[i]);
				}
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), contacts_ids);
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), atom_ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_CONTACT_ADJUNCTS_H_ */
