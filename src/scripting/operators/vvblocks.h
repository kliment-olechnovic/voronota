#ifndef SCRIPTING_OPERATORS_VVBLOCKS_H_
#define SCRIPTING_OPERATORS_VVBLOCKS_H_

#include "../operators_common.h"
#include "../vvblocks_of_data_manager.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class VVBlocks : public OperatorBase<VVBlocks>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::size_t vvblocks_count;

		Result() : vvblocks_count(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("vvblocks_count")=vvblocks_count;
		}
	};

	int vvblock_to_display;

	VVBlocks() : vvblock_to_display(-1)
	{
	}

	void initialize(CommandInput& input)
	{
		vvblock_to_display=input.get_value_or_default<int>("vvblock-to-display", -1);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("vvblock-to-display", CDOD::DATATYPE_INT, "ID of vvblock to display"));
	}

	Result run(DataManager& data_manager) const
	{
		VVBlocksOfDataManager::Parameters params;
		VVBlocksOfDataManager::Result vvblocks_result;

		VVBlocksOfDataManager::construct_result(params, data_manager, vvblocks_result);

		if(vvblock_to_display>=0)
		{
			const std::size_t vvblock_id=static_cast<std::size_t>(vvblock_to_display);

			if(vvblock_id>=vvblocks_result.vvblocks.size())
			{
				throw std::runtime_error(std::string("Invalid provided vvblock ID to display."));
			}

			const VVBlocksOfDataManager::VVBlock& vvblock=vvblocks_result.vvblocks[vvblock_id];

			std::set<std::size_t> contact_ids_main;

			for(std::size_t i=0;i<6;i++)
			{
				VVBlocksOfDataManager::RRContactDescriptor& rr_contact_descriptor=vvblocks_result.rr_contact_descriptors[vvblock.rr_contact_descriptor_ids_main[i]];
				contact_ids_main.insert(rr_contact_descriptor.aa_contact_ids.begin(), rr_contact_descriptor.aa_contact_ids.end());
			}

			std::set<std::size_t> contact_ids_side;

			for(std::size_t i=0;i<12;i++)
			{
				const std::size_t rr_id=vvblock.rr_contact_descriptor_ids_side[i];
				if(rr_id!=VVBlocksOfDataManager::null_id())
				{
					VVBlocksOfDataManager::RRContactDescriptor& rr_contact_descriptor=vvblocks_result.rr_contact_descriptors[rr_id];
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
		result.vvblocks_count=vvblocks_result.vvblocks.size();

		return result;
	}
};

}

}

}

/*
construct-contacts-radically-fast -calculate-adjacencies -generate-graphics
vvblocks -vvblock-to-display 33
hide-atoms
show-atoms [-sel-of-contacts _visible] -full-residues -rep sticks
*/

#endif /* SCRIPTING_OPERATORS_VVBLOCKS_H_ */
