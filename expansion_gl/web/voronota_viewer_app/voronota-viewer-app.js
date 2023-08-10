const voronota_viewer_app_subdirectory=(() =>
{
	var path_str = document.currentScript.getAttribute('src');
	if(!path_str)
	{
		return "";
	}
    var url = new URL(path_str, window.location.href);
    var parts = url.pathname.split('/');
    parts.pop();
    return (parts.join('/')+'/');
})();

(function()
{
	const scripts = ["4ung.js", "dun2010bbdep.js", "font.js", "voronota_viewer.js"];
	for (var i = 0; i < scripts.length; i++)
	{
		var script = document.createElement('script');
		script.src = voronota_viewer_app_subdirectory+scripts[i];
		document.body.appendChild(script);
	}
})();

function voronota_viewer_is_retina_display()
{
	if(window.matchMedia)
	{
		const mq=window.matchMedia("only screen and (min--moz-device-pixel-ratio: 1.3), only screen and (-o-min-device-pixel-ratio: 2.6/2), only screen and (-webkit-min-device-pixel-ratio: 1.3), only screen  and (min-device-pixel-ratio: 1.3), only screen and (min-resolution: 1.3dppx)");
		return (mq && mq.matches || (window.devicePixelRatio>1));
    }
    return false;
}

function voronota_viewer_screen_pixel_ratio()
{
	if(voronota_viewer_is_retina_display())
	{
		return 2.0;
	}
	return 1.0;
}

var voronota_viewer_default_width=function()
{
	return null;
}

var voronota_viewer_default_height=function()
{
	return null;
}

var voronota_viewer_bottom_margin=0;

function voronota_viewer_resize_window(width_arg, height_arg)
{
	const width=((width_arg) ? ((typeof width_arg === "function") ? width_arg() : width_arg) : 500);
	const height=((height_arg) ? ((typeof height_arg === "function") ? height_arg() : height_arg) : 500)-voronota_viewer_bottom_margin;
	const new_width=((width<500) ? 500 : width);
	const new_height=((height<500) ? 500 : height);
	const pixel_ratio=voronota_viewer_screen_pixel_ratio();
	Module.ccall('voronota_viewer_resize_window', null, ['int','int'], [new_width*pixel_ratio,new_height*pixel_ratio]);
	Module.canvas.style.setProperty("width", new_width + "px", "important");
	Module.canvas.style.setProperty("height", new_height + "px", "important");
	Module.canvas.width=new_width*pixel_ratio;
	Module.canvas.height=new_height*pixel_ratio;
}

function voronota_viewer_enqueue_script(str)
{
	if(str)
	{
		Module.ccall('voronota_viewer_enqueue_script', null, ['string'], [str]);
	}
}

function voronota_viewer_execute_native_script(str)
{
	return JSON.parse(Module.ccall('voronota_viewer_execute_native_script', 'string', ['string'], [str]));
}

function voronota_viewer_get_last_script_output()
{
	return JSON.parse(Module.ccall('voronota_viewer_get_last_script_output', 'string'));;
}

function voronota_viewer_upload_file(name, data, parameters)
{
	Module.ccall('voronota_viewer_upload_file', null, ['string','string','string'], [name,data,parameters]);
}

function voronota_viewer_upload_session(data, length)
{
	Module.ccall('voronota_viewer_upload_session', null, ['number', 'number'], [data, length]);
}

function voronota_viewer_setup_js_bindings_to_all_api_functions()
{
	Module.ccall('voronota_viewer_setup_js_bindings_to_all_api_functions', 'string');
}

function voronota_viewer_paste_and_run_command()
{
	var command_str=prompt('Paste or enter command', '');
	if(!command_str)
	{
		return;
	}
	voronota_viewer_enqueue_script(command_str);
}

function voronota_viewer_download_file(filename)
{
	if(!filename)
	{
		alert("No file name provided for download.");
		return;
	}
	const path='/'+filename;
	if(!FS.analyzePath(path).exists)
	{
		alert("File '"+path+"' does not exist.");
		return;
	}
	const data=FS.readFile(path, {encoding: 'binary'});
	const blob=new Blob([data], {type: 'application/octet-stream'});
	const url=URL.createObjectURL(blob);
	var a=document.createElement('a');
	a.href=url;
	a.download=filename;
	document.body.appendChild(a);
	a.click();
	document.body.removeChild(a);
}

