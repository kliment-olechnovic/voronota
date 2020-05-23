param_cache_dir=CommandArgs[1];
param_input_file=CommandArgs[2];
param_restrict_input_atoms='[]';
param_contacts_selection='[-inter-chain]';

voronota_import("-file", param_input_file);
voronota_assert_partial_success("Failed to import file");

voronota_restrict_atoms("-use", param_restrict_input_atoms);
voronota_assert_full_success("Failed to restrict input atoms");

voronota_construct_or_load_quality_scores("-cache-dir", param_cache_dir);
voronota_assert_full_success("Failed to compute or load quality scores");

voronota_voromqa_local("-global-adj-prefix", "voromqa_light_full");
voronota_assert_full_success("Failed to compute quality scores");

voronota_voromqa_local("-contacts", param_contacts_selection, "-global-adj-prefix", "voromqa_light_selected");
voronota_assert_full_success("Failed to compute quality scores");

voronota_voromqa_dark_local("-global-adj-prefix", "voromqa_dark_full");
voronota_assert_full_success("Failed to compute quality scores");

voronota_clash_score("-use", param_contacts_selection, "-global-adj-prefix", "clash");
voronota_assert_full_success("Failed to compute clash score");

voronota_rename_global_adjunct("voromqa_dark_full_quality_score", "full_dark_score");
voronota_rename_global_adjunct("voromqa_light_full_atoms_quality_score", "full_light_score");
voronota_rename_global_adjunct("voromqa_dark_full_residues_count", "full_residues_count");
voronota_rename_global_adjunct("voromqa_light_full_atoms_count", "full_atoms_count");
voronota_rename_global_adjunct("voromqa_light_selected_atoms_quality_score", "sel_light_score");
voronota_rename_global_adjunct("voromqa_light_selected_contacts_pseudo_energy", "sel_energy");
voronota_rename_global_adjunct("voromqa_light_selected_contacts_pseudo_energy_norm", "sel_energy_norm");
voronota_rename_global_adjunct("voromqa_light_selected_residues_count", "sel_residues_count");
voronota_rename_global_adjunct("voromqa_light_selected_atoms_count", "sel_atoms_count");
voronota_rename_global_adjunct("voromqa_light_selected_contacts_count", "sel_contacts_count");
voronota_rename_global_adjunct("voromqa_light_selected_contacts_area", "sel_contacts_area");
voronota_rename_global_adjunct("clash_score", "sel_clash_score");

voronota_export_global_adjuncts("-file", "_stdout", "-adjuncts",
  ["full_dark_score", "full_light_score", "full_residues_count", "full_atoms_count",
   "sel_light_score", "sel_energy", "sel_energy_norm", "sel_residues_count", "sel_atoms_count",
   "sel_contacts_count", "sel_contacts_area", "sel_clash_score"]);
voronota_assert_full_success("Failed to export scores");


