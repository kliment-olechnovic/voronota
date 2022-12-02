#version 100

precision highp float;
uniform int selection_mode_enabled;
varying vec3 fragment_position;
varying vec3 fragment_normal;
varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;
void main()
{
    if(selection_mode_enabled==0)
    {
        vec3 final_color=fragment_color_for_display;
        if(fragment_adjunct[1]<0.9)
        {
            vec3 light_direction=vec3(0.0, 0.0, 1.0);
            float ambient_value=0.05;
            float diffuse_value=abs(dot(normalize(fragment_normal), normalize(light_direction)));
            final_color=max(0.2, min(ambient_value+diffuse_value, 1.0))*fragment_color_for_display;
        }
        if(fragment_adjunct[0]>0.5 && mod(floor(gl_FragCoord.x*0.75), 2.0)==mod(floor(gl_FragCoord.y*0.75), 2.0))
        {
            final_color=vec3(1.0, 0.0, 1.0);
        }
        gl_FragColor=vec4(final_color, gl_FragCoord.z);
    }
    else
    {
        gl_FragColor=vec4(fragment_color_for_selection, 1.0);
    }
}
