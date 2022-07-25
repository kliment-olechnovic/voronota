#version 100
precision mediump float;
uniform int selection_mode_enabled;
uniform int fog_enabled;
varying vec3 fragment_center_in_ndc;
varying float fragment_radius_in_ndc;
varying vec2 fragment_center_in_screen;
varying float fragment_radius_in_screen;
varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;
void main()
{
	float dist_in_screen=distance(fragment_center_in_screen, gl_FragCoord.xy);
	
    if(dist_in_screen>fragment_radius_in_screen)
	{
		discard;
	}
	
	float offset=dist_in_screen/fragment_radius_in_screen;
	
	float depth_in_ndc=fragment_center_in_ndc.z-sqrt(1.0-offset*offset)*fragment_radius_in_ndc;
	gl_FragDepth=((gl_DepthRange.diff*depth_in_ndc)+gl_DepthRange.near+gl_DepthRange.far)/2.0;
	
    if(selection_mode_enabled==0)
    {
    	vec3 fragment_normal=normalize(vec3((gl_FragCoord.xy-fragment_center_in_screen)/fragment_radius_in_screen, sqrt(1.0-offset*offset)));
        vec3 final_color=fragment_color_for_display;
        if(fragment_adjunct[1]<0.9)
        {
            vec3 light_direction=vec3(0.0, 0.0, 1.0);
            vec3 light_color=vec3(1.0, 1.0, 1.0);
            float ambient_value=0.05;
            vec3 ambient=ambient_value*light_color;
            float diffuse_value=abs(dot(normalize(fragment_normal), normalize(light_direction)));
            vec3 diffuse=diffuse_value*light_color;
            final_color=(ambient+diffuse)*fragment_color_for_display;
        }
        if((fragment_adjunct[0]>0.5) && (mod(floor(gl_FragCoord.x), 2.0)<0.5 || mod(floor(gl_FragCoord.y), 2.0)<0.5))
        {
            final_color=vec3(1.0, 0.0, 1.0);
            if(fragment_color_for_display[0]>0.5 && fragment_color_for_display[1]<0.25 && fragment_color_for_display[2]>0.5)
            {
                final_color=vec3(0.0, 1.0, 0.0);
            }
        }
        gl_FragColor=vec4(final_color, 1.0);
    }
    else
    {
        gl_FragColor=vec4(fragment_color_for_selection, 1.0);
    }
}
