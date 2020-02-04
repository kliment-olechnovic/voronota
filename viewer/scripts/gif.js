if(typeof shell !== "function")
{
	throw ("No 'shell' function");
}

if(shell("command -v convert").stdout.trim().length<1)
{
	throw ("No 'convert' executable");
}

gif_rock=function(filename, params)
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
		params={}
	}
	
	if(params.angle===undefined)
	{
		params.angle=2;
	}
	
	if(params.frames===undefined)
	{
		params.frames=40;
	}
	
	if(params.delay===undefined)
	{
		params.delay=2;
	}
	
	if(params.end_stop_frames===undefined)
	{
		params.end_stop_frames=2;
	}
	
	params.frames=params.frames-(params.frames%2);
	
	var tmp_dir=undefined;
	var pending_gui=false;
	var terminal_error=undefined;
	
	try
	{
		tmp_dir=shell("mktemp -d").stdout.trim();
		
		voronota_configure_gui_push();
		pending_gui=true;
		voronota_configure_gui_disable_widgets();
		
		var screenfiles=[];
		
		var first_screenfile=(tmp_dir+'/first_screen.ppm');
		voronota_screenshot(first_screenfile);
		screenfiles.push(first_screenfile);
		
		for(var i=(params.frames/2);i>=1;i--)
		{
			var screenfile=(tmp_dir+'/screen'+i+'.ppm');
			screenfiles.push(screenfile);
			voronota_rotate('-axis 0 1 0', '-angle', 0-params.angle);
			voronota_screenshot(screenfile);
		}
		
		for(var i=0;i<params.end_stop_frames;i++)
		{
			screenfiles.push(screenfiles[screenfiles.length-1]);
		}
		
		for(var i=1;i<=(params.frames/2);i++)
		{
			var screenfile=(tmp_dir+'/screen'+i+'.ppm');
			screenfiles.push(screenfile);
			voronota_rotate('-axis 0 1 0', '-angle', params.angle);
		}
		
		screenfiles.push(first_screenfile);
		
		for(var i=(params.frames/2+1);i<=params.frames;i++)
		{
			var screenfile=(tmp_dir+'/screen'+i+'.ppm');
			screenfiles.push(screenfile);
			voronota_rotate('-axis 0 1 0', '-angle', params.angle);
			voronota_screenshot(screenfile);
		}
		
		for(var i=0;i<params.end_stop_frames;i++)
		{
			screenfiles.push(screenfiles[screenfiles.length-1]);
		}
		
		for(var i=params.frames;i>=(params.frames/2+1);i--)
		{
			var screenfile=(tmp_dir+'/screen'+i+'.ppm');
			screenfiles.push(screenfile);
			voronota_rotate('-axis 0 1 0', '-angle', 0-params.angle);
		}
		
		voronota_configure_gui_pop();
		pending_gui=false;
		
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
	
	if(pending_gui)
	{
		voronota_configure_gui_pop();
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

gif_spin=function(filename, angle, delay)
{
	if(filename===undefined)
	{
		throw ("No file path");
	}
	
	if(shell("command -v convert").stdout.trim().length<1)
	{
		throw ("No 'convert' executable");
	}
	
	if(angle===undefined)
	{
		angle=2;
	}
	
	if(delay===undefined)
	{
		delay=2;
	}
	
	var tmp_dir=undefined;
	var pending_gui=false;
	var terminal_error=undefined;
	
	try
	{
		tmp_dir=shell("mktemp -d").stdout.trim();
		
		var screenfiles=[];
		
		voronota_configure_gui_push();
		pending_gui=true;
		
		voronota_configure_gui_disable_widgets();
		
		for(var a=0;a<360;a+=angle)
		{
			var screenfile=(tmp_dir+'/screen'+a+'.ppm');
			screenfiles.push(screenfile);
			voronota_screenshot(screenfile);
			voronota_rotate('-axis 0 1 0', '-angle', angle);
		}
		
		voronota_configure_gui_pop();
		pending_gui=false;
		
		var convert_script="convert -delay "+delay+" "+screenfiles.join(" ")+" -loop 0 "+filename;
		
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
	
	if(pending_gui)
	{
		voronota_configure_gui_pop();
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

