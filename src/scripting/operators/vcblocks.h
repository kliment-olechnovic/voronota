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
		std::size_t total_count_of_vcblocks;
		std::size_t total_count_of_recorded_vcblocks;
		std::size_t total_count_of_surrounding_residues;

		Result() : total_count_of_vcblocks(0), total_count_of_recorded_vcblocks(0), total_count_of_surrounding_residues(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("total_count_of_vcblocks")=total_count_of_vcblocks;
			heterostorage.variant_object.value("total_count_of_recorded_vcblocks")=total_count_of_recorded_vcblocks;
			heterostorage.variant_object.value("total_count_of_surrounding_residues")=total_count_of_surrounding_residues;
			heterostorage.variant_object.value("avg_count_of_surrounding_residues")=static_cast<double>(total_count_of_surrounding_residues)/static_cast<double>(total_count_of_recorded_vcblocks);
		}
	};

	VCBlocksOfDataManager::Parameters construction_parameters;
	std::string input_standardizer_means;
	std::string input_standardizer_sds;
	std::string selection_for_display;
	std::string output_table;
	std::string output_id;
	bool log_to_stderr;

	VCBlocks() : log_to_stderr(false)
	{
	}

	void initialize(CommandInput& input)
	{
		VCBlocksOfDataManager::Parameters dafault_construction_parameters;
		construction_parameters.selection_of_contacts_for_recording_blocks=input.get_value_or_default<std::string>("sel-for-recording", dafault_construction_parameters.selection_of_contacts_for_recording_blocks);
		construction_parameters.names_of_raw_values_describing_residues=input.get_value_vector_or_default<std::string>("residue-value-names-for-encoding", dafault_construction_parameters.names_of_raw_values_describing_residues);
		construction_parameters.names_of_raw_values_describing_rr_contacts_far=input.get_value_vector_or_default<std::string>("far-contact-value-names-for-encoding", dafault_construction_parameters.names_of_raw_values_describing_rr_contacts_far);
		construction_parameters.names_of_raw_values_describing_rr_contacts_near=input.get_value_vector_or_default<std::string>("near-contact-value-names-for-encoding", dafault_construction_parameters.names_of_raw_values_describing_rr_contacts_near);
		input_standardizer_means=input.get_value_or_default<std::string>("input-standardizer-means", "");
		input_standardizer_sds=input.get_value_or_default<std::string>("input-standardizer-sds", "");
		selection_for_display=input.get_value_or_default<std::string>("sel-for-display", "");
		output_table=input.get_value_or_default<std::string>("output-table", "");
		output_id=input.get_value_or_default<std::string>("output-id", "");
		log_to_stderr=input.get_flag("log-to-stderr");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("sel-for-recording", CDOD::DATATYPE_STRING, "selection expression for contacts for recording", VCBlocksOfDataManager::Parameters().selection_of_contacts_for_recording_blocks));
		doc.set_option_decription(CDOD("residue-value-names-for-encoding", CDOD::DATATYPE_STRING_ARRAY, "list of names of residue values to encode", ""));
		doc.set_option_decription(CDOD("far-contact-value-names-for-encoding", CDOD::DATATYPE_STRING_ARRAY, "list of names of  values to encode for far residue-residue contacts", ""));
		doc.set_option_decription(CDOD("near-contact-value-names-for-encoding", CDOD::DATATYPE_STRING_ARRAY, "list of names of  values to encode for near residue-residue contacts", ""));
		doc.set_option_decription(CDOD("input-standardizer-means", CDOD::DATATYPE_STRING, "file path to input mean values to standardize encodings", ""));
		doc.set_option_decription(CDOD("input-standardizer-sds", CDOD::DATATYPE_STRING, "file path to input sd values to standardize encodings", ""));
		doc.set_option_decription(CDOD("sel-for-display", CDOD::DATATYPE_STRING, "selection expression for contacts to display", ""));
		doc.set_option_decription(CDOD("output-table", CDOD::DATATYPE_STRING, "file path to output results table", ""));
		doc.set_option_decription(CDOD("output-id", CDOD::DATATYPE_STRING, "string identifier to annotate the output table rows", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_primary_structure_info_valid();
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_adjacencies_availability();

		const bool use_standardizer=(!input_standardizer_means.empty() || !input_standardizer_sds.empty());

		VCBlocksOfDataManager::Result vcblocks_result;

		if(use_standardizer)
		{
			if(!VCBlocksOfDataManager::Standardizer::setup_default_standardizer(input_standardizer_means, input_standardizer_sds))
			{
				throw std::runtime_error(std::string("Failed to setup standardizer for vcblocks."));
			}
		}

		VCBlocksOfDataManager::construct_result(construction_parameters, data_manager, vcblocks_result);

		if(!output_table.empty())
		{
			OutputSelector output_selector(output_table);
			std::ostream& output=output_selector.stream();
			assert_io_stream(output_table, output);

			{
				output << "ID chain1 seqnum1 resname1 chain2 seqnum2 resname2";
				const std::vector<std::string>& final_encoding_header=vcblocks_result.header_for_vcblock_encodings;
				for(std::size_t j=0;j<final_encoding_header.size();j++)
				{
					output << " " << final_encoding_header[j];
				}
				output << "\n";
			}

			for(std::vector<std::size_t>::const_iterator vcblock_id_it=vcblocks_result.indices_of_recorded_vcblocks.begin();vcblock_id_it!=vcblocks_result.indices_of_recorded_vcblocks.end();++vcblock_id_it)
			{
				const VCBlocksOfDataManager::VCBlock& vcblock=vcblocks_result.vcblocks[*vcblock_id_it];
				const common::ChainResidueAtomDescriptor& crad1=data_manager.primary_structure_info().residues[vcblock.residue_id_main[0]].chain_residue_descriptor;
				const common::ChainResidueAtomDescriptor& crad2=data_manager.primary_structure_info().residues[vcblock.residue_id_main[1]].chain_residue_descriptor;
				output << (output_id.empty() ? std::string(".") : output_id);
				output << " " << (crad1.chainID.empty() ? std::string(".") : crad1.chainID) << " " << crad1.resSeq << " " << (crad1.resName.empty() ? std::string(".") : crad1.resName);
				output << " " << (crad2.chainID.empty() ? std::string(".") : crad2.chainID) << " " << crad2.resSeq << " " << (crad2.resName.empty() ? std::string(".") : crad2.resName);
				const std::vector<double>& final_encoding=(vcblock.standardized_encoding.empty() ? vcblock.full_encoding : vcblock.standardized_encoding);
				for(std::size_t j=0;j<final_encoding.size();j++)
				{
					output << " " << final_encoding[j];
				}
				output << "\n";
			}
		}

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
				if(vcblock_id!=VCBlocksOfDataManager::null_id() && vcblocks_result.vcblocks[vcblock_id].recorded)
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

				for(std::size_t i=0;i<vcblock.rr_contact_descriptor_ids_paracapping.size();i++)
				{
					for(std::size_t j=0;j<vcblock.rr_contact_descriptor_ids_paracapping[i].size();j++)
					{
						VCBlocksOfDataManager::RRContactDescriptor& rr_contact_descriptor=vcblocks_result.rr_contact_descriptors[vcblock.rr_contact_descriptor_ids_paracapping[i][j]];
						contact_ids_side.insert(rr_contact_descriptor.aa_contact_ids.begin(), rr_contact_descriptor.aa_contact_ids.end());
					}
				}

				if(log_to_stderr)
				{
					std::cerr << data_manager.primary_structure_info().residues[vcblock.residue_id_main[0]].chain_residue_descriptor << " ";
					std::cerr << data_manager.primary_structure_info().residues[vcblock.residue_id_main[1]].chain_residue_descriptor << " ";
					std::cerr << vcblock.residue_ids_surrounding.size() << ":\n";
					for(std::size_t j=0;j<vcblock.residue_ids_surrounding.size();j++)
					{
						std::cerr << " (" << data_manager.primary_structure_info().residues[vcblock.residue_ids_surrounding[j]].chain_residue_descriptor << "=";
						std::cerr << vcblock.angles_of_surrounding_residues[j] << ")";
					}
					std::cerr << "\n";
					for(std::size_t j=0;j<vcblock.residue_ids_surrounding.size();j++)
					{
						std::cerr << " (" << data_manager.primary_structure_info().residues[vcblock.residue_ids_surrounding[j]].chain_residue_descriptor << "=";
						std::cerr << vcblock.adjacency_lengths_of_surrounding_residues[j] << ")";
					}
					std::cerr << "\n";
					{
						std::cerr << " encoding_header_length=" << vcblocks_result.header_for_vcblock_encodings.size();
						std::cerr << " encoding_vector_length=" << vcblock.full_encoding.size();
					}
					std::cerr << "\n";
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
		result.total_count_of_vcblocks=vcblocks_result.vcblocks.size();
		result.total_count_of_recorded_vcblocks=vcblocks_result.indices_of_recorded_vcblocks.size();
		for(std::size_t i=0;i<vcblocks_result.indices_of_recorded_vcblocks.size();i++)
		{
			result.total_count_of_surrounding_residues+=vcblocks_result.vcblocks[vcblocks_result.indices_of_recorded_vcblocks[i]].residue_ids_surrounding.size();
		}

		return result;
	}
};

}

}

}

