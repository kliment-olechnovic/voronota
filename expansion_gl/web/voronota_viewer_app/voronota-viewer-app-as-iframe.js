const voronota_viewer_app_as_iframe_subdirectory=(() =>
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

function voronota_viewer_init_as_iframe(config)
{
	var iframe=((config.iframe_id) ? document.getElementById(config.iframe_id) : null);
	
	var iframe_is_new=false;
	
	if(!iframe)
	{
		iframe_is_new=true;
		var iframe_container=((config.iframe_container_id) ? document.getElementById(config.iframe_container_id) : document.body);
		if(!iframe_container)
		{
			throw new Error("The iframe container '"+config.iframe_container_id+"' was not found in the DOM.");
		}
		iframe = document.createElement('iframe');
		if(config.iframe_id)
		{
			iframe.id=config.iframe_id;
		}
		iframe_container.appendChild(iframe);
	}
	
	if(config.width)
	{
		iframe.width = config.width+'px';
	}
	
	if(config.height)
	{
		iframe.height = config.height+'px';
	}
	
	if(config.border_style)
	{
		iframe.style.border = config.border_style;
	}
	else if(iframe_is_new)
	{
		iframe.style.border = 'none';
	}
		
	const main_script_path_string=JSON.stringify(voronota_viewer_app_as_iframe_subdirectory+"voronota-viewer-app.js");
	const post_init_operations_string=(config.post_init_operations) ? JSON.stringify(config.post_init_operations) : "null";
	const add_buttons_string=(config.add_buttons) ? JSON.stringify(config.add_buttons) : "null";
	
	var bottom_margin_string=(config.bottom_margin) ? JSON.stringify(config.bottom_margin) : ((config.add_buttons) ? JSON.stringify(30) : "null");
	var canvas_container_id_string=(config.canvas_container_id) ? JSON.stringify(config.canvas_container_id) : "null";
	var buttons_container_id_string=(config.buttons_container_id) ? JSON.stringify(config.buttons_container_id) : "null";
	var buttons_style_class_string=(config.buttons_style_class) ? JSON.stringify(config.buttons_style_class) : "null";
	
	var additional_head_content_string=(config.additional_head_content) ? config.additional_head_content : "";
	var additional_body_content_string=(config.additional_body_content) ? config.additional_body_content : "";
	
	if(config.prettify_buttons_panel)
	{
		bottom_margin_to_use=(config.bottom_margin) ? config.bottom_margin : 30;
		
		additional_head_content_string=`
			<style>
				.voronota-viewer-added-button { background-color: #555555; border: none; color: white; text-align: center; text-decoration: none; display: inline-block; padding-top: 5px; padding-bottom: 5px; padding-left: 10px; padding-right: 10px; margin-top: 0px; margin-bottom: 0px; margin-left: 5px; margin-right: 5px; height: 22px; font-size: 12px; }
				.voronota-viewer-added-button:hover { background-color: #777777; }
			</style>
			`+additional_head_content_string;
		
		bottom_margin_string=JSON.stringify(bottom_margin_to_use);
		canvas_container_id_string="null";
		buttons_container_id_string="null";
		buttons_style_class_string=JSON.stringify("voronota-viewer-added-button");
	}

	var srcdoc = `
		<!doctype html>
		<html lang="en-us">
			<head>
				<meta charset="utf-8"><meta http-equiv="Content-Type" content="text/html; charset=utf-8">
				<title>Voronota-GL</title>
				<style>
					body { font-family: arial; width: 100%; height: 100%; margin: 0px; padding: 0px; border: 0px none; display: block; overflow: hidden; background-color: #333333; }
				</style>
				${additional_head_content_string}
			</head>
			<body>
				${additional_body_content_string}
				<script src=${main_script_path_string}><\/script>
				<script>
					voronota_viewer_init({
						width: function(){return window.innerWidth;},
						height: function(){return window.innerHeight;},
						bottom_margin: ${bottom_margin_string},
						post_init_operations: ${post_init_operations_string},
						add_buttons: ${add_buttons_string},
						canvas_container_id: ${canvas_container_id_string},
						buttons_container_id: ${buttons_container_id_string},
						buttons_style_class: ${buttons_style_class_string},
					});
				<\/script>
			</body>
		</html>
		`;
	
	iframe.contentWindow.document.open();
	iframe.contentWindow.document.write(srcdoc);
	iframe.contentWindow.document.close();
	
	return iframe;
}

