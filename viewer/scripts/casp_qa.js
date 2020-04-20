if(typeof shell !== "function")
{
	throw ("No 'shell' function");
}

casp_qa_init=function(target_sequence_file, model_file, output_prefix, rebuild_side_chains)
{
	if(target_sequence_file===undefined)
	{
		throw ("No target sequence file");
	}
	
	if(model_file===undefined)
	{
		throw ("No model file");
	}
	
	if(output_prefix===undefined)
	{
		throw ("No output prefix");
	}
	
	if(shell('[ -s "'+target_sequence_file+'" ]').exit_status!==0)
	{
		throw ("No target sequence file '"+target_sequence_file+"'");
	}
	
	if(shell('[ -s "'+model_file+'" ]').exit_status!==0)
	{
		throw ("No model file '"+model_file+"'");
	}
	
	shell('mkdir -p "$(dirname '+output_prefix+'_mock)"');
	
	voronota_delete_objects();
	
	voronota_import("-file", model_file, "-format", "pdb");
	voronota_assert_partial_success("Failed to import PDB file");
	
	voronota_list_objects("-picked");
	voronota_assert_full_success("Failed to import PDB file");
	if(voronota_last_output().results[0].output.objects.length!==1)
	{
		throw ("Not one object picked");
	}
	
	model_name=voronota_last_output().results[0].output.objects[0].name;
	
	voronota_set_adjunct_of_atoms_by_sequence_alignment("-name", "refseq", "-sequence-file", target_sequence_file, "-alignment-file", output_prefix+"sequence_alignment");
	voronota_assert_full_success("Failed to set residue sequence number adjunct");
	
	target_sequence_length=voronota_last_output().results[0].output.sequence_length;
	if(target_sequence_length===undefined)
	{
		throw ("Failed to determine target sequence length");
	}
	
	voronota_restrict_atoms_and_renumber_residues_by_adjunct("-name", "refseq");
	voronota_assert_full_success("Failed to renumber residues by adjunct");
	
	if(rebuild_side_chains===true)
	{
		scwrl_and_replace_all();
	}
	
	return model_name;
}

casp_qa_voromqa_dark=function(target_sequence_file, model_file, output_prefix, rebuild_side_chains)
{
	model_name=casp_qa_init(target_sequence_file, model_file, output_prefix, rebuild_side_chains);
	
	voronota_construct_contacts();
	voronota_assert_full_success("Failed to construct contacts");
	
	voronota_voromqa_global();
	voronota_assert_full_success("Failed to calculate basic VoroMQA scores");
	
	voronota_voromqa_dark_global();
	voronota_assert_full_success("Failed to compute scores");
	global_score=voronota_last_output().results[0].output.global_score;
	if(global_score===undefined)
	{
		throw ("Failed to compute global score");
	}
	
	voronota_set_adjunct_of_atoms_by_residue_pooling("-source-name vd1 -destination-name vd1s -pooling-mode min -smoothing-window 3");
	voronota_assert_full_success("Failed to pool and smooth residue adjuncts");
	
	voronota_set_adjunct_of_atoms_by_expression("-expression _reverse_s -input-adjuncts vd1s -parameters 0.5 0.1 0.5 0.2 3.0 -output-adjunct vd1sd");
	voronota_assert_full_success("Failed to transform adjuncts");
	
	voronota_export_adjuncts_of_atoms_as_casp_qa_line("-file", output_prefix+"casp_qa_line", "-adjunct", "vd1sd", "-title", model_name, "-global-score", global_score, "-sequence-length", target_sequence_length, "-scale-by-completeness", 0.85, "-wrap", 20);
	voronota_assert_full_success("Failed to output CASP QA line");
	
	voronota_export_atoms("-file", output_prefix+"scores.pdb", "-pdb-b-factor", "vd1s", "-as-pdb");
	voronota_assert_full_success("Failed to export atoms");
	
	return true;
}

casp_qa_voromqa_light=function(target_sequence_file, model_file, output_prefix, rebuild_side_chains)
{
	model_name=casp_qa_init(target_sequence_file, model_file, output_prefix, rebuild_side_chains);
	
	voronota_construct_contacts();
	voronota_assert_full_success("Failed to construct contacts");
	
	voronota_voromqa_global("-adj-residue-quality", "vl1s", "-smoothing-window", 5);
	voronota_assert_full_success("Failed to calculate basic VoroMQA scores");
	global_score=voronota_last_output().results[0].output.quality_score;
	if(global_score===undefined)
	{
		throw ("Failed to compute global score");
	}
	
	voronota_set_adjunct_of_atoms_by_expression("-expression _reverse_s -input-adjuncts vl1s -parameters 0.3 0.1 0.5 0.2 3.0 -output-adjunct vl1sd");
	voronota_assert_full_success("Failed to transform adjuncts");
	
	voronota_export_adjuncts_of_atoms_as_casp_qa_line("-file", output_prefix+"casp_qa_line", "-adjunct", "vl1sd", "-title", model_name, "-global-score", global_score, "-sequence-length", target_sequence_length, "-scale-by-completeness", 0.85, "-wrap", 20);
	voronota_assert_full_success("Failed to output CASP QA line");
	
	voronota_export_atoms("-file", output_prefix+"scores.pdb", "-pdb-b-factor", "vl1s", "-as-pdb");
	voronota_assert_full_success("Failed to export atoms");
	
	return true;
}

