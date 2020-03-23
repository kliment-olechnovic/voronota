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

