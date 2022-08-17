#version 100
#extension GL_EXT_frag_depth : enable

precision highp float;

uniform int selection_mode_enabled;
uniform int fog_enabled;
uniform vec4 viewport;

varying mat4 VPMT_inverse;
varying mat4 VP_inverse;
varying vec3 centernormclip;

varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;

void main()
{
    vec4 c3 = VPMT_inverse[2];
    vec4 xpPrime = VPMT_inverse*vec4(gl_FragCoord.x, gl_FragCoord.y, 0.0, 1.0);

    float c3TDc3 = dot(c3.xyz, c3.xyz)-c3.w*c3.w;
    float xpPrimeTDc3 = dot(xpPrime.xyz, c3.xyz)-xpPrime.w*c3.w;
    float xpPrimeTDxpPrime = dot(xpPrime.xyz, xpPrime.xyz)-xpPrime.w*xpPrime.w;

    float square = xpPrimeTDc3*xpPrimeTDc3 - c3TDc3*xpPrimeTDxpPrime;
    if (square<0.0)
    {
        discard;
    }
    
    float z = ((-xpPrimeTDc3-sqrt(square))/c3TDc3);
    gl_FragDepthEXT=z;

    vec4 pointclip = VP_inverse*vec4(gl_FragCoord.x, gl_FragCoord.y, z, 1);
    vec3 pointnormclip = vec3(pointclip)/pointclip.w;
    
    if(selection_mode_enabled==0)
    {
    	vec3 fragment_normal=normalize(pointnormclip-centernormclip);
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
        gl_FragColor=vec4(final_color, 1.0);
    }
    else
    {
        gl_FragColor=vec4(fragment_color_for_selection, 1.0);
    }
}

