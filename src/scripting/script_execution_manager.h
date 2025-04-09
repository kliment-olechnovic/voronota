#ifndef SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_
#define SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_

#include "operators/add_figure.h"
#include "operators/add_figures_of_labels.h"
#include "operators/add_figure_of_convex_hull.h"
#include "operators/add_figure_of_nth_order_cell.h"
#include "operators/add_figure_of_2nd_order_cells_for_contacts.h"
#include "operators/add_figure_of_text.h"
#include "operators/add_figure_of_triangulation.h"
#include "operators/add_figure_of_voxels_test.h"
#include "operators/add_figures_of_primitive_directions.h"
#include "operators/cad_score_many.h"
#include "operators/cad_score.h"
#include "operators/calculate_akbps.h"
#include "operators/calculate_akbps_layered.h"
#include "operators/calculate_betweenness.h"
#include "operators/calculate_burial_depth.h"
#include "operators/cat_files.h"
#include "operators/center_atoms.h"
#include "operators/check_distance_constraint.h"
#include "operators/clash_score.h"
#include "operators/collect_inter_atom_contacts_area_ranges.h"
#include "operators/collect_inter_residue_contacts_area_ranges.h"
#include "operators/color_atoms.h"
#include "operators/color_contacts.h"
#include "operators/color_figures.h"
#include "operators/color_figures_of_labels.h"
#include "operators/construct_contacts.h"
#include "operators/construct_triangulation.h"
#include "operators/copy_object.h"
#include "operators/count_common_tessellation_elements.h"
#include "operators/delete_adjuncts_of_atoms.h"
#include "operators/delete_adjuncts_of_contacts.h"
#include "operators/delete_figures.h"
#include "operators/delete_figures_of_labels.h"
#include "operators/delete_global_adjuncts.h"
#include "operators/delete_objects.h"
#include "operators/delete_objects_if.h"
#include "operators/delete_selections_of_atoms.h"
#include "operators/delete_selections_of_contacts.h"
#include "operators/delete_tags_of_atoms.h"
#include "operators/delete_tags_of_contacts.h"
#include "operators/delete_virtual_files.h"
#include "operators/describe_exposure.h"
#include "operators/detect_any_contact_between_structures.h"
#include "operators/distance.h"
#include "operators/download_virtual_file.h"
#include "operators/echo.h"
#include "operators/ensure_exactly_matching_atom_ids_in_objects.h"
#include "operators/estimate_axis.h"
#include "operators/exit.h"
#include "operators/explain_command.h"
#include "operators/export_adjuncts_of_atoms_as_casp_qa_line.h"
#include "operators/export_adjuncts_of_atoms.h"
#include "operators/export_adjuncts_of_contacts.h"
#include "operators/export_atoms_and_contacts.h"
#include "operators/export_atoms_as_pymol_cgo.h"
#include "operators/export_atoms.h"
#include "operators/export_cartoon_as_pymol_cgo.h"
#include "operators/export_contacts_as_connected_mesh.h"
#include "operators/export_contacts_as_pymol_cgo.h"
#include "operators/export_contacts.h"
#include "operators/export_directional_atom_type_tuples.h"
#include "operators/export_figures_as_pymol_cgo.h"
#include "operators/export_global_adjuncts.h"
#include "operators/export_objects.h"
#include "operators/export_selection_of_atoms.h"
#include "operators/export_selection_of_contacts.h"
#include "operators/export_sequence.h"
#include "operators/export_triangulated_convex_hull.h"
#include "operators/export_triangulation.h"
#include "operators/export_triangulation_voxels.h"
#include "operators/find_connected_components.h"
#include "operators/generate_residue_voromqa_energy_profile.h"
#include "operators/import_adjuncts_of_atoms.h"
#include "operators/import_contacts.h"
#include "operators/import_docking_result.h"
#include "operators/import_figure_voxels.h"
#include "operators/import_many.h"
#include "operators/import_objects.h"
#include "operators/import.h"
#include "operators/import_selection_of_atoms.h"
#include "operators/import_selection_of_contacts.h"
#include "operators/list_commands.h"
#include "operators/list_figures.h"
#include "operators/list_objects.h"
#include "operators/list_selections_of_atoms.h"
#include "operators/list_selections_of_contacts.h"
#include "operators/list_virtual_files.h"
#include "operators/make_drawable_contacts.h"
#include "operators/make_undrawable_contacts.h"
#include "operators/mark_atoms.h"
#include "operators/mark_contacts.h"
#include "operators/merge_objects.h"
#include "operators/mock.h"
#include "operators/mock_voromqa_local_contacts.h"
#include "operators/move_atoms.h"
#include "operators/pick_objects.h"
#include "operators/print_atoms.h"
#include "operators/print_contacts.h"
#include "operators/print_figures.h"
#include "operators/print_global_adjuncts.h"
#include "operators/print_sequence.h"
#include "operators/print_time.h"
#include "operators/print_triangulation.h"
#include "operators/print_virtual_file.h"
#include "operators/ranks_jury_score.h"
#include "operators/reduce_table_redundancy_by_bucketing.h"
#include "operators/redundancy_score.h"
#include "operators/rename_global_adjunct.h"
#include "operators/rename_object.h"
#include "operators/rename_selection_of_atoms.h"
#include "operators/rename_selection_of_contacts.h"
#include "operators/reset_time.h"
#include "operators/restrict_atoms_and_renumber_residues_by_adjunct.h"
#include "operators/restrict_atoms.h"
#include "operators/select_atoms_by_triangulation_query.h"
#include "operators/select_atoms_close_to_inter_chain_interfaces.h"
#include "operators/select_atoms.h"
#include "operators/select_contacts.h"
#include "operators/set_adjunct_of_atoms_by_contact_adjuncts.h"
#include "operators/set_adjunct_of_atoms_by_contact_areas.h"
#include "operators/set_adjunct_of_atoms_by_expression.h"
#include "operators/set_adjunct_of_atoms_by_projection.h"
#include "operators/set_adjunct_of_atoms_by_residue_pooling.h"
#include "operators/set_adjunct_of_atoms_by_sequence_alignment.h"
#include "operators/set_adjunct_of_atoms_by_type_number.h"
#include "operators/set_adjunct_of_atoms.h"
#include "operators/set_adjunct_of_contacts_by_atom_adjuncts.h"
#include "operators/set_adjunct_of_contacts_by_expression.h"
#include "operators/set_adjunct_of_contacts.h"
#include "operators/set_adjuncts_of_atoms_by_ufsr.h"
#include "operators/set_adjuncts_of_contacts_by_ufsr.h"
#include "operators/set_alias.h"
#include "operators/set_atom_names.h"
#include "operators/set_atom_serials.h"
#include "operators/set_chain_name.h"
#include "operators/set_chain_names_and_residue_numbers_by_sequences.h"
#include "operators/set_chain_names_by_guessing.h"
#include "operators/set_chain_residue_numbers_by_sequence.h"
#include "operators/set_global_adjunct_by_pooling.h"
#include "operators/set_global_adjunct.h"
#include "operators/set_residue_numbers_sequentially.h"
#include "operators/set_tag_of_atoms_by_secondary_structure.h"
#include "operators/set_tag_of_atoms.h"
#include "operators/set_tag_of_contacts.h"
#include "operators/setup_akbps.h"
#include "operators/setup_akbps_layered.h"
#include "operators/setup_chemistry_annotating.h"
#include "operators/setup_loading.h"
#include "operators/setup_mock_voromqa.h"
#include "operators/setup_parallelization.h"
#include "operators/setup_random_seed.h"
#include "operators/setup_voromqa.h"
#include "operators/show_atoms.h"
#include "operators/show_contacts.h"
#include "operators/show_figures.h"
#include "operators/show_figures_of_labels.h"
#include "operators/show_next_picked_object.h"
#include "operators/show_objects.h"
#include "operators/smooth_adjacent_contact_adjunct_values.h"
#include "operators/sort_atoms_by_residue_id.h"
#include "operators/source.h"
#include "operators/spectrum_atoms.h"
#include "operators/spectrum_contacts.h"
#include "operators/split_blocks_file.h"
#include "operators/split_pdb_file.h"
#include "operators/summarize_linear_structure.h"
#include "operators/tournament_sort.h"
#include "operators/unset_aliases.h"
#include "operators/upload_virtual_file.h"
#include "operators/vcblocks.h"
#include "operators/voromqa_frustration.h"
#include "operators/voromqa_global.h"
#include "operators/voromqa_interface_frustration.h"
#include "operators/voromqa_local.h"
#include "operators/voromqa_membrane_place.h"
#include "operators/zoom_by_atoms.h"
#include "operators/zoom_by_contacts.h"
#include "operators/zoom_by_objects.h"

