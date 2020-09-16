#ifndef SCRIPTING_OPERATORS_PRINT_CONTACTS_H_
#define SCRIPTING_OPERATORS_PRINT_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class PrintContacts : public OperatorBase<PrintContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<VariantObject> contacts;
		std::size_t number_of_inter_residue_contacts;
		SummaryOfContacts contacts_summary;

		Result() : number_of_inter_residue_contacts(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.objects_array("contacts")=contacts;
			if(number_of_inter_residue_contacts>0)
			{
				heterostorage.variant_object.value("number_of_inter_residue_contacts")=number_of_inter_residue_contacts;
			}
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	bool inter_residue;

	PrintContacts() : inter_residue(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		inter_residue=input.get_flag("inter-residue");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("inter-residue", CDOD::DATATYPE_BOOL, "flag to print inter-residue contacts"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		const std::vector<Atom>& atoms=data_manager.atoms();

		Result result;

		if(inter_residue)
		{
			std::map<common::ChainResidueAtomDescriptorsPair, common::ContactValue> map_for_output;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Contact& contact=data_manager.contacts()[*it];
				if(contact.solvent())
				{
					map_for_output[common::ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad.without_atom(), common::ChainResidueAtomDescriptor::solvent())].add(contact.value);
				}
				else
				{
					map_for_output[common::ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad.without_atom(), atoms[contact.ids[1]].crad.without_atom())].add(contact.value);
				}
			}
			result.contacts.reserve(map_for_output.size());
			for(std::map<common::ChainResidueAtomDescriptorsPair, common::ContactValue>::const_iterator it=map_for_output.begin();it!=map_for_output.end();++it)
			{
				result.contacts.push_back(VariantObject());
				VariantSerialization::write(it->first, it->second, result.contacts.back());
			}
			result.number_of_inter_residue_contacts=map_for_output.size();
		}
		else
		{
			result.contacts.reserve(ids.size());
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				result.contacts.push_back(VariantObject());
				VariantSerialization::write(atoms, data_manager.contacts()[*it], result.contacts.back());
				result.contacts.back().value("id")=(*it);
			}
		}

		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_PRINT_CONTACTS_H_ */
