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

function voronota_viewer_init_as_iframe(width, height, post_init_operations, iframe_container_id, new_iframe_id)
{
	var iframe_container=((iframe_container_id) ? document.getElementById(iframe_container_id) : document.body);
	if(!iframe_container)
	{
		throw new Error("The iframe container '"+iframe_container_id+"' was not found in the DOM.");
	}
	
	var iframe = document.createElement('iframe');
	if(new_iframe_id)
	{
		iframe.id=new_iframe_id;
	}
	iframe.width = width+'px';
	iframe.height = height+'px';
	iframe.style.border = 'none';
	
	document.body.appendChild(iframe);
	
	const main_script_path_string=JSON.stringify(voronota_viewer_app_as_iframe_subdirectory+"voronota-viewer-app.js");
	const post_init_operations_string=JSON.stringify(post_init_operations);

	var srcdoc = 
		`<!doctype html>
		<html lang="en-us">
			<head>
				<meta charset="utf-8"><meta http-equiv="Content-Type" content="text/html; charset=utf-8">
				<title>Voronota-GL</title>
				<style>
					body { font-family: arial; width: 100%; height: 100%; margin: 0px; padding: 0px; border: 0px none; display: block; overflow: hidden; }
				</style>
			</head>
			<body>
				<script src=${main_script_path_string}><\/script>
				<script>
					voronota_viewer_init(
						function(){return window.innerWidth;},
						function(){return window.innerHeight;},
						${post_init_operations_string});
				<\/script>
			</body>
		</html>`;
	
	iframe.contentWindow.document.open();
	iframe.contentWindow.document.write(srcdoc);
	iframe.contentWindow.document.close();
	
	return iframe.contentWindow;
}

