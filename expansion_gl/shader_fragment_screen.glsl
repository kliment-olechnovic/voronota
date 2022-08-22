#version 100
precision highp float;

uniform int fog_enabled;

varying vec2 v_texcoord;

uniform sampler2D screen_texture;

#define PI 3.1415926535897932384626433832795

float rand(vec2 coords)
{
	return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec4 full_value=texture2D(screen_texture, v_texcoord);
    if(fog_enabled==1 && full_value.w<1.0)
    {
    	float central_d=full_value.w;
    	
    	float area_fogval_sum=0.0;
    	float area_fogval_count=0.1;
    	float span=0.05;
    	float span_step=span/5.0;
    	
    	float pattern_value=max(rand(v_texcoord.xy), 0.01);

    	for(float x=-span;x<=span;x+=span_step)
    	{
    		float sx=v_texcoord.x+x;
    		if(sx>=-1.0 && sx<=1.0)
    		{
	    		for(float y=-span;y<=span;y+=span_step)
	    		{
	    			if((x*x+y*y)<=(span*span) && (x*x+y*y)>(span*span*0.0))
	    			{
		    		    float sy=v_texcoord.y+y;
	    				if(sy>=-1.0 && sy<=1.0)
		    			{
		    				if(rand(vec2(sx*pattern_value, sy*(1.0-pattern_value)))>=0.0)
		    				{
			    				float d=texture2D(screen_texture, vec2(sx,sy)).w;
			    				if(central_d>d+0.01)
			    				{
				    				area_fogval_sum+=1.0;
			    				}
			    				area_fogval_count+=1.0;
		    				}
		    			}
	    			}
	    		}
    		}
    	}
    	float fogval=min(area_fogval_sum/area_fogval_count+pattern_value*0.0, 1.0)*0.5;

	    vec3 color=mix(full_value.xyz, vec3(0.0, 0.0, 0.0), fogval);
	    gl_FragColor=vec4(color, 1.0);
	    //gl_FragColor=vec4(1.0-fogval, 1.0-fogval, 1.0-fogval, 1.0);
    }
    else
    {
	    gl_FragColor=vec4(full_value.xyz, 1.0);
    }
}
