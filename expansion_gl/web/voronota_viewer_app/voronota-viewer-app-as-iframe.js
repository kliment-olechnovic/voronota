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
		document.body.appendChild(iframe);
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
	const bottom_margin_string=(config.bottom_margin) ? JSON.stringify(config.bottom_margin) : "null";
	const add_buttons_string=(config.add_buttons) ? JSON.stringify(config.add_buttons) : "null";
	const canvas_container_id_string=(config.canvas_container_id) ? JSON.stringify(config.canvas_container_id) : "null";
	const buttons_container_id_string=(config.buttons_container_id) ? JSON.stringify(config.buttons_container_id) : "null";
	const buttons_style_class_string=(config.buttons_style_class) ? JSON.stringify(config.buttons_style_class) : "null";
	
	const additional_head_content_string=(config.additional_head_content) ? config.additional_head_content : "";
	const additional_body_content_string=(config.additional_body_content) ? config.additional_body_content : "";

	var srcdoc = 
		`<!doctype html>
		<html lang="en-us">
			<head>
				<meta charset="utf-8"><meta http-equiv="Content-Type" content="text/html; charset=utf-8">
				<title>Voronota-GL</title>
				<style>
					body { font-family: arial; width: 100%; height: 100%; margin: 0px; padding: 0px; border: 0px none; display: block; overflow: hidden; }
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
		</html>`;
	
	iframe.contentWindow.document.open();
	iframe.contentWindow.document.write(srcdoc);
	iframe.contentWindow.document.close();
	
	return iframe;
}

