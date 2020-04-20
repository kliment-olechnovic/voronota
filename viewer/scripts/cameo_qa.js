if(typeof shell !== "function")
{
	throw ("No 'shell' function");
}

cameo_qa_voromqa_dark=function(model_file, output_file)
{
	if(model_file===undefined)
	{
		throw ("No model file");
	}
	
	if(output_file===undefined)
	{
		throw ("No output file");
	}
	
	if(shell('[ -s "'+model_file+'" ]').exit_status!==0)
	{
		throw ("No model file '"+model_file+"'");
	}
	
	shell('mkdir -p "$(dirname '+output_file+')"');
	
	voronota_delete_objects();
	
	voronota_import("-file", model_file, "-format", "pdb");
	voronota_assert_partial_success("Failed to import PDB file");
	
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
	
	voronota_export_atoms("-file", output_file, "-pdb-b-factor", "vd1sd", "-as-pdb");
	voronota_assert_full_success("Failed to export atoms");
	
	writeln("global_score="+global_score);
	
	return true;
}