function voronota_viewer_init(width, height, canvas_container_id, post_init_function_or_script)
{
	const canvas_id=((canvas_container_id) ? ("voronota_viewer_canvas_in_"+canvas_container_id) : "voronota_viewer_canvas");
	
	var canvas_container=((canvas_container_id) ? document.getElementById(canvas_container_id) : document.body);
	if(!canvas_container)
	{
		throw new Error("The canvas container '"+canvas_container_id+"' was not found in the DOM.");
	}
	
	var canvas = document.createElement('canvas');
	canvas.className = "emscripten";
	canvas.id = canvas_id;
	canvas.oncontextmenu=function(event){event.preventDefault();};
	canvas.style.border = "0px none";
	canvas.style.backgroundColor = "black";
	canvas_container.appendChild(canvas);
	canvas.addEventListener("webglcontextlost", (e) => { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);
	
	Module = {
		preRun: [],
		postRun: [(() =>
		{
			voronota_viewer_setup_js_bindings_to_all_api_functions();
			voronota_viewer_default_width=width;
			voronota_viewer_default_height=height;
			voronota_viewer_resize_window(width, height);
			window.addEventListener('resize', function(event){voronota_viewer_resize_window(voronota_viewer_default_width, voronota_viewer_default_height);});
			if(typeof post_init_function_or_script === "function")
			{
				post_init_function_or_script();
			}
			else if(typeof post_init_function_or_script === "string")
			{
				voronota_viewer_enqueue_script(post_init_function_or_script);
			}
		})],
		arguments: ['--window-width', '500', '--window-height', '500', '--gui-scaling', ''+voronota_viewer_screen_pixel_ratio(), '--custom-font-file', 'font.ttf'],
		print: ((text) => {	console.log(text);}),
		locateFile: ((s) =>	{return (voronota_viewer_app_subdirectory + s);}),
		canvas: canvas
	};
}

function voronota_viewer_add_button_for_custom_action(action_function, label, button_container_id, style_class)
{
	var button_container=((button_container_id) ? document.getElementById(button_container_id) : document.body);
	if(!button_container)
	{
		throw new Error("The button container '"+button_container_id+"' was not found in the DOM.");
	}
	
	var button = document.createElement("button");
	button.className = ((style_class) ? style_class : "voronota-viewer-added-button");
	button.innerHTML = ((label) ? label : action_function.name);
	button.onclick = function() {action_function();};
	
	button_container.appendChild(button);
}

function voronota_viewer_add_button_for_native_script(native_script, label, button_container_id, style_class)
{
	const native_script_call=function(){voronota_viewer_enqueue_script(native_script);}
	voronota_viewer_add_button_for_custom_action(native_script_call, label, button_container_id, style_class);
}

function voronota_viewer_add_button_for_file_input(label, button_container_id, style_class)
{
	const file_input_id=((button_container_id) ? ("voronota_viewer_file_input_in_"+button_container_id) : "voronota_viewer_file_input");
	
	if(document.getElementById(file_input_id))
	{
		throw new Error("The the generated file input ID is already present in DOM.");
	}
	
	var button_container=((button_container_id) ? document.getElementById(button_container_id) : document.body);
	if(!button_container)
	{
		throw new Error("The button container '"+button_container_id+"' was not found in the DOM.");
	}
	
	var file_input = document.createElement("input");
	file_input.type = "file";
	file_input.id = file_input_id;
	file_input.style.display = "none";
	file_input.multiple = true;
	
	const setup_file_reader=function(file)
	{
		var file_reader=new FileReader();
		file_reader.onloadend=function(e){voronota_viewer_upload_file(file.name, file_reader.result, '--include-heteroatoms');}
		file_reader.readAsText(file);	
	}
	
	file_input.addEventListener('change', function(e)
	{
		for(var i=0;i<file_input.files.length;i++)
		{
			setup_file_reader(file_input.files[i]);
		}
	});
	
	var button = document.createElement("button");
	button.className = ((style_class) ? style_class : "voronota-viewer-added-button");
	button.innerHTML = ((label) ? label : "Import local structures");
	button.onclick = function() {document.getElementById(file_input_id).click();};
	
	button_container.appendChild(file_input);
	button_container.appendChild(button);
}

function voronota_viewer_add_button_for_session_input(label, button_container_id, style_class)
{
	const session_input_id=((button_container_id) ? ("voronota_viewer_session_input_in_"+button_container_id) : "voronota_viewer_session_input");
	
	if(document.getElementById(session_input_id))
	{
		throw new Error("The the generated session input ID is already present in DOM.");
	}
	
	var button_container=((button_container_id) ? document.getElementById(button_container_id) : document.body);
	if(!button_container)
	{
		throw new Error("The button container '"+button_container_id+"' was not found in the DOM.");
	}
	
	var session_input = document.createElement("input");
	session_input.type = "file";
	session_input.id = session_input_id;
	session_input.style.display = "none";
	session_input.multiple = false;
	
	const setup_session_reader=function(file)
	{
		var session_reader=new FileReader();
		session_reader.onloadend=function(e){
			var array_buffer=session_reader.result;
			var byte_array=new Uint8Array(array_buffer);
			var buffer=Module._malloc(byte_array.length);
			Module.HEAPU8.set(byte_array, buffer); 
			voronota_viewer_upload_session(buffer, byte_array.length);
			Module._free(buffer);
		}
		session_reader.readAsArrayBuffer(file);
	}
	
	session_input.addEventListener('change', function(e)
	{
		for(var i=0;i<session_input.files.length;i++)
		{
			setup_session_reader(session_input.files[i]);
		}
	});
	
	var button = document.createElement("button");
	button.className = ((style_class) ? style_class : "voronota-viewer-added-button");
	button.innerHTML = ((label) ? label : "Import local session");
	button.onclick = function() {document.getElementById(session_input_id).click();};
	
	button_container.appendChild(session_input);
	button_container.appendChild(button);
}

