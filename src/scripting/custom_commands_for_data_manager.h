#ifndef SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_
#define SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_

#include "generic_command_for_data_manager.h"

#include "operators/all.h"

namespace scripting
{

class CustomCommandsForDataManager
{
public:
	typedef GenericCommandForDataManagerFromOperator<operators::AddFigure, true> add_figure;
	typedef GenericCommandForDataManagerFromOperator<operators::AddFigureOfTriangulation, true> add_figure_of_triangulation;
	typedef GenericCommandForDataManagerFromOperator<operators::CalculateBetweenness, true> calculate_betweenness;
	typedef GenericCommandForDataManagerFromOperator<operators::CalculateBurialDepth, true> calculate_burial_depth;
	typedef GenericCommandForDataManagerFromOperator<operators::CenterAtoms, true> center_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ColorAtoms, true> color_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ColorContacts, true> color_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::ColorFigures, true> color_figures;
	typedef GenericCommandForDataManagerFromOperator<operators::ConstructContacts, true> construct_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::ConstructTriangulation, true> construct_triangulation;
	typedef GenericCommandForDataManagerFromOperator<operators::DeleteAdjunctsOfAtoms, true> delete_adjuncts_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::DeleteAdjunctsOfContacts, true> delete_adjuncts_of_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::DeleteFigures, true> delete_figures;
	typedef GenericCommandForDataManagerFromOperator<operators::DeleteSelectionsOfAtoms, true> delete_selections_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::DeleteSelectionsOfContacts, true> delete_selections_of_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::DeleteTagsOfAtoms, true> delete_tags_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::DeleteTagsOfContacts, true> delete_tags_of_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::DescribeExposure, true> describe_exposure;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportAdjunctsOfAtoms, false> export_adjuncts_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportAtomsAndContacts, false> export_atoms_and_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportAtomsAsPymolCGO, false> export_atoms_as_pymol_cgo;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportAtoms, false> export_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportCartoonAsPymolCGO, false> export_cartoon_as_pymol_cgo;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportContactsAsPymolCGO, false> export_contacts_as_pymol_cgo;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportContacts, false> export_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportFiguresAsPymolCGO, false> export_figures_as_pymol_cgo;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportSelectionOfAtoms, false> export_selection_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportSelectionOfContacts, false> export_selection_of_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportTriangulation, false> export_triangulation;
	typedef GenericCommandForDataManagerFromOperator<operators::ExportTriangulationVoxels, false> export_triangulation_voxels;
	typedef GenericCommandForDataManagerFromOperator<operators::FindConnectedComponents, true> find_connected_components;
	typedef GenericCommandForDataManagerFromOperator<operators::HideAtoms, true> hide_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::HideContacts, true> hide_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::HideFigures, true> hide_figures;
	typedef GenericCommandForDataManagerFromOperator<operators::ImportAdjunctsOfAtoms, true> import_adjuncts_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ImportContacts, false> import_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::ImportSelectionOfAtoms, true> import_selection_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ImportSelectionOfContacts, true> import_selection_of_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::ListFigures, true> list_figures;
	typedef GenericCommandForDataManagerFromOperator<operators::ListSelectionsOfAtoms, true> list_selections_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ListSelectionsOfContacts, true> list_selections_of_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::MakeDrawableContacts, true> make_drawable_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::MakeUndrawableContacts, true> make_undrawable_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::MarkAtoms, true> mark_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::MarkContacts, true> mark_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::MoveAtoms, true> move_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::PrintAtoms, true> print_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::PrintContacts, true> print_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::PrintFigures, true> print_figures;
	typedef GenericCommandForDataManagerFromOperator<operators::PrintSequence, true> print_sequence;
	typedef GenericCommandForDataManagerFromOperator<operators::PrintTextDescription, true> print_text_description;
	typedef GenericCommandForDataManagerFromOperator<operators::PrintTriangulation, true> print_triangulation;
	typedef GenericCommandForDataManagerFromOperator<operators::RenameSelectionOfAtoms, true> rename_selection_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::RenameSelectionOfContacts, true> rename_selection_of_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::RestrictAtoms, true> restrict_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::SelectAtomsByTriangulationQuery, true> select_atoms_by_triangulation_query;
	typedef GenericCommandForDataManagerFromOperator<operators::SelectAtoms, true> select_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::SelectContacts, true> select_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::SetAdjunctOfAtomsByContactAdjuncts, true> set_adjunct_of_atoms_by_contact_adjuncts;
	typedef GenericCommandForDataManagerFromOperator<operators::SetAdjunctOfAtomsByContactAreas, true> set_adjunct_of_atoms_by_contact_areas;
	typedef GenericCommandForDataManagerFromOperator<operators::SetAdjunctOfAtomsByResiduePooling, true> set_adjunct_of_atoms_by_residue_pooling;
	typedef GenericCommandForDataManagerFromOperator<operators::SetAdjunctOfAtoms, true> set_adjunct_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::SetAdjunctOfContacts, true> set_adjunct_of_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::SetTagOfAtomsBySecondaryStructure, true> set_tag_of_atoms_by_secondary_structure;
	typedef GenericCommandForDataManagerFromOperator<operators::SetTagOfAtoms, true> set_tag_of_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::SetTagOfContacts, true> set_tag_of_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::SetTextDescription, true> set_text_description;
	typedef GenericCommandForDataManagerFromOperator<operators::ShowAtoms, true> show_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ShowContacts, true> show_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::ShowFigures, true> show_figures;
	typedef GenericCommandForDataManagerFromOperator<operators::SpectrumAtoms, true> spectrum_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::SpectrumContacts, true> spectrum_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::UnmarkAtoms, true> unmark_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::UnmarkContacts, true> unmark_contacts;
	typedef GenericCommandForDataManagerFromOperator<operators::VoroMQAFrustration, true> voromqa_frustration;
	typedef GenericCommandForDataManagerFromOperator<operators::VoroMQAGlobal, true> voromqa_global;
	typedef GenericCommandForDataManagerFromOperator<operators::VoroMQALocal, true> voromqa_local;
	typedef GenericCommandForDataManagerFromOperator<operators::VoroMQAMembranePlace, true> voromqa_membrane_place;
	typedef GenericCommandForDataManagerFromOperator<operators::ZoomByAtoms, false> zoom_by_atoms;
	typedef GenericCommandForDataManagerFromOperator<operators::ZoomByContacts, false> zoom_by_contacts;
};

}

#endif /* SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_ */
