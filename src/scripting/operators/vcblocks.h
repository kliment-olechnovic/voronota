#ifndef SCRIPTING_OPERATORS_VCBLOCKS_H_
#define SCRIPTING_OPERATORS_VCBLOCKS_H_

#include "../operators_common.h"
#include "../vcblocks_of_data_manager.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class VCBlocks : public OperatorBase<VCBlocks>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::size_t vcblocks_count;

		Result() : vcblocks_count(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("vcblocks_count")=vcblocks_count;
		}
	};

	VCBlocksOfDataManager::Parameters construction_parameters;
	std::string selection_for_display;

	VCBlocks()
	{
	}

	void initialize(CommandInput& input)
	{
		construction_parameters.with_parasiding=input.get_flag("with-parasiding");
		construction_parameters.with_paracapping=input.get_flag("with-paracapping");
		selection_for_display=input.get_value_or_default<std::string>("sel-for-display", "");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("with-parasiding", CDOD::DATATYPE_BOOL, "falg to include parasiding contacts"));
		doc.set_option_decription(CDOD("with-paracapping", CDOD::DATATYPE_BOOL, "falg to include paracapping contacts"));
		doc.set_option_decription(CDOD("sel-for-display", CDOD::DATATYPE_STRING, "selection expression for contacts to display"));
	}

	Result run(DataManager& data_manager) const
	{
		VCBlocksOfDataManager::Result vcblocks_result;

		VCBlocksOfDataManager::construct_result(construction_parameters, data_manager, vcblocks_result);

		if(!selection_for_display.empty())
		{
			const std::set<std::size_t> selected_contact_ids=data_manager.selection_manager().select_contacts(SelectionManager::Query(selection_for_display, false));

			if(selected_contact_ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected for display."));
			}

			std::set<std::size_t> vcblock_ids_for_display;

			for(std::set<std::size_t>::const_iterator it=selected_contact_ids.begin();it!=selected_contact_ids.end();++it)
			{
				const std::size_t vcblock_id=vcblocks_result.map_of_aa_contact_ids_to_rr_contact_descriptors[*it];
				if(vcblock_id!=VCBlocksOfDataManager::null_id())
				{
					vcblock_ids_for_display.insert(vcblock_id);
				}
			}

			if(vcblock_ids_for_display.empty())
			{
				throw std::runtime_error(std::string("No vcblocks selected for display."));
			}

			std::set<std::size_t> contact_ids_main;
			std::set<std::size_t> contact_ids_side;

			for(std::set<std::size_t>::const_iterator it=vcblock_ids_for_display.begin();it!=vcblock_ids_for_display.end();++it)
			{
				const VCBlocksOfDataManager::VCBlock& vcblock=vcblocks_result.vcblocks[*it];

				{
					VCBlocksOfDataManager::RRContactDescriptor& rr_contact_descriptor=vcblocks_result.rr_contact_descriptors[vcblock.rr_contact_descriptor_id_main];
					contact_ids_main.insert(rr_contact_descriptor.aa_contact_ids.begin(), rr_contact_descriptor.aa_contact_ids.end());
				}

				for(int j=0;j<2;j++)
				{
					for(std::size_t i=0;i<vcblock.rr_contact_descriptor_ids_surrounding[j].size();i++)
					{
						VCBlocksOfDataManager::RRContactDescriptor& rr_contact_descriptor=vcblocks_result.rr_contact_descriptors[vcblock.rr_contact_descriptor_ids_surrounding[j][i]];
						contact_ids_main.insert(rr_contact_descriptor.aa_contact_ids.begin(), rr_contact_descriptor.aa_contact_ids.end());
					}
				}

				for(int j=0;j<2;j++)
				{
					for(std::size_t i=0;i<vcblock.rr_contact_descriptor_ids_capping[j].size();i++)
					{
						VCBlocksOfDataManager::RRContactDescriptor& rr_contact_descriptor=vcblocks_result.rr_contact_descriptors[vcblock.rr_contact_descriptor_ids_capping[j][i]];
						contact_ids_side.insert(rr_contact_descriptor.aa_contact_ids.begin(), rr_contact_descriptor.aa_contact_ids.end());
					}
				}

				for(std::size_t i=0;i<vcblock.rr_contact_descriptor_ids_parasiding.size();i++)
				{
					for(std::size_t j=0;j<vcblock.rr_contact_descriptor_ids_parasiding[i].size();j++)
					{
						VCBlocksOfDataManager::RRContactDescriptor& rr_contact_descriptor=vcblocks_result.rr_contact_descriptors[vcblock.rr_contact_descriptor_ids_parasiding[i][j]];
						contact_ids_side.insert(rr_contact_descriptor.aa_contact_ids.begin(), rr_contact_descriptor.aa_contact_ids.end());
					}
				}

				for(std::size_t i=0;i<vcblock.rr_contact_descriptor_ids_paracapping.size();i++)
				{
					for(std::size_t j=0;j<vcblock.rr_contact_descriptor_ids_paracapping[i].size();j++)
					{
						VCBlocksOfDataManager::RRContactDescriptor& rr_contact_descriptor=vcblocks_result.rr_contact_descriptors[vcblock.rr_contact_descriptor_ids_paracapping[i][j]];
						contact_ids_side.insert(rr_contact_descriptor.aa_contact_ids.begin(), rr_contact_descriptor.aa_contact_ids.end());
					}
				}
			}

			{
				std::set<std::size_t> contact_ids_all=contact_ids_main;
				contact_ids_all.insert(contact_ids_side.begin(), contact_ids_side.end());
				common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts;
				parameters_to_draw_contacts.probe=data_manager.history_of_actions_on_contacts().probe();
				data_manager.reset_contacts_graphics_by_creating(parameters_to_draw_contacts, contact_ids_all, true);
			}

			{
				std::set<std::size_t> representation_ids=data_manager.contacts_representation_descriptor().ids_by_names(std::vector<std::string>(1, std::string("faces")));
				data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_show(true), contact_ids_main);
			}

			if(!contact_ids_side.empty())
			{
				std::set<std::size_t> representation_ids=data_manager.contacts_representation_descriptor().ids_by_names(std::vector<std::string>(1, std::string("edges")));
				data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_show(true), contact_ids_side);
			}
		}

		Result result;
		result.vcblocks_count=vcblocks_result.vcblocks.size();

		return result;
	}
};

}

}

}

/*
construct-contacts-radically-fast -calculate-adjacencies -generate-graphics
vcblocks -sel-for-display [-a1 [-chain A -rnum 4] -a2 [-chain A -rnum 22]]
hide-atoms
show-atoms [-sel-of-contacts _visible] -full-residues -rep sticks
color-contacts -col black -rep edges
spectrum-contacts -by residue-ids -rep faces -scheme rygwbwbcgyr
zoom-by-atoms [-sel-of-contacts _visible]
*/

#endif /* SCRIPTING_OPERATORS_VCBLOCKS_H_ */
