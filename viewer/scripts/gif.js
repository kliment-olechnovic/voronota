if(typeof shell !== "function")
{
	throw ("No 'shell' function");
}

if(shell("command -v convert").stdout.trim().length<1)
{
	throw ("No 'convert' executable");
}

gif=function(filename, params)
{
	if(filename===undefined)
	{
		throw ("No file path");
	}
	
	if(shell("command -v convert").stdout.trim().length<1)
	{
		throw ("No 'convert' executable");
	}
	
	if(params===undefined)
	{
		params={angle:2, steps:30, delay:2}
	}
	
	params.steps=params.steps-(params.steps%2);
	
	var tmp_dir=undefined;
	var terminal_error=undefined;
	
	try
	{
		tmp_dir=shell("mktemp -d").stdout.trim();
		
		var screenfiles=[];
		
		for(var i=(params.steps/2);i>=1;i--)
		{
			screenfile=(tmp_dir+'/screen'+i+'.ppm');
			screenfiles.push(screenfile);
			voronota_rotate('-axis 0 1 0', '-angle', 0-params.angle);
			voronota_screenshot(screenfile);
		}
		
		for(var i=0;i<5;i++)
		{
			screenfiles.push(screenfiles[screenfiles.length-1]);
		}
		
		for(var i=1;i<=(params.steps/2);i++)
		{
			screenfile=(tmp_dir+'/screen'+i+'.ppm');
			screenfiles.push(screenfile);
			voronota_rotate('-axis 0 1 0', '-angle', params.angle);
		}
		
		for(var i=(params.steps/2+1);i<=params.steps;i++)
		{
			screenfile=(tmp_dir+'/screen'+i+'.ppm');
			screenfiles.push(screenfile);
			voronota_rotate('-axis 0 1 0', '-angle', params.angle);
			voronota_screenshot(screenfile);
		}
		
		for(var i=0;i<5;i++)
		{
			screenfiles.push(screenfiles[screenfiles.length-1]);
		}
		
		for(var i=params.steps;i>=(params.steps/2+1);i--)
		{
			screenfile=(tmp_dir+'/screen'+i+'.ppm');
			screenfiles.push(screenfile);
			voronota_rotate('-axis 0 1 0', '-angle', 0-params.angle);
		}
		
		var convert_script="convert -delay "+params.delay+" "+screenfiles.join(" ")+" -loop 0 "+filename;
		
		var convert_result=shell(convert_script);
		
		if(convert_result.exit_status!==0 || shell('[ -s "'+filename+'" ]').exit_status!==0)
		{
			throw ("Failed to generate '"+filename+"', convert stderr= "+convert_result.stderr);
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

