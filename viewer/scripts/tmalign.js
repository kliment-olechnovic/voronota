if(typeof shell !== "function")
{
	throw ("No 'shell' function");
}

if(shell("command -v TMalign").stdout.trim().length<1)
{
	throw ("No 'TMalign' executable");
}

tmalign=function(target_name, model_name, target_sel, model_sel)
{
	if(target_name===undefined)
	{
		throw ("No target object name");
	}
	
	if(model_name===undefined)
	{
		throw ("No model object name");
	}
	
	if(target_sel===undefined)
	{
		target_sel="[-aname CA]";
	}
	else
	{
		target_sel="(("+target_sel+") and ([-aname CA]))";
	}
	
	if(model_sel===undefined)
	{
		model_sel="[-aname CA]";
	}
	else
	{
		model_sel="(("+model_sel+") and ([-aname CA]))";
	}
	
	if(voronota_list_objects(target_name).results_summary.full_success!==true)
	{
		throw ("No target object '"+target_name+"'");
	}
	
	if(voronota_list_objects(model_name).results_summary.full_success!==true)
	{
		throw ("No model object '"+model_name+"'");
	}
	
	if(voronota_select_atoms('-on-objects', target_name, '-use', target_sel).results_summary.full_success!==true)
	{
		throw ("No target atoms for selection '"+target_sel+"'");
	}
	
	if(voronota_select_atoms('-on-objects', model_name, '-use', model_sel).results_summary.full_success!==true)
	{
		throw ("No model atoms for selection '"+model_sel+"'");
	}
	
	if(shell("command -v TMalign").stdout.trim().length<1)
	{
		throw ("No 'TMalign' executable");
	}
	
	var tmp_dir=undefined;
	var terminal_error=undefined;
	var tmscore=undefined;
	
	try
	{
		tmp_dir=shell("mktemp -d").stdout.trim();
		
		if(voronota_export_atoms('-on-objects', target_name, '-use', target_sel, '-as-pdb', '-file', tmp_dir+'/target.pdb').results_summary.full_success!==true)
		{
			throw ("Failed to export target atoms");
		}
		
		if(voronota_export_atoms('-on-objects', model_name, '-use', model_sel, '-as-pdb', '-file', tmp_dir+'/model.pdb').results_summary.full_success!==true)
		{
			throw ("Failed to export model atoms");
		}
		
		shell("TMalign "+tmp_dir+"/model.pdb "+tmp_dir+"/target.pdb -m "+tmp_dir+"/matrix > "+tmp_dir+"/tmalign.out");
		
		tmscore=shell("cat "+tmp_dir+"/tmalign.out | egrep '^TM-score= ' | grep 'Chain_2' | sed 's/^TM-score=\\s*\\(\\S*\\)\\s*.*/\\1/'").stdout.trim();
		
		if(tmscore.length<1)
		{
			throw ("Invalid TMalign output");
		}
		
		var translation=shell("cat "+tmp_dir+"/matrix | head -5 | tail -3 | awk '{print $2}' | tr '\n' ' '").stdout.trim();
		var rotation=shell("cat "+tmp_dir+"/matrix | head -5 | tail -3 | awk '{print $3 \" \" $4 \" \" $5}' | tr '\n' ' '").stdout.trim();
		
		if(translation.length<1 || rotation.length<1)
		{
			throw ("Invalid TMalign matrix output");
		}
		
		if(voronota_move_atoms('-on-objects', model_name, '-rotate-by-matrix '+rotation, '-translate '+translation).results_summary.full_success!==true)
		{
			throw ("Failed to move atoms");
		}
		
		voronota_zoom_by_atoms('-on-objects', target_name, '-use', target_sel);
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
	
	return tmscore;
}

tmalign_all_on_one=function(target_name, target_sel, model_sel)
{
	if(target_name===undefined)
	{
		throw ("No target object name");
	}
	
    var result_of_list_objects=voronota_list_objects();
    
    if(result_of_list_objects.results_summary.full_success!==true)
	{
		throw ("No objects available");
	}
    
    var objects=result_of_list_objects.results[0].output.objects;
	if(objects.length<2)
	{
		throw ("Less than two objects available");
	}
	
	var results=[];
	for(var i=0;i<objects.length;i++)
	{
		if(objects[i].name!==target_name)
		{
			var result={model_name:"", tmscore:0};
			result.model_name=objects[i].name;
			result.tmscore=tmalign(target_name, result.model_name, target_sel, model_sel);
			results.push(result);
		}
	}
	
	return results;
}

tmalign_all_on_first_one=function(target_and_model_sel)
{
    var result_of_list_objects=voronota_list_objects();
    
    if(result_of_list_objects.results_summary.full_success!==true)
	{
		throw ("No objects available");
	}
    
    var objects=result_of_list_objects.results[0].output.objects;
    
	if(objects.length<2)
	{
		throw ("Less than two objects available");
	}
	
	return tmalign_all_on_one(objects[0].name, target_and_model_sel, target_and_model_sel);
}

