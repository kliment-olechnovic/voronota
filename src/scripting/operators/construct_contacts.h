#ifndef SCRIPTING_OPERATORS_CONSTRUCT_CONTACTS_H_
#define SCRIPTING_OPERATORS_CONSTRUCT_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

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

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation parameters_to_construct_contacts;
	common::ConstructionOfContacts::ParametersToEnhanceContacts parameters_to_enhance_contacts;
	std::string sas_mask_tag;
	bool force;

	ConstructContacts() : force(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_to_construct_contacts=common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation();
		parameters_to_construct_contacts.probe=input.get_value_or_default<double>("probe", parameters_to_construct_contacts.probe);
		parameters_to_construct_contacts.calculate_volumes=!input.get_flag("no-calculate-volumes");
		parameters_to_construct_contacts.calculate_bounding_arcs=input.get_flag("calculate-bounding-arcs");
		parameters_to_construct_contacts.calculate_adjacencies=input.get_flag("calculate-adjacencies");
		parameters_to_construct_contacts.step=input.get_value_or_default<double>("step", parameters_to_construct_contacts.step);
		parameters_to_construct_contacts.projections=input.get_value_or_default<int>("projections", parameters_to_construct_contacts.projections);
		parameters_to_construct_contacts.sih_depth=input.get_value_or_default<int>("sih-depth", parameters_to_construct_contacts.sih_depth);
		parameters_to_construct_contacts.skip_sas=input.get_flag("skip-sas");
		parameters_to_construct_contacts.skip_same_group=input.get_flag("skip-same-chain");
		parameters_to_enhance_contacts=common::ConstructionOfContacts::ParametersToEnhanceContacts();
		parameters_to_enhance_contacts.probe=parameters_to_construct_contacts.probe;
		parameters_to_enhance_contacts.sih_depth=parameters_to_construct_contacts.sih_depth;
		parameters_to_enhance_contacts.tag_centrality=!input.get_flag("no-tag-centrality");
		parameters_to_enhance_contacts.tag_peripherial=!input.get_flag("no-tag-peripherial");
		parameters_to_enhance_contacts.adjunct_solvent_direction=input.get_flag("adjunct-solvent-direction");
		sas_mask_tag=input.get_value_or_default<std::string>("sas-mask-tag", "");
		force=input.get_flag("force");
	}

	void document(CommandDocumentation& doc) const
	{
		common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation params;
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", params.probe));
		doc.set_option_decription(CDOD("no-calculate-volumes", CDOD::DATATYPE_BOOL, "flag to not calculate volumes"));
		doc.set_option_decription(CDOD("calculate-bounding-arcs", CDOD::DATATYPE_BOOL, "flag to calculate contact solvent bounding arcs"));
		doc.set_option_decription(CDOD("calculate-adjacencies", CDOD::DATATYPE_BOOL, "flag to calculate contact adjacencies"));
		doc.set_option_decription(CDOD("step", CDOD::DATATYPE_FLOAT, "edge step size", params.step));
		doc.set_option_decription(CDOD("projections", CDOD::DATATYPE_INT, "number of projections for edge calculation", params.projections));
		doc.set_option_decription(CDOD("sih-depth", CDOD::DATATYPE_FLOAT, "icosahedron subdivision depth for SAS calculation", params.sih_depth));
		doc.set_option_decription(CDOD("skip-sas", CDOD::DATATYPE_BOOL, "flag to skip SAS"));
		doc.set_option_decription(CDOD("skip-same-chain", CDOD::DATATYPE_BOOL, "flag to skip same chain contacts"));
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

		if(!parameters_to_construct_contacts.skip_same_group)
		{
			data_manager.reset_contacts_by_creating(parameters_to_construct_contacts, parameters_to_enhance_contacts);
		}
		else
		{
			common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation parameters_to_construct_contacts_with_lookup=parameters_to_construct_contacts;

			parameters_to_construct_contacts_with_lookup.lookup_groups.resize(data_manager.atoms().size(), 0);
			std::map<std::string, int> chain_numbers;
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				chain_numbers[data_manager.atoms()[i].crad.chainID]=0;
			}
			{
				int group=0;
				for(std::map<std::string, int>::iterator it=chain_numbers.begin();it!=chain_numbers.end();++it)
				{
					it->second=group;
					group++;
				}
			}
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				parameters_to_construct_contacts_with_lookup.lookup_groups[i]=chain_numbers[data_manager.atoms()[i].crad.chainID];
			}

			if(parameters_to_construct_contacts.skip_sas && !sas_mask_tag.empty())
			{
				parameters_to_construct_contacts_with_lookup.sas_mask.resize(data_manager.atoms().size(), 0);
				for(std::size_t i=0;i<data_manager.atoms().size();i++)
				{
					if(data_manager.atoms()[i].value.props.tags.count(sas_mask_tag)>0)
					{
						parameters_to_construct_contacts_with_lookup.sas_mask[i]=1;
					}
				}
			}

			data_manager.reset_contacts_by_creating(parameters_to_construct_contacts_with_lookup, parameters_to_enhance_contacts);
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts());

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CONSTRUCT_CONTACTS_H_ */