namespace voronota
{

namespace scripting
{

class ScriptExecutionManager
{
public:
	struct ScriptRecord
	{
		struct CommandRecord
		{
			CommandInput command_input;
			bool successful;

			CommandRecord() : successful(false)
			{
			}
		};

		std::vector<CommandRecord> command_records;
		std::string termination_error;

		std::size_t count_successfull_commmand_records() const
		{
			std::size_t n=0;
			for(std::size_t i=0;i<command_records.size();i++)
			{
				if(command_records[i].successful)
				{
					n++;
				}
			}
			return n;
		}
	};

	ScriptExecutionManager() :
		exit_requested_(false)
	{
		set_command_for_script_partitioner("set-alias", operators::SetAlias());
		set_command_for_script_partitioner("unset-aliases", operators::UnsetAliases());
		set_command_for_script_partitioner("source", operators::Source());

		set_command_for_congregation_of_data_managers("list-objects", operators::ListObjects());
		set_command_for_congregation_of_data_managers("delete-objects", operators::DeleteObjects());
		set_command_for_congregation_of_data_managers("delete-objects-if", operators::DeleteObjectsIf());
		set_command_for_congregation_of_data_managers("rename-object", operators::RenameObject());
		set_command_for_congregation_of_data_managers("collect-inter-atom-contact-area-ranges", operators::CollectInterAtomContactAreaRanges());
		set_command_for_congregation_of_data_managers("collect-inter-residue-contact-area-ranges", operators::CollectInterResidueContactAreaRanges());
		set_command_for_congregation_of_data_managers("copy-object", operators::CopyObject());
		set_command_for_congregation_of_data_managers("import-docking-result", operators::ImportDockingResult());
		set_command_for_congregation_of_data_managers("import-many", operators::ImportMany());
		set_command_for_congregation_of_data_managers("import", operators::Import());
		set_command_for_congregation_of_data_managers("merge-objects", operators::MergeObjects());
		set_command_for_congregation_of_data_managers("pick-objects", operators::PickObjects());
		set_command_for_congregation_of_data_managers("pick-more-objects", operators::PickMoreObjects());
		set_command_for_congregation_of_data_managers("unpick-objects", operators::UnpickObjects());
		set_command_for_congregation_of_data_managers("show-next-picked-object", operators::ShowNextPickedObject());
		set_command_for_congregation_of_data_managers("show-objects", operators::ShowObjects());
		set_command_for_congregation_of_data_managers("hide-objects", operators::HideObjects());
		set_command_for_congregation_of_data_managers("zoom-by-objects", operators::ZoomByObjects());
		set_command_for_congregation_of_data_managers("cad-score", operators::CADScore());
		set_command_for_congregation_of_data_managers("cad-score-many", operators::CADScoreMany());
		set_command_for_congregation_of_data_managers("export-global-adjuncts", operators::ExportGlobalAdjuncts());
		set_command_for_congregation_of_data_managers("export-objects", operators::ExportObjects());
		set_command_for_congregation_of_data_managers("import-objects", operators::ImportObjects());
		set_command_for_congregation_of_data_managers("ensure-exactly-matching-atom-ids-in-objects", operators::EnsureExactlyMatchingAtomIDsInObjects());
		set_command_for_congregation_of_data_managers("count-common-tessellation-elements", operators::CountCommonTessellationElements());

		set_command_for_data_manager("add-figure", operators::AddFigure(), true);
		set_command_for_data_manager("add-figures-of-labels", operators::AddFiguresOfLabels(), true);
		set_command_for_data_manager("add-figure-of-convex-hull", operators::AddFigureOfConvexHull(), true);
		set_command_for_data_manager("add-figure-of-nth-order-cell", operators::AddFigureOfNthOrderCell(), true);
		set_command_for_data_manager("add-figure-of-2nd-order-cells-for-contacts", operators::AddFigureOf2ndOrderCellsForContacts(), true);
		set_command_for_data_manager("add-figure-of-text", operators::AddFigureOfText(), true);
		set_command_for_data_manager("add-figure-of-triangulation", operators::AddFigureOfTriangulation(), true);
		set_command_for_data_manager("add-figure-of-voxels-test", operators::AddFigureOfVoxelsTest(), true);
		set_command_for_data_manager("add-figures-of-primitive-directions", operators::AddFiguresOfPrimitiveDirections(), true);
		set_command_for_data_manager("calculate-akbps", operators::CalculateAKBPs(), true);
		set_command_for_data_manager("calculate-akbps-layered", operators::CalculateAKBPsLayered(), true);
		set_command_for_data_manager("calculate-betweenness", operators::CalculateBetweenness(), true);
		set_command_for_data_manager("calculate-burial-depth", operators::CalculateBurialDepth(), true);
		set_command_for_data_manager("center-atoms", operators::CenterAtoms(), true);
		set_command_for_data_manager("check-distance-constraint", operators::CheckDistanceConstraint(), true);
		set_command_for_data_manager("clash-score", operators::ClashScore(), true);
		set_command_for_data_manager("color-atoms", operators::ColorAtoms(), true);
		set_command_for_data_manager("color-contacts", operators::ColorContacts(), true);
		set_command_for_data_manager("color-figures", operators::ColorFigures(), true);
		set_command_for_data_manager("color-figures-of-labels", operators::ColorFiguresOfLabels(), true);
		set_command_for_data_manager("construct-contacts", operators::ConstructContacts(), true);
		set_command_for_data_manager("construct-triangulation", operators::ConstructTriangulation(), true);
		set_command_for_data_manager("delete-adjuncts-of-atoms", operators::DeleteAdjunctsOfAtoms(), true);
		set_command_for_data_manager("delete-adjuncts-of-contacts", operators::DeleteAdjunctsOfContacts(), true);
		set_command_for_data_manager("delete-figures", operators::DeleteFigures(), true);
		set_command_for_data_manager("delete-figures-of-labels", operators::DeleteFiguresOfLabels(), true);
		set_command_for_data_manager("delete-global-adjuncts", operators::DeleteGlobalAdjuncts(), true);
		set_command_for_data_manager("delete-selections-of-atoms", operators::DeleteSelectionsOfAtoms(), true);
		set_command_for_data_manager("delete-selections-of-contacts", operators::DeleteSelectionsOfContacts(), true);
		set_command_for_data_manager("delete-tags-of-atoms", operators::DeleteTagsOfAtoms(), true);
		set_command_for_data_manager("delete-tags-of-contacts", operators::DeleteTagsOfContacts(), true);
		set_command_for_data_manager("describe-exposure", operators::DescribeExposure(), true);
		set_command_for_data_manager("distance", operators::Distance(), true);
		set_command_for_data_manager("estimate-axis", operators::EstimateAxis(), true);
		set_command_for_data_manager("export-adjuncts-of-atoms-as-casp-qa-line", operators::ExportAdjunctsOfAtomsAsCASPQALine(), false);
		set_command_for_data_manager("export-adjuncts-of-atoms", operators::ExportAdjunctsOfAtoms(), false);
		set_command_for_data_manager("export-adjuncts-of-contacts", operators::ExportAdjunctsOfContacts(), false);
		set_command_for_data_manager("export-atoms-and-contacts", operators::ExportAtomsAndContacts(), false);
		set_command_for_data_manager("export-atoms-as-pymol-cgo", operators::ExportAtomsAsPymolCGO(), false);
		set_command_for_data_manager("export-atoms", operators::ExportAtoms(), false);
		set_command_for_data_manager("export-cartoon-as-pymol-cgo", operators::ExportCartoonAsPymolCGO(), false);
		set_command_for_data_manager("export-contacts-as-connected-mesh", operators::ExportContactsAsConnectedMesh(), false);
		set_command_for_data_manager("export-contacts-as-pymol-cgo", operators::ExportContactsAsPymolCGO(), false);
		set_command_for_data_manager("export-contacts", operators::ExportContacts(), false);
		set_command_for_data_manager("export-directional-atom-type-tuples", operators::ExportDirectionalAtomTypeTuples(), false);
		set_command_for_data_manager("export-figures-as-pymol-cgo", operators::ExportFiguresAsPymolCGO(), false);
		set_command_for_data_manager("export-selection-of-atoms", operators::ExportSelectionOfAtoms(), false);
		set_command_for_data_manager("export-selection-of-contacts", operators::ExportSelectionOfContacts(), false);
		set_command_for_data_manager("export-sequence", operators::ExportSequence(), false);
		set_command_for_data_manager("export-triangulated-convex-hull", operators::ExportTriangulatedConvexHull(), false);
		set_command_for_data_manager("export-triangulation", operators::ExportTriangulation(), false);
		set_command_for_data_manager("export-triangulation-voxels", operators::ExportTriangulationVoxels(), false);
		set_command_for_data_manager("find-connected-components", operators::FindConnectedComponents(), true);
		set_command_for_data_manager("generate-residue-voromqa-energy-profile", operators::GenerateResidueVoroMQAEnergyProfile(), false);
		set_command_for_data_manager("hide-atoms", operators::HideAtoms(), true);
		set_command_for_data_manager("hide-contacts", operators::HideContacts(), true);
		set_command_for_data_manager("hide-figures", operators::HideFigures(), true);
		set_command_for_data_manager("hide-figures-of-labels", operators::HideFiguresOfLabels(), true);
		set_command_for_data_manager("import-adjuncts-of-atoms", operators::ImportAdjunctsOfAtoms(), true);
		set_command_for_data_manager("import-contacts", operators::ImportContacts(), false);
		set_command_for_data_manager("import-figure-voxels", operators::ImportFigureVoxels(), false);
		set_command_for_data_manager("import-selection-of-atoms", operators::ImportSelectionOfAtoms(), true);
		set_command_for_data_manager("import-selection-of-contacts", operators::ImportSelectionOfContacts(), true);
		set_command_for_data_manager("list-figures", operators::ListFigures(), true);
		set_command_for_data_manager("list-selections-of-atoms", operators::ListSelectionsOfAtoms(), true);
		set_command_for_data_manager("list-selections-of-contacts", operators::ListSelectionsOfContacts(), true);
		set_command_for_data_manager("make-drawable-contacts", operators::MakeDrawableContacts(), true);
		set_command_for_data_manager("make-undrawable-contacts", operators::MakeUndrawableContacts(), true);
		set_command_for_data_manager("mark-atoms", operators::MarkAtoms(), true);
		set_command_for_data_manager("mark-contacts", operators::MarkContacts(), true);
		set_command_for_data_manager("mock-voromqa-local-contacts", operators::MockVoroMQALocalContacts(), true);
		set_command_for_data_manager("move-atoms", operators::MoveAtoms(), true);
		set_command_for_data_manager("print-atoms", operators::PrintAtoms(), true);
		set_command_for_data_manager("print-contacts", operators::PrintContacts(), true);
		set_command_for_data_manager("print-figures", operators::PrintFigures(), true);
		set_command_for_data_manager("print-global-adjuncts", operators::PrintGlobalAdjuncts(), true);
		set_command_for_data_manager("print-sequence", operators::PrintSequence(), true);
		set_command_for_data_manager("print-triangulation", operators::PrintTriangulation(), true);
		set_command_for_data_manager("rename-global-adjunct", operators::RenameGlobalAdjunct(), true);
		set_command_for_data_manager("rename-selection-of-atoms", operators::RenameSelectionOfAtoms(), true);
		set_command_for_data_manager("rename-selection-of-contacts", operators::RenameSelectionOfContacts(), true);
		set_command_for_data_manager("restrict-atoms-and-renumber-residues-by-adjunct", operators::RestrictAtomsAndRenumberResiduesByAdjunct(), true);
		set_command_for_data_manager("restrict-atoms", operators::RestrictAtoms(), true);
		set_command_for_data_manager("select-atoms-by-triangulation-query", operators::SelectAtomsByTriangulationQuery(), true);
		set_command_for_data_manager("select-atoms-close-to-interchain-interface", operators::SelectAtomsCloseToInterchainInterfaces(), true);
		set_command_for_data_manager("select-atoms", operators::SelectAtoms(), true);
		set_command_for_data_manager("select-contacts", operators::SelectContacts(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-contact-adjuncts", operators::SetAdjunctOfAtomsByContactAdjuncts(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-contact-areas", operators::SetAdjunctOfAtomsByContactAreas(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-expression", operators::SetAdjunctOfAtomsByExpression(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-projection", operators::SetAdjunctOfAtomsByProjection(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-residue-pooling", operators::SetAdjunctOfAtomsByResiduePooling(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-sequence-alignment", operators::SetAdjunctOfAtomsBySequenceAlignment(), true);
		set_command_for_data_manager("set-adjunct-of-atoms-by-type-number", operators::SetAdjunctOfAtomsByTypeNumber(), true);
		set_command_for_data_manager("set-adjunct-of-atoms", operators::SetAdjunctOfAtoms(), true);
		set_command_for_data_manager("set-adjunct-of-contacts-by-atom-adjuncts", operators::SetAdjunctOfContactsByAtomAdjuncts(), true);
		set_command_for_data_manager("set-adjunct-of-contacts-by-expression", operators::SetAdjunctOfContactsByExpression(), true);
		set_command_for_data_manager("set-adjunct-of-contacts", operators::SetAdjunctOfContacts(), true);
		set_command_for_data_manager("set-adjuncts-of-atoms-by-ufsr", operators::SetAdjunctsOfAtomsByTypeUFSR(), true);
		set_command_for_data_manager("set-adjuncts-of-contacts-by-ufsr", operators::SetAdjunctsOfContactsByUFSR(), true);
		set_command_for_data_manager("set-atom-names", operators::SetAtomNames(), true);
		set_command_for_data_manager("set-atom-serials", operators::SetAtomSerials(), true);
		set_command_for_data_manager("set-chain-name", operators::SetChainName(), true);
		set_command_for_data_manager("set-chain-names-and-residue-numbers-by-sequences", operators::SetChainNamesAndResidueNumbersBySequences(), true);
		set_command_for_data_manager("set-chain-names-by-guessing", operators::SetChainNamesByGuessing(), true);
		set_command_for_data_manager("set-chain-residue-numbers-by-sequence", operators::SetChainResidueNumbersBySequences(), true);
		set_command_for_data_manager("set-global-adjunct-by-pooling", operators::SetGlobalAdjunctByPooling(), true);
		set_command_for_data_manager("set-global-adjunct", operators::SetGlobalAdjunct(), true);
		set_command_for_data_manager("set-residue-numbers-sequentially", operators::SetResidueNumbersSequentially(), true);
		set_command_for_data_manager("set-tag-of-atoms-by-secondary-structure", operators::SetTagOfAtomsBySecondaryStructure(), true);
		set_command_for_data_manager("set-tag-of-atoms", operators::SetTagOfAtoms(), true);
		set_command_for_data_manager("set-tag-of-contacts", operators::SetTagOfContacts(), true);
		set_command_for_data_manager("show-atoms", operators::ShowAtoms(), true);
		set_command_for_data_manager("show-contacts", operators::ShowContacts(), true);
		set_command_for_data_manager("show-figures", operators::ShowFigures(), true);
		set_command_for_data_manager("show-figures-of-labels", operators::ShowFiguresOfLabels(), true);
		set_command_for_data_manager("smooth-adjacent-contact-adjunct-values", operators::SmoothAdjacentContactAdjunctValues(), true);
		set_command_for_data_manager("sort-atoms-by-residue-id", operators::SortAtomsByResidueID(), true);
		set_command_for_data_manager("spectrum-atoms", operators::SpectrumAtoms(), true);
		set_command_for_data_manager("spectrum-contacts", operators::SpectrumContacts(), true);
		set_command_for_data_manager("summarize-linear-structure", operators::SummarizeLinearStructure(), true);
		set_command_for_data_manager("unmark-atoms", operators::UnmarkAtoms(), true);
		set_command_for_data_manager("unmark-contacts", operators::UnmarkContacts(), true);
		set_command_for_data_manager("voromqa-frustration", operators::VoroMQAFrustration(), true);
		set_command_for_data_manager("voromqa-global", operators::VoroMQAGlobal(), true);
		set_command_for_data_manager("voromqa-interface-frustration", operators::VoroMQAInterfaceFrustration(), true);
		set_command_for_data_manager("voromqa-local", operators::VoroMQALocal(), true);
		set_command_for_data_manager("voromqa-membrane-place", operators::VoroMQAMembranePlace(), true);
		set_command_for_data_manager("vcblocks", operators::VCBlocks(), true);
		set_command_for_data_manager("zoom-by-atoms", operators::ZoomByAtoms(), false);
		set_command_for_data_manager("zoom-by-contacts", operators::ZoomByContacts(), false);

		set_command_for_extra_actions("reset-time", operators::ResetTime(elapsed_processor_time_));
		set_command_for_extra_actions("print-time", operators::PrintTime(elapsed_processor_time_));
		set_command_for_extra_actions("exit", operators::Exit(exit_requested_));
		set_command_for_extra_actions("echo", operators::Echo());
		set_command_for_extra_actions("list-virtual-files", operators::ListVirtualFiles());
		set_command_for_extra_actions("upload-virtual-file", operators::UploadVirtualFile());
		set_command_for_extra_actions("download-virtual-file", operators::DownloadVirtualFile());
		set_command_for_extra_actions("print-virtual-file", operators::PrintVirtualFile());
		set_command_for_extra_actions("delete-virtual-files", operators::DeleteVirtualFiles());
		set_command_for_extra_actions("setup-akbps", operators::SetupAKBPs());
		set_command_for_extra_actions("setup-akbps-layered", operators::SetupAKBPsLayered());
		set_command_for_extra_actions("setup-chemistry-annotating", operators::SetupChemistryAnnotating());
		set_command_for_extra_actions("setup-loading", operators::SetupLoading());
		set_command_for_extra_actions("setup-mock-voromqa", operators::SetupMockVoroMQA());
		set_command_for_extra_actions("setup-parallelization", operators::SetupParallelization());
		set_command_for_extra_actions("setup-random-seed", operators::SetupRandomSeed());
		set_command_for_extra_actions("setup-voromqa", operators::SetupVoroMQA());
		set_command_for_extra_actions("explain-command", operators::ExplainCommand(collection_of_command_documentations_));
		set_command_for_extra_actions("list-commands", operators::ListCommands(collection_of_command_documentations_));
		set_command_for_extra_actions("tournament-sort", operators::TournamentSort());
		set_command_for_extra_actions("split-blocks-file", operators::SplitBlocksFile());
		set_command_for_extra_actions("split-pdb-file", operators::SplitPDBFile());
		set_command_for_extra_actions("cat-files", operators::CatFiles());
		set_command_for_extra_actions("ranks-jury-score", operators::RanksJuryScore());
		set_command_for_extra_actions("reduce-table-redundancy-by-bucketing", operators::ReduceTableRedundancyByBucketing());
		set_command_for_extra_actions("redundancy-score", operators::RedundancyScore());
		set_command_for_extra_actions("detect-any-contact-between-structures", operators::DetectAnyContactBetweenStructures());
	}

	virtual ~ScriptExecutionManager()
	{
		SafeUtilitiesForMapOfPointers::clear(commands_for_script_partitioner_);
		SafeUtilitiesForMapOfPointers::clear(commands_for_congregation_of_data_managers_);
		SafeUtilitiesForMapOfPointers::clear(commands_for_data_manager_);
		SafeUtilitiesForMapOfPointers::clear(commands_for_extra_actions_);
	}

	const CollectionOfCommandDocumentations& collection_of_command_documentations() const
	{
		return collection_of_command_documentations_;
	}

	bool exit_requested() const
	{
		return exit_requested_;
	}

	ScriptRecord execute_script(const std::string& script, const bool exit_on_first_failure)
	{
		on_before_script(script);

		ScriptRecord script_record;

		execute_script(script, exit_on_first_failure, script_record);

		on_after_script(script_record);

		return script_record;
	}

protected:
	class GenericCommandRecord
	{
	public:
		bool successful;
		CommandInput command_input;
		HeterogeneousStorage heterostorage;

		explicit GenericCommandRecord(const CommandInput& command_input) :
			successful(false),
			command_input(command_input)
		{
		}

		virtual ~GenericCommandRecord()
		{
		}

		void save_error(const std::exception& e)
		{
			heterostorage.errors.push_back(std::string(e.what()));
		}
	};

	ScriptPartitioner& script_partitioner()
	{
		return script_partitioner_;
	}

	CongregationOfDataManagers& congregation_of_data_managers()
	{
		return congregation_of_data_managers_;
	}

	void unset_command(const std::string& name)
	{
		SafeUtilitiesForMapOfPointers::erase(commands_for_script_partitioner_, name);
		SafeUtilitiesForMapOfPointers::erase(commands_for_congregation_of_data_managers_, name);
		SafeUtilitiesForMapOfPointers::erase(commands_for_data_manager_, name);
		SafeUtilitiesForMapOfPointers::erase(commands_for_extra_actions_, name);
		collection_of_command_documentations_.delete_documentation(name);
	}

	template<class Operator>
	void set_command_for_script_partitioner(const std::string& name, const Operator& op)
	{
		unset_command(name);
		GenericCommandForScriptPartitioner* command_ptr=new GenericCommandForScriptPartitionerFromOperator<Operator>(op);
		SafeUtilitiesForMapOfPointers::set_key_value(commands_for_script_partitioner_, name, command_ptr);
		collection_of_command_documentations_.set_documentation(name, op.documentation());
	}

	template<class Operator>
	void set_command_for_congregation_of_data_managers(const std::string& name, const Operator& op)
	{
		unset_command(name);
		GenericCommandForCongregationOfDataManagers* command_ptr=new GenericCommandForCongregationOfDataManagersFromOperator<Operator>(op);
		SafeUtilitiesForMapOfPointers::set_key_value(commands_for_congregation_of_data_managers_, name, command_ptr);
		collection_of_command_documentations_.set_documentation(name, op.documentation());
	}

	template<class Operator>
	void set_command_for_data_manager(const std::string& name, const Operator& op, const bool on_multiple)
	{
		unset_command(name);
		GenericCommandForDataManager* command_ptr=new GenericCommandForDataManagerFromOperator<Operator>(op, on_multiple);
		SafeUtilitiesForMapOfPointers::set_key_value(commands_for_data_manager_, name, command_ptr);
		collection_of_command_documentations_.set_documentation(name, op.documentation());
	}

	template<class Operator>
	void set_command_for_extra_actions(const std::string& name, const Operator& op)
	{
		unset_command(name);
		GenericCommandForExtraActions* command_ptr=new GenericCommandForExtraActionsFromOperator<Operator>(op);
		SafeUtilitiesForMapOfPointers::set_key_value(commands_for_extra_actions_, name, command_ptr);
		collection_of_command_documentations_.set_documentation(name, op.documentation());
	}

	virtual void on_before_script(const std::string&)
	{
	}

	virtual void on_before_any_command(const CommandInput&)
	{
	}

	virtual void on_after_command_for_script_partitioner(const GenericCommandRecord&, ScriptPartitioner&)
	{
	}

	virtual void on_after_command_for_congregation_of_data_managers(const GenericCommandRecord&, CongregationOfDataManagers&)
	{
	}

	virtual void on_after_command_for_data_manager(const GenericCommandRecord&, DataManager&)
	{
	}

	virtual void on_after_command_for_extra_actions(const GenericCommandRecord&)
	{
	}

	virtual void on_command_not_allowed_for_multiple_data_managers(const CommandInput&)
	{
	}

	virtual void on_no_picked_data_manager_for_command(const CommandInput&)
	{
	}

	virtual void on_unrecognized_command(const std::string&)
	{
	}

	virtual void on_after_any_command(const GenericCommandRecord&)
	{
	}

	virtual void on_after_script(const ScriptRecord&)
	{
	}

private:
	ScriptExecutionManager(const ScriptExecutionManager&);

	const ScriptExecutionManager& operator=(const ScriptExecutionManager&);

	template<class Subject, class Operator>
	class GenericCommandForSubject
	{
	public:
		explicit GenericCommandForSubject(const Operator& op) : op_(op),  on_multiple_(true)
		{
		}

		GenericCommandForSubject(const Operator& op, const bool on_multiple) : op_(op),  on_multiple_(on_multiple)
		{
		}

		virtual ~GenericCommandForSubject()
		{
		}

		bool on_multiple() const
		{
			return on_multiple_;
		}

		bool run(GenericCommandRecord& record, Subject& subject) const
		{
			try
			{
				prepare(subject, record.command_input);
				{
					Operator op=op_;
					op.init(record.command_input);
					op.run(subject).write(record.heterostorage);
				}
				record.successful=true;
			}
			catch(const std::exception& e)
			{
				record.save_error(e);
			}

			return record.successful;
		}

	protected:
		virtual void prepare(Subject&, CommandInput&) const
		{
		}

	private:
		Operator op_;
		bool on_multiple_;
	};

	class GenericCommandForDataManager
	{
	public:
		GenericCommandForDataManager()
		{
		}

		virtual ~GenericCommandForDataManager()
		{
		}

		virtual bool execute(GenericCommandRecord&, DataManager&) const = 0;

		virtual bool multiplicable() const = 0;
	};

	template<class Operator>
	class GenericCommandForDataManagerFromOperator : public GenericCommandForDataManager, public GenericCommandForSubject<DataManager, Operator>
	{
	public:
		GenericCommandForDataManagerFromOperator(const Operator& op, const bool on_multiple) : GenericCommandForSubject<DataManager, Operator>(op, on_multiple)
		{
		}

		bool execute(GenericCommandRecord& record, DataManager& data_manager) const
		{
			return GenericCommandForSubject<DataManager, Operator>::run(record, data_manager);
		}

		bool multiplicable() const
		{
			return GenericCommandForSubject<DataManager, Operator>::on_multiple();
		}

	private:
		void prepare(DataManager& data_manager, CommandInput& input) const
		{
			data_manager.reset_change_indicator();
			data_manager.sync_selections_with_display_states_if_requested_in_string(input.get_canonical_input_command_string());
			data_manager.selection_manager().reset_change_indicator();
		}
	};

	class GenericCommandForCongregationOfDataManagers
	{
	public:
		GenericCommandForCongregationOfDataManagers()
		{
		}

		virtual ~GenericCommandForCongregationOfDataManagers()
		{
		}

		virtual bool execute(GenericCommandRecord&, CongregationOfDataManagers&) const = 0;
	};

	template<class Operator>
	class GenericCommandForCongregationOfDataManagersFromOperator : public GenericCommandForCongregationOfDataManagers, public GenericCommandForSubject<CongregationOfDataManagers, Operator>
	{
	public:
		explicit GenericCommandForCongregationOfDataManagersFromOperator(const Operator& op) : GenericCommandForSubject<CongregationOfDataManagers, Operator>(op)
		{
		}

		bool execute(GenericCommandRecord& record, CongregationOfDataManagers& congregation_of_data_managers) const
		{
			return GenericCommandForSubject<CongregationOfDataManagers, Operator>::run(record, congregation_of_data_managers);
		}

	protected:
		void prepare(CongregationOfDataManagers& congregation_of_data_managers, CommandInput& input) const
		{
			congregation_of_data_managers.reset_change_indicator();
			congregation_of_data_managers.reset_change_indicators_of_all_objects();
			congregation_of_data_managers.sync_selections_with_display_states_of_all_objects_if_requested_in_string(input.get_canonical_input_command_string());
		}
	};

	class GenericCommandForScriptPartitioner
	{
	public:
		GenericCommandForScriptPartitioner()
		{
		}

		virtual ~GenericCommandForScriptPartitioner()
		{
		}

		virtual bool execute(GenericCommandRecord&, ScriptPartitioner&) const = 0;
	};

	template<class Operator>
	class GenericCommandForScriptPartitionerFromOperator : public GenericCommandForScriptPartitioner, public GenericCommandForSubject<ScriptPartitioner, Operator>
	{
	public:
		explicit GenericCommandForScriptPartitionerFromOperator(const Operator& op) : GenericCommandForSubject<ScriptPartitioner, Operator>(op)
		{
		}

		bool execute(GenericCommandRecord& record, ScriptPartitioner& script_partitioner) const
		{
			return GenericCommandForSubject<ScriptPartitioner, Operator>::run(record, script_partitioner);
		}
	};

	class GenericCommandForExtraActions
	{
	public:
		GenericCommandForExtraActions()
		{
		}

		virtual ~GenericCommandForExtraActions()
		{
		}

		virtual bool execute(GenericCommandRecord&) const = 0;
	};

	template<class Operator>
	class GenericCommandForExtraActionsFromOperator : public GenericCommandForExtraActions, public GenericCommandForSubject<void*, Operator>
	{
	public:
		explicit GenericCommandForExtraActionsFromOperator(const Operator& op) : GenericCommandForSubject<void*, Operator>(op)
		{
		}

		bool execute(GenericCommandRecord& record) const
		{
			void* subject=0;
			return GenericCommandForSubject<void*, Operator>::run(record, subject);
		}
	};

	class SafeUtilitiesForMapOfPointers
	{
	public:
		template<typename Map>
		static void clear(Map& map)
		{
			for(typename Map::iterator it=map.begin();it!=map.end();++it)
			{
				if(it->second!=0)
				{
					delete it->second;
				}
			}
			map.clear();
		}

		template<typename Map, typename PointerValue>
		static void set_key_value(Map& map, const std::string& key, PointerValue pointer_value)
		{
			typename Map::iterator it=map.find(key);
			if(it==map.end())
			{
				if(pointer_value!=0)
				{
					map[key]=pointer_value;
				}
			}
			else
			{
				delete it->second;
				if(pointer_value==0)
				{
					map.erase(it);
				}
				else
				{
					it->second=pointer_value;
				}
			}
		}

		template<typename Map>
		static void erase(Map& map, const std::string& key)
		{
			typename Map::iterator it=map.find(key);
			if(it!=map.end())
			{
				delete it->second;
				map.erase(it);
			}
		}
	};

	void execute_script(const std::string& script, const bool exit_on_first_failure, ScriptRecord& script_record)
	{
		exit_requested_=false;

		try
		{
			script_partitioner_.add_pending_sentences_from_string_to_front(script);
		}
		catch(const std::exception& e)
		{
			script_record.termination_error=e.what();
			return;
		}

		while(!script_partitioner_.pending_sentences().empty())
		{
			std::string command_string;

			try
			{
				command_string=script_partitioner_.extract_pending_sentence();
			}
			catch(const std::exception& e)
			{
				script_record.termination_error=e.what();
				return;
			}

			ScriptRecord::CommandRecord script_command_record;

			try
			{
				script_command_record.command_input=CommandInput(command_string);
			}
			catch(const std::exception& e)
			{
				script_record.termination_error=e.what();
				return;
			}

			execute_command(script_command_record);

			script_record.command_records.push_back(script_command_record);

			if(!script_command_record.successful && exit_on_first_failure)
			{
				script_record.termination_error="Terminated on the first failure.";
				return;
			}

			if(exit_requested_)
			{
				return;
			}
		}
	}

	void execute_command(ScriptRecord::CommandRecord& script_command_record)
	{
		const std::string& command_name=script_command_record.command_input.get_command_name();

		if(commands_for_script_partitioner_.count(command_name)==1)
		{
			on_before_any_command(script_command_record.command_input);
			GenericCommandRecord cr(script_command_record.command_input);
			script_command_record.successful=commands_for_script_partitioner_[command_name]->execute(cr, script_partitioner_);
			on_after_command_for_script_partitioner(cr, script_partitioner_);
			on_after_any_command(cr);
		}
		else if(commands_for_congregation_of_data_managers_.count(command_name)==1)
		{
			on_before_any_command(script_command_record.command_input);
			GenericCommandRecord cr(script_command_record.command_input);
			script_command_record.successful=commands_for_congregation_of_data_managers_[command_name]->execute(cr, congregation_of_data_managers_);
			on_after_command_for_congregation_of_data_managers(cr, congregation_of_data_managers_);
			on_after_any_command(cr);
		}
		else if(commands_for_data_manager_.count(command_name)==1)
		{
			CongregationOfDataManagers::ObjectQuery query;
			query.picked=true;
			if(script_command_record.command_input.is_option_with_values("on-objects"))
			{
				const std::vector<std::string> on_objects=script_command_record.command_input.get_value_vector_or_default<std::string>("on-objects", std::vector<std::string>());
				if(!on_objects.empty())
				{
					query.names.insert(on_objects.begin(), on_objects.end());
					query.picked=false;
				}
			}
			std::vector<DataManager*> picked_data_managers=congregation_of_data_managers_.get_objects(query);
			if(!picked_data_managers.empty())
			{
				if(script_command_record.command_input.check_for_any_value_with_string("${objectname}"))
				{
					for(std::size_t i=0;i<picked_data_managers.size();i++)
					{
						CommandInput adjusted_command_input=script_command_record.command_input;
						adjusted_command_input.replace_string_in_values("${objectname}", congregation_of_data_managers_.get_object_attributes(picked_data_managers[i]).name);
						on_before_any_command(adjusted_command_input);
						GenericCommandRecord cr(adjusted_command_input);
						script_command_record.successful=commands_for_data_manager_[command_name]->execute(cr, *picked_data_managers[i]);
						on_after_command_for_data_manager(cr, *picked_data_managers[i]);
						on_after_any_command(cr);
					}
				}
				else if(picked_data_managers.size()==1 || commands_for_data_manager_[command_name]->multiplicable())
				{
					const bool in_parallel=Parallelization::Configuration::get_default_configuration().in_script;
					if(!in_parallel || picked_data_managers.size()==1)
					{
						for(std::size_t i=0;i<picked_data_managers.size();i++)
						{
							on_before_any_command(script_command_record.command_input);
							GenericCommandRecord cr(script_command_record.command_input);
							script_command_record.successful=commands_for_data_manager_[command_name]->execute(cr, *picked_data_managers[i]);
							on_after_command_for_data_manager(cr, *picked_data_managers[i]);
							on_after_any_command(cr);
						}
					}
					else
					{
						int n=static_cast<int>(picked_data_managers.size());
						GenericCommandForDataManager* common_command_pointer=commands_for_data_manager_[command_name];
						std::vector<GenericCommandRecord> array_of_command_records(n, GenericCommandRecord(script_command_record.command_input));

						{
#ifdef _OPENMP
							#pragma omp parallel for
#endif
							for(int i=0;i<n;i++)
							{
								common_command_pointer->execute(array_of_command_records[i], *picked_data_managers[i]);
							}
						}

						for(int i=0;i<n;i++)
						{
							on_before_any_command(script_command_record.command_input);
							on_after_command_for_data_manager(array_of_command_records[i], *picked_data_managers[i]);
							on_after_any_command(array_of_command_records[i]);
						}
					}
				}
				else
				{
					on_before_any_command(script_command_record.command_input);
					on_command_not_allowed_for_multiple_data_managers(script_command_record.command_input);
					on_after_any_command(GenericCommandRecord(script_command_record.command_input));
				}
			}
			else
			{
				on_before_any_command(script_command_record.command_input);
				on_no_picked_data_manager_for_command(script_command_record.command_input);
				on_after_any_command(GenericCommandRecord(script_command_record.command_input));
			}
		}
		else if(commands_for_extra_actions_.count(command_name)==1)
		{
			on_before_any_command(script_command_record.command_input);
			GenericCommandRecord cr(script_command_record.command_input);
			script_command_record.successful=commands_for_extra_actions_[command_name]->execute(cr);
			on_after_command_for_extra_actions(cr);
			on_after_any_command(cr);
		}
		else
		{
			on_before_any_command(script_command_record.command_input);
			on_unrecognized_command(command_name);
			on_after_any_command(GenericCommandRecord(script_command_record.command_input));
		}
	}

	std::map<std::string, GenericCommandForScriptPartitioner*> commands_for_script_partitioner_;
	std::map<std::string, GenericCommandForCongregationOfDataManagers*> commands_for_congregation_of_data_managers_;
	std::map<std::string, GenericCommandForDataManager*> commands_for_data_manager_;
	std::map<std::string, GenericCommandForExtraActions*> commands_for_extra_actions_;
	ScriptPartitioner script_partitioner_;
	CongregationOfDataManagers congregation_of_data_managers_;
	CollectionOfCommandDocumentations collection_of_command_documentations_;
	auxiliaries::ElapsedProcessorTime elapsed_processor_time_;
	bool exit_requested_;
};

}

}

#endif /* SCRIPTING_SCRIPT_EXECUTION_MANAGER_H_ */
