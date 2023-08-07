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
        vec3 light_direction=vec3(0.0, 0.0, 1.0);
        float diffuse_value=abs(dot(normalize(fragment_normal), normalize(light_direction)));
        vec3 final_color=max(0.2, min(0.05+diffuse_value, 1.0))*fragment_color_for_display;
        if(fragment_adjunct[0]>0.5)
        {
            float x_shift=0.0;
            if(mod(gl_FragCoord.y, 8.0)<4.0)
            {
                x_shift=2.0;
            }
            float y_a=1.0;
            float y_b=0.0;
            if(mod(gl_FragCoord.x, 8.0)<4.0)
            {
                y_a=-1.0;
                y_b=3.0;
            }
            if((mod(floor(gl_FragCoord.x+x_shift), 4.0)<0.5 && (mod(floor(gl_FragCoord.y), 4.0)*y_a+y_b)>0.5) || (mod(floor(gl_FragCoord.x+x_shift), 4.0)>0.5 && (mod(floor(gl_FragCoord.y), 4.0)*y_a+y_b)<0.5) || (mod(floor(gl_FragCoord.x+x_shift+2.0), 4.0)==(mod(floor(gl_FragCoord.y+2.0), 4.0)*y_a+y_b)))
            {
                final_color=max(0.4, min(0.4+diffuse_value, 1.0))*vec3(1.0, 0.0, 1.0);
            }
        }
        gl_FragColor=vec4(final_color, gl_FragCoord.z);
    }
    else
    {
        gl_FragColor=vec4(fragment_color_for_selection, 1.0);
    }
}
