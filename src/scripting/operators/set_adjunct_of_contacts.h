#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctOfContacts : public OperatorBase<SetAdjunctOfContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string name;
	bool value_present;
	double value;
	bool remove;
	bool multiply_by_area;
	std::vector<std::string> residue_residue_values;

	SetAdjunctOfContacts() : value_present(false), value(0.0), remove(false), multiply_by_area(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		name=input.get_value<std::string>("name");
		value_present=input.is_option("value");
		value=input.get_value_or_default<double>("value", 0.0);
		remove=input.get_flag("remove");
		multiply_by_area=input.get_flag("multiply-by-area");
		residue_residue_values=input.get_value_vector_or_default<std::string>("residue-residue-values", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "adjunct name"));
		doc.set_option_decription(CDOD("value", CDOD::DATATYPE_FLOAT, "adjunct value", ""));
		doc.set_option_decription(CDOD("remove", CDOD::DATATYPE_BOOL, "flag to remove adjunct"));
		doc.set_option_decription(CDOD("multiply-by-area", CDOD::DATATYPE_BOOL, "flag to assign value multiplied by area"));
		doc.set_option_decription(CDOD("residue-residue-values", CDOD::DATATYPE_STRING_ARRAY, "residue-residue IDs and values", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		if(value_present && remove)
		{
			throw std::runtime_error(std::string("Value setting and removing options used together."));
		}

		if(!value_present && multiply_by_area)
		{
			throw std::runtime_error(std::string("Area multiplication option used without providing a value."));
		}

		if(!residue_residue_values.empty() && residue_residue_values.size()%5!=0)
		{
			throw std::runtime_error(std::string("The length of the vector of residue-residue IDs and values must be a multiple of 5 (chain rnum chain rnum value)."));
		}

		assert_adjunct_name_input(name, false);

		std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		if(residue_residue_values.empty())
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(*it);
				if(remove)
				{
					contact_adjuncts.erase(name);
				}
				else
				{
					if(multiply_by_area)
					{
						contact_adjuncts[name]=value*data_manager.contacts()[*it].value.area;
					}
					else
					{
						contact_adjuncts[name]=value;
					}
				}
			}
		}
		else
		{
			std::map<common::ChainResidueAtomDescriptorsPair, double> rr_adjunct_values;
			for(std::size_t j=0;j<residue_residue_values.size();j+=5)
			{
				common::ChainResidueAtomDescriptor a(residue_residue_values[j+0]);
				common::ChainResidueAtomDescriptor b(residue_residue_values[j+2]);
				double rr_value=value;
				std::string line=residue_residue_values[j+1]+" "+residue_residue_values[j+3]+" "+residue_residue_values[j+4];
				std::istringstream line_input(line);
				line_input >> a.resSeq >> b.resSeq >> rr_value;
				if(line_input.fail())
				{
					throw std::runtime_error(std::string("Invalid vector of residue-residue IDs and values."));
				}
				else
				{
					rr_adjunct_values[common::ChainResidueAtomDescriptorsPair(a, b)]=rr_value;
				}
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Contact& contact=data_manager.contacts()[*it];
				if(!contact.solvent())
				{
					const Atom& atom1=data_manager.atoms()[contact.ids[0]];
					common::ChainResidueAtomDescriptor crad1(atom1.crad.chainID);
					crad1.resSeq=atom1.crad.resSeq;
					const Atom& atom2=data_manager.atoms()[contact.ids[1]];
					common::ChainResidueAtomDescriptor crad2(atom2.crad.chainID);
					crad2.resSeq=atom2.crad.resSeq;
					const common::ChainResidueAtomDescriptorsPair crads(crad1, crad2);
					std::map<common::ChainResidueAtomDescriptorsPair, double>::const_iterator jt=rr_adjunct_values.find(crads);
					if(jt!=rr_adjunct_values.end())
					{
						const double rr_value=jt->second;
						std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(*it);
						if(remove)
						{
							contact_adjuncts.erase(name);
						}
						else
						{
							if(multiply_by_area)
							{
								contact_adjuncts[name]=rr_value*contact.value.area;
							}
							else
							{
								contact_adjuncts[name]=rr_value;
							}
						}
					}
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

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_H_ */
