#version 100
#extension GL_EXT_frag_depth : enable

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
            vec3 light_color=vec3(1.0, 1.0, 1.0);
            float ambient_value=0.05;
            vec3 ambient=ambient_value*light_color;
            float diffuse_value=abs(dot(normalize(fragment_normal), normalize(light_direction)));
            vec3 diffuse=diffuse_value*light_color;
            final_color=(ambient+diffuse)*fragment_color_for_display;
        }
        if((fragment_adjunct[0]>0.5) && (mod(floor(gl_FragCoord.x), 4.0)<1.5 || mod(floor(gl_FragCoord.y), 4.0)<1.5))
        {
            final_color=vec3(1.0, 0.0, 1.0);
            if(fragment_color_for_display[0]>0.5 && fragment_color_for_display[1]<0.25 && fragment_color_for_display[2]>0.5)
            {
                final_color=vec3(0.0, 1.0, 0.0);
            }
        }
        gl_FragColor=vec4(final_color, gl_FragCoord.z);
    }
    else
    {
        gl_FragColor=vec4(fragment_color_for_selection, 1.0);
    }
}
