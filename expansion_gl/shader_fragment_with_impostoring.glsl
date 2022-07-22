#version 100
precision mediump float;
uniform int selection_mode_enabled;
uniform int fog_enabled;
varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;
void main()
{
    if(selection_mode_enabled==0)
    {
    	vec3 final_color=fragment_color_for_display;
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
