#version 100
precision mediump float;
uniform int selection_mode_enabled;
uniform int fog_enabled;
varying vec3 fragment_position;
varying vec3 fragment_normal;
varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;
void main()
{
    if(selection_mode_enabled==0)
    {
        vec3 light_direction=vec3(0.0, 0.0, 1.0);
        vec3 light_color=vec3(1.0, 1.0, 1.0);
        float ambient_value=0.05;
        vec3 ambient=ambient_value*light_color;
        float diffuse_value=abs(dot(normalize(fragment_normal), normalize(light_direction)));
        vec3 diffuse=diffuse_value*light_color;
        vec3 final_color=(ambient+diffuse)*fragment_color_for_display;
        if(fog_enabled==1)
        {
            float fog_density=1.0/(1.0+exp(0.1*(fragment_position.z+0.0)));
            final_color=mix(final_color, vec3(1.0, 1.0, 1.0), fog_density);
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
