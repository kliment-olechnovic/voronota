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
	
	if(voronota_select_atoms('-on-objects', source_name, '-use', '[-aname CA,C,N,O,OXT]').results_summary.full_success!==true)
	{
		throw ("No protein backbone atoms in source object '"+source_name+"'");
	}
	
	if(shell("command -v Scwrl4").stdout.trim().length<1)
	{
		throw ("No 'Scwrl4' executable");
	}
	
	var tmp_dir=undefined;
	var success=false;
	
	try
	{
		tmp_dir=shell("mktemp -d").stdout.trim();
		
		if(voronota_export_atoms('-on-objects', source_name, '-use', '[-aname CA,C,N,O,OXT]', '-as-pdb', '-file', tmp_dir+'/source.pdb').results_summary.full_success!==true)
		{
			throw ("Failed to export source backbone atoms");
		}
		
		shell("Scwrl4 -h -i "+tmp_dir+"/source.pdb -o "+tmp_dir+"/destination.pdb > /dev/null");
		
		if(voronota_import('-file', tmp_dir+'/destination.pdb', '-title', destination_name).results_summary.partial_success!==true)
		{
			throw ("Invalid generated PDB file");
		}
		
		success=true;
		
		shell("rm -r "+tmp_dir);
		tmp_dir=undefined;
	}
	catch(err)
	{
		if(tmp_dir!==undefined)
		{
			shell("rm -r "+tmp_dir);
		}
		throw ("Failed to use Scwrl4: "+err);
	}
	
	return success;
}

