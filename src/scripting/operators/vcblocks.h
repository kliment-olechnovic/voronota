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

	int vcblock_to_display;

	VCBlocks() : vcblock_to_display(-1)
	{
	}

	void initialize(CommandInput& input)
	{
		vcblock_to_display=input.get_value_or_default<int>("vcblock-to-display", -1);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("vcblock-to-display", CDOD::DATATYPE_INT, "ID of vcblock to display"));
	}

	Result run(DataManager& data_manager) const
	{
		VCBlocksOfDataManager::Parameters params;
		VCBlocksOfDataManager::Result vcblocks_result;

		VCBlocksOfDataManager::construct_result(params, data_manager, vcblocks_result);

		if(vcblock_to_display>=0)
		{
			const std::size_t vcblock_id=static_cast<std::size_t>(vcblock_to_display);

			if(vcblock_id>=vcblocks_result.vcblocks.size())
			{
				throw std::runtime_error(std::string("Invalid provided vcblock ID to display."));
			}

			const VCBlocksOfDataManager::VCBlock& vcblock=vcblocks_result.vcblocks[vcblock_id];

			std::set<std::size_t> contact_ids_main;

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

			std::set<std::size_t> contact_ids_side;

			for(int j=0;j<2;j++)
			{
				for(std::size_t i=0;i<vcblock.rr_contact_descriptor_ids_capping[j].size();i++)
				{
					VCBlocksOfDataManager::RRContactDescriptor& rr_contact_descriptor=vcblocks_result.rr_contact_descriptors[vcblock.rr_contact_descriptor_ids_capping[j][i]];
					contact_ids_side.insert(rr_contact_descriptor.aa_contact_ids.begin(), rr_contact_descriptor.aa_contact_ids.end());
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
vcblocks -vcblock-to-display 30
hide-atoms
show-atoms [-sel-of-contacts _visible] -full-residues -rep sticks
*/

#endif /* SCRIPTING_OPERATORS_VCBLOCKS_H_ */
