#ifndef SCRIPTING_OPERATORS_CONSTRUCT_CONTACTS_H_
#define SCRIPTING_OPERATORS_CONSTRUCT_CONTACTS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class ConstructContacts : public OperatorBase<ConstructContacts>
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

	common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation parameters_to_construct_contacts;
	common::ConstructionOfContacts::ParametersToEnhanceContacts parameters_to_enhance_contacts;
	bool force;

	ConstructContacts() : force(false)
	{
	}

	ConstructContacts& init(CommandInput& input)
	{
		parameters_to_construct_contacts=common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation();
		parameters_to_construct_contacts.probe=input.get_value_or_default<double>("probe", parameters_to_construct_contacts.probe);
		parameters_to_construct_contacts.calculate_volumes=!input.get_flag("no-calculate-volumes");
		parameters_to_construct_contacts.step=input.get_value_or_default<double>("step", parameters_to_construct_contacts.step);
		parameters_to_construct_contacts.projections=input.get_value_or_default<int>("projections", parameters_to_construct_contacts.projections);
		parameters_to_construct_contacts.sih_depth=input.get_value_or_default<int>("sih-depth", parameters_to_construct_contacts.sih_depth);
		parameters_to_enhance_contacts=common::ConstructionOfContacts::ParametersToEnhanceContacts();
		parameters_to_enhance_contacts.probe=parameters_to_construct_contacts.probe;
		parameters_to_enhance_contacts.sih_depth=parameters_to_construct_contacts.sih_depth;
		parameters_to_enhance_contacts.tag_centrality=!input.get_flag("no-tag-centrality");
		parameters_to_enhance_contacts.tag_peripherial=!input.get_flag("no-tag-peripherial");
		parameters_to_enhance_contacts.adjunct_solvent_direction=input.get_flag("adjunct-solvent-direction");
		force=input.get_flag("force");
		return (*this);
	}

	void document(CommandDocumentation& doc) const
	{
		common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation params;
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", params.probe));
		doc.set_option_decription(CDOD("no-calculate-volumes", CDOD::DATATYPE_BOOL, "flag to not calculate volumes"));
		doc.set_option_decription(CDOD("step", CDOD::DATATYPE_FLOAT, "edge step size", params.step));
		doc.set_option_decription(CDOD("projections", CDOD::DATATYPE_INT, "number of projections for edge calculation", params.projections));
		doc.set_option_decription(CDOD("sih-depth", CDOD::DATATYPE_FLOAT, "icosahedron subdivision depth for SAS calculation", params.sih_depth));
		doc.set_option_decription(CDOD("no-tag-centrality", CDOD::DATATYPE_BOOL, "flag to not add contact central tags"));
		doc.set_option_decription(CDOD("no-tag-peripherial", CDOD::DATATYPE_BOOL, "flag to not add contact peripherial tags"));
		doc.set_option_decription(CDOD("adjunct-solvent-direction", CDOD::DATATYPE_BOOL, "flag calculate SAS direction approximation"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(force)
		{
			data_manager.remove_contacts();
		}

		data_manager.reset_contacts_by_creating(parameters_to_construct_contacts, parameters_to_enhance_contacts);

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts());

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_CONSTRUCT_CONTACTS_H_ */
