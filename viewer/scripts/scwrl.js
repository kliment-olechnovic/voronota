if(typeof shell !== "function")
{
	throw ("No 'shell' function");
}

if(shell("command -v Scwrl4").stdout.trim().length<1)
{
	throw ("No 'Scwrl4' executable");
}

scwrl=function(source_name, destination_name)
{
	if(shell("command -v Scwrl4").stdout.trim().length<1)
	{
		throw ("No 'Scwrl4' executable");
	}
	
	if(source_name===undefined)
	{
		throw ("No source object name");
	}
	
	if(destination_name===undefined)
	{
		throw ("No destination object name");
	}
	
	if(voronota_list_objects(source_name).results_summary.full_success!==true)
	{
		throw ("No source object '"+source_name+"'");
	}
	
	if(voronota_list_objects(destination_name).results_summary.full_success===true)
	{
		throw ("Destination object '"+destination_name+"' already exists");
	}
	
	voronota_select_atoms('-on-objects', source_name, '-use', '[-aname CA,C,N,O,OXT]');
	voronota_assert_full_success("No protein backbone atoms in source object '"+source_name+"'");
	
	var tmp_dir=undefined;
	var terminal_error=undefined;
	
	try
	{
		tmp_dir=shell("mktemp -d").stdout.trim();
		
		voronota_export_atoms('-on-objects', source_name, '-use', '[-aname CA,C,N,O,OXT]', '-as-pdb', '-file', tmp_dir+'/source.pdb');
		voronota_assert_full_success("Failed to export source backbone atoms");
		
		shell("Scwrl4 -h -i "+tmp_dir+"/source.pdb -o "+tmp_dir+"/destination.pdb > /dev/null");
		
		voronota_import('-file', tmp_dir+'/destination.pdb', '-title', destination_name);
		voronota_assert_partial_success("Invalid generated PDB file");
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

scwrl_and_replace=function(source_name)
{
	destination_name=source_name+"__scwrl_tmp__";
	
	scwrl(source_name, destination_name);
	
	voronota_delete_objects(source_name);
	voronota_assert_full_success("Failed to delete object '"+source_name+"'");
	
	voronota_rename_object(destination_name, source_name);
	voronota_assert_full_success("Failed to rename object '"+destination_name+"' to '"+source_name+"'");
	
	return true;
}

scwrl_and_replace_all=function()
{
    voronota_list_objects();
    voronota_assert_full_success("No objects available");
    
    var objects=voronota_last_output().results[0].output.objects;
	
	for(var i=0;i<objects.length;i++)
	{
		scwrl_and_replace(objects[i].name);
	}
	
	return true;
}

