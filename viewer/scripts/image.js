if(typeof shell !== "function")
{
	throw ("No 'shell' function");
}

if(shell("command -v convert").stdout.trim().length<1)
{
	throw ("No 'convert' executable");
}

image=function(filename)
{
	if(filename===undefined)
	{
		throw ("No file path");
	}
	
	if(shell("command -v convert").stdout.trim().length<1)
	{
		throw ("No 'convert' executable");
	}
	
	var tmp_dir=undefined;
	var terminal_error=undefined;
	
	try
	{
		tmp_dir=shell("mktemp -d").stdout.trim();
		
		if(voronota_screenshot(tmp_dir+'/screen.ppm').results_summary.partial_success!==true)
		{
			throw ("Failed to make screenshot");
		}
		
		if(shell("convert '"+tmp_dir+"/screen.ppm' '"+filename+"'").exit_status!==0 || shell('[ -s "'+filename+'" ]').exit_status!==0)
		{
			throw ("Failed to write image '"+filename+"'");
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

