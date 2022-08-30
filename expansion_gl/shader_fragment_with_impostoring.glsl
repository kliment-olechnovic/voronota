#version 100
#extension GL_EXT_frag_depth : enable

precision highp float;

uniform int selection_mode_enabled;
uniform vec4 viewport;

varying mat4 VPMT_inverse;
varying mat4 VP_inverse;
varying vec3 centernormclip;

varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;

// Some info about the reused ideas and code:
//   The sphere impostoring code is adapted from
//     https://github.com/ssloy/glsltuto/blob/master/shaders/ (by Dmitry V. Sokolov, no license stated)
//     who used the paper "GPU-Based Ray-Casting of Quadratic Surfaces" (http://dl.acm.org/citation.cfm?id=2386396) by Christian Sigg, Tim Weyrich, Mario Botsch, Markus Gross.

void main()
{
	if(gl_FragCoord.x<viewport.x || gl_FragCoord.x>(viewport.x+viewport.z) || gl_FragCoord.y<viewport.y || gl_FragCoord.y>(viewport.y+viewport.w))
	{
		discard;
	}
	
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
            float ambient_value=0.05;
            float diffuse_value=abs(dot(normalize(fragment_normal), normalize(light_direction)));
            final_color=max(0.2, min(ambient_value+diffuse_value, 1.0))*fragment_color_for_display;
        }
        if((fragment_adjunct[0]>0.5) && (mod(floor(gl_FragCoord.x), 2.0)<0.5 || mod(floor(gl_FragCoord.y), 2.0)<0.5))
        {
            final_color=vec3(1.0, 0.0, 1.0);
            if(fragment_color_for_display[0]>0.5 && fragment_color_for_display[1]<0.25 && fragment_color_for_display[2]>0.5)
            {
                final_color=vec3(0.0, 1.0, 0.0);
            }
        }
        gl_FragColor=vec4(final_color, gl_FragDepthEXT);
    }
    else
    {
        gl_FragColor=vec4(fragment_color_for_selection, gl_FragDepthEXT);
    }
}

