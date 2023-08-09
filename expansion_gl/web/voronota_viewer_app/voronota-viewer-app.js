function voronota_viewer_get_app_subdirectory_from_script_src()
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
}

var voronota_viewer_app_subdirectory=voronota_viewer_get_app_subdirectory_from_script_src();

function voronota_viewer_is_retina_display()
{
	if(window.matchMedia)
	{
		var mq=window.matchMedia("only screen and (min--moz-device-pixel-ratio: 1.3), only screen and (-o-min-device-pixel-ratio: 2.6/2), only screen and (-webkit-min-device-pixel-ratio: 1.3), only screen  and (min-device-pixel-ratio: 1.3), only screen and (min-resolution: 1.3dppx)");
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

function voronota_viewer_resize_window(width, height)
{
	var pixel_ratio=voronota_viewer_screen_pixel_ratio();
	var new_width=width;
	if(new_width<500)
	{
		new_width=500;
	}
	var new_heigth=height;
	if(new_heigth<500)
	{
		new_heigth=500;
	}
	Module.ccall('voronota_viewer_resize_window', null, ['int','int'], [new_width*pixel_ratio,new_heigth*pixel_ratio]);
	Module.canvas.style.setProperty("width", new_width + "px", "important");
	Module.canvas.style.setProperty("height", new_heigth + "px", "important");
	Module.canvas.width=new_width*pixel_ratio;
	Module.canvas.height=new_heigth*pixel_ratio;
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
	var result=JSON.parse(Module.ccall('voronota_viewer_execute_native_script', 'string', ['string'], [str]));
	return result;
}

function voronota_viewer_get_last_script_output()
{
	var result=JSON.parse(Module.ccall('voronota_viewer_get_last_script_output', 'string'));
	return result;
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
	if(command_str===null || command_str=='')
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
	var path='/'+filename;
	if(!FS.analyzePath(path).exists)
	{
		alert("File '"+path+"' does not exist.");
		return;
	}
	var data=FS.readFile(path, {encoding: 'binary'});
	var blob=new Blob([data], {type: 'application/octet-stream'});
	var url=URL.createObjectURL(blob);
	var a=document.createElement('a');
	a.href=url;
	a.download=filename;
	document.body.appendChild(a);
	a.click();
	document.body.removeChild(a);
}

function voronota_viewer_init(width, height, canvas_container_id, post_init_function)
{
	var canvas = document.createElement('canvas');
	canvas.className = "emscripten";
	canvas.id = "canvas";
	canvas.oncontextmenu=function(event){event.preventDefault();};
	canvas.style.border = "0px none";
	canvas.style.backgroundColor = "black";
	if(canvas_container_id)
	{
		var canvas_container=document.getElementById(canvas_container_id);
		if(canvas_container)
		{
			canvas_container.appendChild(canvas);
		}
		else
		{
			throw new Error("The canvas container '"+canvas_container_id+"' was not found in the DOM.");
		}
	}
	else
	{
		document.body.appendChild(canvas);
	}
	
	var scripts = ["4ung.js", "dun2010bbdep.js", "font.js", "voronota_viewer.js"];
	for (var i = 0; i < scripts.length; i++)
	{
		var script = document.createElement('script');
		script.src = voronota_viewer_app_subdirectory+scripts[i];
		document.body.appendChild(script);
	}
	
	Module = {
		preRun: [],
		postRun: [(function()
		{
			voronota_viewer_resize_window(width, height);
			voronota_viewer_setup_js_bindings_to_all_api_functions();
			if(typeof post_init_function === "function")
			{
				post_init_function();
			}
		})],
		arguments: ['--window-width', '500', '--window-height', '500', '--gui-scaling', ''+voronota_viewer_screen_pixel_ratio(), '--custom-font-file', 'font.ttf'],
		print: (function()
		{
			return function(text)
			{
				console.log(text);
			};
		})(),
		locateFile: (function(s)
		{
			return (voronota_viewer_app_subdirectory + s);
		}),
		canvas: (function()
		{
			var canvas = document.getElementById('canvas');
			canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);
			return canvas;
		})()
	};
}

function voronota_viewer_add_button_for_custom_action(action_function, label, button_container_id, style_class)
{
	var button = document.createElement("button");
	
	if(style_class)
	{
		button.className = style_class;
	}
	else
	{
		button.className = "voronota-viewer-added-button";
	}
	
	if(label)
	{
		button.innerHTML = label;
	}
	else
	{
		button.innerHTML = action_function.name;
	}
	
	button.onclick = function() {action_function();};
	
	if(button_container_id)
	{
		var button_container=document.getElementById(button_container_id);
		if(button_container)
		{
			button_container.appendChild(button);
		}
		else
		{
			throw new Error("The button container '"+button_container_id+"' was not found in the DOM.");
		}
	}
	else
	{
		document.body.appendChild(button);
	}
}

function voronota_viewer_add_button_for_file_input(label, button_container_id, style_class)
{
	if(document.getElementById('voronota_viewer_file_input'))
	{
		throw new Error("The function 'voronota_viewer_add_button_for_file_input' can be called only once.");
	}
	
	var file_input = document.createElement("input");
	file_input.type = "file";
	file_input.id = "voronota_viewer_file_input";
	file_input.style.display = "none";
	file_input.multiple = true;
	
	setup_file_reader=function(file)
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
	if(style_class)
	{
		button.className = style_class;
	}
	else
	{
		button.className = "voronota-viewer-added-button";
	}
	
	if(label)
	{
		button.innerHTML = label;
	}
	else
	{
		button.innerHTML = "Import local structures";
	}
	button.onclick = function() {document.getElementById('voronota_viewer_file_input').click();};
	
	if(button_container_id)
	{
		var button_container=document.getElementById(button_container_id);
		if(button_container)
		{
			button_container.appendChild(file_input);
			button_container.appendChild(button);
		}
		else
		{
			throw new Error("The button container '"+button_container_id+"' was not found in the DOM.");
		}
	}
	else
	{
		document.body.appendChild(file_input);
		document.body.appendChild(button);
	}
}

function voronota_viewer_add_button_for_session_input(label, button_container_id, style_class)
{
	if(document.getElementById('voronota_viewer_session_input'))
	{
		throw new Error("The function 'voronota_viewer_add_button_for_session_input' can be called only once.");
	}
	
	var session_input = document.createElement("input");
	session_input.type = "file";
	session_input.id = "voronota_viewer_session_input";
	session_input.style.display = "none";
	session_input.multiple = false;
	
	setup_session_reader=function(file)
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
	if(style_class)
	{
		button.className = style_class;
	}
	else
	{
		button.className = "voronota-viewer-added-button";
	}
	if(label)
	{
		button.innerHTML = label;
	}
	else
	{
		button.innerHTML = "Import local session";
	}
	button.onclick = function() {document.getElementById('voronota_viewer_session_input').click();};
	
	if(button_container_id)
	{
		var button_container=document.getElementById(button_container_id);
		if(button_container)
		{
			button_container.appendChild(session_input);
			button_container.appendChild(button);
		}
		else
		{
			throw new Error("The button container '"+button_container_id+"' was not found in the DOM.");
		}
	}
	else
	{
		document.body.appendChild(session_input);
		document.body.appendChild(button);
	}
}