/*
setup-akbps-layered -potentials ../tmp/akbps_layered_lib -summing-weights ../tmp/akbps_layered_lib_weights
construct-contacts-radically-fast -adjunct-circle-restrictions -1.6 -1.2 -0.8 -0.4 -0.0000001 -precutting-shifts 0.9 -add-collapsed-adjuncts -calculate-adjacencies -generate-graphics
calculate-akbps-layered -use [-min-seq-sep 2]  -adj-prefix AKBP #-verbose-adjuncts
select-atoms [-chain A -rnum 4] -name asel1
select-atoms [-chain A -rnum 22] -name asel2
reset-time
#vcblocks -sel-for-display [-a1 [asel1] -a2 [asel2]] -log-to-stderr \
#  -residue-value-names-for-encoding atoms_count volume sas_area area_near area_far \
#  -far-contact-value-names-for-encoding area boundary AKBP_kbp1_exp AKBP_kbp1_exp_sa1x AKBP_kbp1_exp_sa2x AKBP_kbp1_exp_sa3x AKBP_kbp1_exp_sa4x AKBP_kbp1_exp_sa5x AKBP_kbp1_exp_saXa AKBP_kbp1_exp_saXb AKBP_kbp1_exp_saXc AKBP_kbp1_exp_saXx AKBP_kbp1_obs AKBP_kbp1_obs_sa1x AKBP_kbp1_obs_sa2x AKBP_kbp1_obs_sa3x AKBP_kbp1_obs_sa4x AKBP_kbp1_obs_sa5x AKBP_kbp1_obs_saXa AKBP_kbp1_obs_saXb AKBP_kbp1_obs_saXc AKBP_kbp1_obs_saXx AKBP_kbp2_exp_a AKBP_kbp2_exp_a_sa1x AKBP_kbp2_exp_a_sa2x AKBP_kbp2_exp_a_sa3x AKBP_kbp2_exp_a_sa4x AKBP_kbp2_exp_a_sa5x AKBP_kbp2_exp_a_saXa AKBP_kbp2_exp_a_saXb AKBP_kbp2_exp_a_saXc AKBP_kbp2_exp_a_saXx AKBP_kbp2_exp_b AKBP_kbp2_exp_b_sa1x AKBP_kbp2_exp_b_sa2x AKBP_kbp2_exp_b_sa3x AKBP_kbp2_exp_b_sa4x AKBP_kbp2_exp_b_sa5x AKBP_kbp2_exp_b_saXa AKBP_kbp2_exp_b_saXb AKBP_kbp2_exp_b_saXc AKBP_kbp2_exp_b_saXx AKBP_kbp2_obs AKBP_kbp2_obs_sa1x AKBP_kbp2_obs_sa2x AKBP_kbp2_obs_sa3x AKBP_kbp2_obs_sa4x AKBP_kbp2_obs_sa5x AKBP_kbp2_obs_saXa AKBP_kbp2_obs_saXb AKBP_kbp2_obs_saXc AKBP_kbp2_obs_saXx AKBP_known_area AKBP_raw_sa1x AKBP_raw_sa2x AKBP_raw_sa3x AKBP_raw_sa4x AKBP_raw_sa5x AKBP_raw_saXa AKBP_raw_saXb AKBP_raw_saXc AKBP_raw_saXx AKBP_weighted_sum pcut00000levelareaM00000 pcut00000levelareaM00040 pcut00000levelareaM00080 pcut00000levelareaM00120 pcut00000levelareaM00160 pcut00000subareaM00040toM00000 pcut00000subareaM00080toM00040 pcut00000subareaM00120toM00080 pcut00000subareaM00160toM00120 pcut00000subareaM99900toM00000 pcut00000subareaM99900toM00160 pcut00001levelareaM00000 pcut00001levelareaM00040 pcut00001levelareaM00080 pcut00001levelareaM00120 pcut00001levelareaM00160 pcut00001subareaM00040toM00000 pcut00001subareaM00080toM00040 pcut00001subareaM00120toM00080 pcut00001subareaM00160toM00120 pcut00001subareaM99900toM00000 pcut00001subareaM99900toM00160 pcut00003levelareaM00000 pcut00003levelareaM00040 pcut00003levelareaM00080 pcut00003levelareaM00120 pcut00003levelareaM00160 pcut00003subareaM00040toM00000 pcut00003subareaM00080toM00040 pcut00003subareaM00120toM00080 pcut00003subareaM00160toM00120 pcut00003subareaM99900toM00000 pcut00003subareaM99900toM00160 \
#  -near-contact-value-names-for-encoding area boundary \
#  -output-table ../tmp/vcblocks.txt
vcblocks -sel-for-display [-a1 [asel1] -a2 [asel2]] -log-to-stderr \
  -residue-value-names-for-encoding volume sas_area \
  -far-contact-value-names-for-encoding area boundary \
  -near-contact-value-names-for-encoding area boundary \
  -output-table ../tmp/vcblocks.txt
print-time
select-contacts [-a1 [asel1] -no-solvent] -name csel1
select-contacts [-a1 [asel2] -no-solvent] -name csel2
select-atoms [-sel-of-contacts csel1] -full-residues -name aselcsel1
select-atoms [-sel-of-contacts csel2] -full-residues -name aselcsel2
select-atoms ([aselcsel1] and [aselcsel2]) -name aselCommon
hide-atoms
show-atoms [aselCommon] -rep sticks
color-atoms [aselCommon] -col 0xFFFFFF
color-atoms ([asel1] or [asel2]) -col 0xFFBBFF
color-contacts -col black -rep edges
spectrum-contacts -by residue-ids -rep faces -scheme rygwbwbcgyr
zoom-by-atoms [-sel-of-contacts _visible]
*/

#endif /* SCRIPTING_OPERATORS_VCBLOCKS_H_ */
