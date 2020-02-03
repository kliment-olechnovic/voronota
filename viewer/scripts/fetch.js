if(typeof shell !== "function")
{
	throw ("No 'shell' function");
}

if(shell("command -v curl").stdout.trim().length<1)
{
	throw ("No 'curl' executable");
}

fetch_url=function(file_url, destination_name, as_assembly)
{
	if(file_url===undefined)
	{
		throw ("No file url");
	}
	
	if(destination_name===undefined)
	{
		throw ("No destination object name");
	}
	
	if(as_assembly===undefined)
	{
		as_assembly=false;
	}
	
	if(shell("command -v curl").stdout.trim().length<1)
	{
		throw ("No 'curl' executable");
	}
	
	var need_to_uzip=RegExp('.*\.gz$').test(file_url);
	
	if(need_to_uzip===true && shell("command -v zcat").stdout.trim().length<1)
	{
		throw ("No 'zcat' executable");
	}
	
	var tmp_dir=undefined;
	var success=false;
	
	try
	{
		tmp_dir=shell("mktemp -d").stdout.trim();
		
		if(need_to_uzip===true)
		{
			shell("curl '"+file_url+"' 2> /dev/null | zcat 2> /dev/null >"+tmp_dir+"/destination.pdb");
		}
		else
		{
			shell("curl '"+file_url+"' 2> /dev/null > "+tmp_dir+"/destination.pdb");
		}
		
		if(shell('[ -s "'+tmp_dir+'/destination.pdb" ]').exit_status!==0)
		{
			throw ("No file from URL '"+file_url+"'");
		}
		
		if(voronota_import('-file', tmp_dir+'/destination.pdb', '-title', destination_name, '-as-assembly', as_assembly).results_summary.partial_success!==true)
		{
			throw ("Failed to import PDB file");
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
		throw ("Failed to use download file: "+err);
	}
	
	return success;
}

fetch_pdb=function(pdb_id)
{
	if(pdb_id===undefined)
	{
		throw ("No PDB ID");
	}
	
	var valid_pdb_id=RegExp('^....$').test(pdb_id);
	
	if(valid_pdb_id!==true)
	{
		throw ("Invalid PDB ID '"+pdb_id+"'");
	}
	
	var file_url="https://files.rcsb.org/download/"+pdb_id+".pdb.gz";
	
	return fetch_url(file_url, pdb_id);
}

fetch_pdb_assembly=function(pdb_id, assembly_number)
{
	if(pdb_id===undefined)
	{
		throw ("No PDB ID");
	}
	
	if(assembly_number===undefined)
	{
		throw ("No assembly number");
	}
	
	if((typeof assembly_number !== "number") || assembly_number<1)
	{
		throw ("Invalid assembly number");
	}
	
	var valid_pdb_id=RegExp('^....$').test(pdb_id);
	
	if(valid_pdb_id!==true)
	{
		throw ("Invalid PDB ID '"+pdb_id+"'");
	}
	
	var file_url="https://files.rcsb.org/download/"+pdb_id+".pdb"+assembly_number+".gz";
	
	return fetch_url(file_url, pdb_id+"a"+assembly_number, true);
}

