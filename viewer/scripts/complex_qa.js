params_str=CommandArgs[1];

if(params_str===undefined)
{
	params_str='{}';
}

params=JSON.parse(params_str);

if(params===undefined)
{
	throw ("Failed to parse json input: "+params_str);
}

if(params.input_file===undefined)
{
	throw ("No input file");
}

if(params.cache_dir===undefined)
{
	params.cache_dir='';
}

if(params.restrict_input_atoms===undefined)
{
	params.restrict_input_atoms='[]';
}

if(params.contacts_selection===undefined)
{
	params.contacts_selection='[-min-seq-sep 1]';
}

voronota_import("-file", params.input_file);
voronota_assert_partial_success("Failed to import file");

voronota_restrict_atoms("-use", params.restrict_input_atoms);
voronota_assert_full_success("Failed to restrict input atoms");

voronota_construct_or_load_quality_scores("-cache-dir", params.cache_dir);
voronota_assert_full_success("Failed to compute or load quality scores");

voronota_select_contacts("-use", params.contacts_selection, "-name", "relevant_contacts")
voronota_assert_full_success("Failed to select requested contacts");

voronota_voromqa_local("-global-adj-prefix", "voromqa_light_full");
voronota_assert_full_success("Failed to compute quality scores");

voronota_voromqa_local("-contacts", "[relevant_contacts]", "-global-adj-prefix", "voromqa_light_selected");
voronota_assert_full_success("Failed to compute quality scores");

voronota_voromqa_dark_local("-global-adj-prefix", "voromqa_dark_full");
voronota_assert_full_success("Failed to compute quality scores");

voronota_voromqa_dark_local("-atoms", "[-sel-of-contacts relevant_contacts]", "-global-adj-prefix", "voromqa_dark_selected");
voronota_assert_full_success("Failed to compute quality scores");

voronota_clash_score("-use", "[relevant_contacts]", "-global-adj-prefix", "clash");
voronota_assert_full_success("Failed to compute clash score");

voronota_rename_global_adjunct("voromqa_dark_full_quality_score", "full_dark_score");
voronota_rename_global_adjunct("voromqa_light_full_atoms_quality_score", "full_light_score");
voronota_rename_global_adjunct("voromqa_dark_full_residues_count", "full_residues_count");
voronota_rename_global_adjunct("voromqa_light_full_atoms_count", "full_atoms_count");
voronota_rename_global_adjunct("voromqa_dark_selected_quality_score", "sel_dark_score");
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
   "sel_dark_score", "sel_light_score", "sel_energy", "sel_energy_norm", "sel_residues_count", "sel_atoms_count",
   "sel_contacts_count", "sel_contacts_area", "sel_clash_score"]);
voronota_assert_full_success("Failed to export scores");


