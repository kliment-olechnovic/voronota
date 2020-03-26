if(typeof shell !== "function")
{
	throw ("No 'shell' function");
}

if(shell("command -v nnport-normalize").stdout.trim().length<1)
{
	throw ("No 'nnport-normalize' executable");
}

if(shell("command -v nnport-predict").stdout.trim().length<1)
{
	throw ("No 'nnport-predict' executable");
}

nnport_voromqa_dark=function(nnport_data_directory)
{
	if(nnport_data_directory===undefined)
	{
		throw ("No nnport data directory");
	}
	
	if(shell("command -v nnport-normalize").stdout.trim().length<1)
	{
		throw ("No 'nnport-normalize' executable");
	}
	
	if(shell("command -v nnport-predict").stdout.trim().length<1)
	{
		throw ("No 'nnport-predict' executable");
	}
	
	if(shell('[ -d "'+nnport_data_directory+'" ]').exit_status!==0)
	{
		throw ("No nnport data directory '"+nnport_data_directory+"'");
	}
	
	nnport_header_file=nnport_data_directory+"/nnport_input_header";
	nnport_statistics_file=nnport_data_directory+"/nnport_input_statistics";
	nnport_fdeep_model_file=nnport_data_directory+"/nnport_input_fdeep_model.json";
	
	var tmp_dir=undefined;
	var terminal_error=undefined;
	
	try
	{
		tmp_dir=shell("mktemp -d").stdout.trim();
		
		profile_file=tmp_dir+"/profile";
		ids_file=tmp_dir+"/ids";
		scores_file=tmp_dir+"/scores";
		adjuncts_file=tmp_dir+"/adjuncts";
		
		if(voronota_construct_contacts().results_summary.full_success!==true)
		{
			throw ("Failed to construct contacts");
		}
		
		if(voronota_voromqa_global().results_summary.full_success!==true)
		{
			throw ("Failed to calculate basic VoroMQA scores");
		}
		
		if(voronota_generate_residue_voromqa_energy_profile("-file", profile_file).results_summary.full_success!==true)
		{
			throw ("Failed to generate residue VoroMQA energy profile");
		}
		
		header=fread(nnport_header_file).trim();
		
		shell("cat '"+profile_file+"' | nnport-normalize ID > "+ids_file);
		
		shell("cat '"+profile_file+"' | nnport-normalize '"+header+"' '"+nnport_statistics_file+"' | nnport-predict '"+nnport_fdeep_model_file+"' > "+scores_file);
		
		shell("(echo 'ID vd1 vd2 vd3 vd4 vd5 vd6' ; paste '"+ids_file+"' '"+scores_file+"') | column -t > "+adjuncts_file);
		
		if(voronota_import_adjuncts_of_atoms(adjuncts_file).results_summary.full_success!==true)
		{
			throw ("Failed to import score adjuncts");
		}
	}
	catch(err)
	{
		terminal_error=err;
	}
	
	if(tmp_dir!==undefined)
	{
		shell("rm -r "+tmp_dir);
	}
	
	if(terminal_error!==undefined)
	{
		throw terminal_error;
	}
	
	return true;
}

nnport_voromqa_dark_for_casp=function(nnport_data_directory, target_sequence_file, model_file, output_prefix)
{
	if(nnport_data_directory===undefined)
	{
		throw ("No nnport data directory");
	}
	
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
	
	if(shell('[ -d "'+nnport_data_directory+'" ]').exit_status!==0)
	{
		throw ("No nnport data directory '"+nnport_data_directory+"'");
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
	
	lastres=voronota_import("-file", model_file, "-format", "pdb");
	if(lastres.results_summary.partial_success!==true)
	{
		throw ("Failed to import PDB file");
	}
	
	lastres=voronota_list_objects("-picked");
	if(lastres.results_summary.full_success!==true || lastres.results[0].output.objects.length!==1)
	{
		throw ("Not one object picked");
	}
	
	model_name=lastres.results[0].output.objects[0].name;
	
	lastres=voronota_set_adjunct_of_atoms_by_sequence_alignment("-name", "refseq", "-sequence-file", target_sequence_file, "-alignment-file", output_prefix+"sequence_alignment");
	if(lastres.results_summary.full_success!==true)
	{
		throw ("Failed to set residue sequence number adjunct");
	}
	
	target_sequence_length=lastres.results[0].output.sequence_length;
	if(target_sequence_length===undefined)
	{
		throw ("Failed to determine target sequence length");
	}
	
	lastres=voronota_restrict_atoms_and_renumber_residues_by_adjunct("-name", "refseq");
	if(lastres.results_summary.full_success!==true)
	{
		throw ("Failed to renumber residues by adjunct");
	}
	
	nnport_voromqa_dark(nnport_data_directory);
	
	lastres=voronota_spectrum_atoms("-use [-aname CA] -adjunct vd1 -only-summarize");
	if(lastres.results_summary.full_success!==true)
	{
		throw ("Failed to summarize adjuncts");
	}
	
	global_score=lastres.results[0].output.spectrum_summary.mean_of_values;
	if(global_score===undefined)
	{
		throw ("Failed to compute global score");
	}
	
	lastres=voronota_set_adjunct_of_atoms_by_residue_pooling("-source-name vd1 -destination-name vd1s -pooling-mode min -smoothing-window 2");
	if(lastres.results_summary.full_success!==true)
	{
		throw ("Failed to pool and smooth residue adjuncts");
	}
	
	lastres=voronota_set_adjunct_of_atoms_by_expression("-expression _reverse_s -input-adjuncts vd1s -parameters 0.5 0.2 0.5 0.2 3 -output-adjunct vd1sd");
	if(lastres.results_summary.full_success!==true)
	{
		throw ("Failed to transform adjuncts");
	}
	
	lastres=voronota_export_adjuncts_of_atoms_as_casp_qa_line("-file", output_prefix+"casp_qa_line", "-adjunct", "vd1sd", "-title", model_name, "-global-score", global_score, "-sequence-length", target_sequence_length, "-scale-by-completeness", 0.85, "-wrap", 20);
	
	if(lastres.results_summary.full_success!==true)
	{
		throw ("Failed to output CASP QA line");
	}
	
	return true;
}

