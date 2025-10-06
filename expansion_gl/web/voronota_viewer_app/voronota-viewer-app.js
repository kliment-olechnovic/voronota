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
	const pixel_ratio=1.0;//voronota_viewer_screen_pixel_ratio();
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
	const result=Module.ccall('voronota_viewer_execute_native_script', 'string', ['string'], [str]);
	if(result=="rejected")
	{
		throw new Error("Immediate execution call rejected because some asynchronous downloads are not finished.");
	}
	return JSON.parse(result);
}

function voronota_viewer_get_last_script_output()
{
	return JSON.parse(Module.ccall('voronota_viewer_get_last_script_output', 'string'));;
}

function voronota_viewer_upload_file(name, data, length, parameters)
{
	Module.ccall('voronota_viewer_upload_file', null, ['string', 'number', 'number', 'string'], [name, data, length, parameters]);
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

function voronota_viewer_init(config)
{
	document.addEventListener('keydown', function (e)
	{
		if (e.key === 'F11' || e.code === 'F11' || e.keyCode === 122
			|| e.key === 'F12' || e.code === 'F12' || e.keyCode === 123)
		{
			e.stopImmediatePropagation();
		}
	}, true);
	
	const canvas_id="voronota_viewer_canvas";
	
	if(document.getElementById(canvas_id))
	{
		throw new Error("The canvas '"+canvas_id+"' is already in the DOM.");
	}
	
	var canvas_container=voronota_viewer_return_document_element_or_body(config.canvas_container_id);
	
	(function()
	{
		const scripts = ["4ung.js", "3szd.js", "ensemble.js", "dun2010bbdep.js", "voronota_viewer.js"];
		for (var i = 0; i < scripts.length; i++)
		{
			var script = document.createElement('script');
			script.src = voronota_viewer_app_subdirectory+scripts[i];
			document.body.appendChild(script);
		}
	})();
	
	var canvas = document.createElement('canvas');
	canvas.className = "emscripten";
	canvas.id = canvas_id;
	canvas.oncontextmenu=function(event){event.preventDefault();};
	canvas.style.border = "0px none";
	canvas.style.padding = "0px";
	canvas.style.margin = "0px";
	canvas.style.backgroundColor = "black";
	canvas_container.appendChild(canvas);
	canvas.addEventListener("webglcontextlost", (e) => { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);
	
	Module = {
		preRun: [],
		postRun: [(() =>
		{
			voronota_viewer_setup_js_bindings_to_all_api_functions();
			voronota_viewer_default_width=config.width;
			voronota_viewer_default_height=config.height;
			voronota_viewer_resize_window(config.width, config.height);
			window.addEventListener('resize', function(event){voronota_viewer_resize_window(voronota_viewer_default_width, voronota_viewer_default_height);});
			if(Array.isArray(config.post_init_operations))
			{
				for(var i=0;i<config.post_init_operations.length;i++)
				{
					var single_post_init_operation=config.post_init_operations[i];
					if(typeof single_post_init_operation === "function")
					{
						single_post_init_operation();
					}
					else if(typeof single_post_init_operation === "string")
					{
						voronota_viewer_enqueue_script(single_post_init_operation);
					}
				}
			}
			else
			{
				if(typeof config.post_init_operations === "function")
				{
					config.post_init_operations();
				}
				else if(typeof config.post_init_operations === "string")
				{
					voronota_viewer_enqueue_script(config.post_init_operations);
				}
			}
			if(config.loadscreen_id)
			{
				loadscreen=document.getElementById(config.loadscreen_id);
				if(loadscreen)
				{
					loadscreen.style.display='none';
				}
			}
		})],
		arguments: ['--window-width', '500', '--window-height', '500', '--gui-scaling', ''+voronota_viewer_screen_pixel_ratio()],
		print: ((text) => {	console.log(text);}),
		locateFile: ((s) =>	{return (voronota_viewer_app_subdirectory + s);}),
		canvas: canvas
	};
	
	if(config.bottom_margin)
	{
		voronota_viewer_bottom_margin=config.bottom_margin;
	}
	
	if(config.add_buttons)
	{
		if(!Array.isArray(config.add_buttons))
		{
			throw new Error("Button descriptions not in array.");
		}
		
		for (var i = 0; i < config.add_buttons.length; i++)
		{
			const button_info=config.add_buttons[i];
			if(!button_info.action)
			{
				throw new Error("Missing action in button descriptor "+"JSON.stringify(button_info)"+".");
			}
			if(typeof button_info.action === "function")
			{
				voronota_viewer_add_button_for_custom_action(button_info.action, button_info.label, config.buttons_container_id, config.buttons_style_class);
			}
			else if(typeof button_info.action === "string")
			{
				if(button_info.action=="_input_file")
				{
					voronota_viewer_add_button_for_file_input(button_info.label, config.buttons_container_id, config.buttons_style_class);
				}
				else if(button_info.action=="_input_session")
				{
					voronota_viewer_add_button_for_session_input(button_info.label, config.buttons_container_id, config.buttons_style_class);
				}
				else if(button_info.action=="_paste_and_run")
				{
					voronota_viewer_add_button_for_custom_action(voronota_viewer_paste_and_run_command, button_info.label, config.buttons_container_id, config.buttons_style_class);
				}
				else if(button_info.action=="_hspace")
				{
					voronota_viewer_add_horizontal_spacer(button_info.size, config.buttons_container_id);
				}
				else if(button_info.action=="_vspace")
				{
					voronota_viewer_add_vertical_spacer(button_info.size, config.buttons_container_id);
				}
				else
				{
					voronota_viewer_add_button_for_native_script(button_info.action, button_info.label, config.buttons_container_id, config.buttons_style_class);
				}
			}
			else
			{
				throw new Error("Invalid action in button descriptor "+"JSON.stringify(button_info)"+".");
			}
		}
	}
}

function voronota_viewer_return_document_element_or_body(element_id)
{
	var element=((element_id) ? document.getElementById(element_id) : document.body);
	if(!element)
	{
		throw new Error("The element '"+element_id+"' was not found in the DOM.");
	}
	return element;
}

function voronota_viewer_add_button_for_custom_action(action_function, label, button_container_id, style_class)
{
	var button_container=voronota_viewer_return_document_element_or_body(button_container_id);
	
	var button = document.createElement("button");
	if(style_class)
	{
		button.className=style_class;
	}
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
	
	var button_container=voronota_viewer_return_document_element_or_body(button_container_id);
	
	var file_input = document.createElement("input");
	file_input.type = "file";
	file_input.id = file_input_id;
	file_input.style.display = "none";
	file_input.multiple = true;
	
	const setup_file_reader=function(file)
	{
		var file_reader=new FileReader();
		file_reader.onloadend=function(e){
			var array_buffer=file_reader.result;
			var byte_array=new Uint8Array(array_buffer);
			var buffer=Module._malloc(byte_array.length);
			Module.HEAPU8.set(byte_array, buffer); 
			voronota_viewer_upload_file(file.name, buffer, byte_array.length, '--include-heteroatoms');
			Module._free(buffer);
		}
		file_reader.readAsArrayBuffer(file);
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
		button.className=style_class;
	}
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
	
	var button_container=voronota_viewer_return_document_element_or_body(button_container_id);
	
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
	if(style_class)
	{
		button.className=style_class;
	}
	button.innerHTML = ((label) ? label : "Import local session");
	button.onclick = function() {document.getElementById(session_input_id).click();};
	
	button_container.appendChild(session_input);
	button_container.appendChild(button);
}

function voronota_viewer_add_horizontal_spacer(width, button_container_id)
{
	var button_container=voronota_viewer_return_document_element_or_body(button_container_id);
	
	var spacer = document.createElement("div");
	spacer.style.width = ((width) ? width : 10)+"px";
	spacer.style.display = "inline-block";

	button_container.appendChild(spacer);
}

function voronota_viewer_add_vertical_spacer(height, button_container_id)
{
	var button_container=voronota_viewer_return_document_element_or_body(button_container_id);
	
	var spacer = document.createElement("div");
	spacer.style.height = ((height) ? height : 10)+"px";

	button_container.appendChild(spacer);
}
