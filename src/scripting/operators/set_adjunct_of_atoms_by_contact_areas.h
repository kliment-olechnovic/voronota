#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_CONTACT_AREAS_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_CONTACT_AREAS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctOfAtomsByContactAreas : public OperatorBase<SetAdjunctOfAtomsByContactAreas>
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
	std::string name;

	SetAdjunctOfAtomsByContactAreas() : parameters_for_selecting("[--min-seq-sep 1]")
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query("", "[--min-seq-sep 1]", input);
		name=input.get_value<std::string>("name");
		assert_adjunct_name_input(name, false);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query("", "[--min-seq-sep 1]", doc);
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "adjunct name"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(name, false);

		const std::set<std::size_t> contacts_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(contacts_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			data_manager.atom_adjuncts_mutable(i).erase(name);
		}

		std::set<std::size_t> atom_ids;

		for(std::set<std::size_t>::const_iterator it=contacts_ids.begin();it!=contacts_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];

			for(int i=0;i<(contact.solvent() ? 1 : 2);i++)
			{
				data_manager.atom_adjuncts_mutable(contact.ids[i])[name]+=contact.value.area;
				atom_ids.insert(contact.ids[i]);
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

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_CONTACT_AREAS_H_ */
