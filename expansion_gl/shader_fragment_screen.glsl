#version 100
precision highp float;

uniform int mode_number;
uniform vec4 viewport;

varying vec2 v_texcoord;

uniform sampler2D screen_texture;

float rand(vec2 coords)
{
	return fract(sin(dot(coords.xy, vec2(12.9898,78.233)))*43758.5453);
}

void main()
{
    vec4 full_value=texture2D(screen_texture, v_texcoord);
    if(mode_number==1 && full_value.w<1.0)
    {
    	float central_d=full_value.w;
    	
    	float area_fogval_sum=0.0;
    	float area_fogval_count=0.1;
    	float span=0.1;
    	
    	vec2 coord=v_texcoord;
    	for(int i=0;i<50;i++)
    	{
    		float xv=(rand(vec2(coord.x, coord.y))-0.5)*2.0;
    		float yv=(rand(vec2(0.5*(coord.x+xv), coord.y))-0.5)*2.0;
    		float x=span*xv;
    		float y=span*yv;
    		float sx=v_texcoord.x+x;
    		float sy=v_texcoord.y+y;
    		if(sx>=-1.0 && sx<=1.0 && sy>=-1.0 && sy<=1.0)
    		{
    			coord=vec2(sx, sy);
    			float d=texture2D(screen_texture, coord).w;
			    if(central_d>d)
				{
    				area_fogval_sum+=1.0;
				}
				area_fogval_count+=1.0;
    		}
    	}
    	
    	float fogval=min(area_fogval_sum/area_fogval_count, 1.0)*0.7;
    	fogval+=rand(v_texcoord.xy)*0.15;

	    gl_FragColor=vec4(full_value.xyz, fogval);
    }
    else if(mode_number==2 && full_value.w<1.0)
    {
    	vec2 texel_size=vec2(1.0/viewport.z, 1.0/viewport.w);
    	
    	float sum_values=0.0;
    	float sum_weights=0.001;
    	for(int x=-6;x<=6;x+=2)
    	{
    	    for(int y=-6;y<=6;y+=2)
	    	{
	    		vec2 coord=v_texcoord+(vec2(float(x), float(y))*texel_size);
	    		if(coord.x>=-1.0 && coord.x<=1.0 && coord.y>=-1.0 && coord.y<=1.0)
	    		{
	    			float value=texture2D(screen_texture, coord).w;
	    			if(value<1.0)
	    			{
	    				float weight=1.0;
			    		sum_values+=value*weight;
			    		sum_weights+=weight;
		    		}
	    		}
	    	}
    	}
    	float fogval=sum_values/sum_weights;
    	
    	fogval=pow(fogval, 2.0);
    	
    	gl_FragColor=vec4(mix(full_value.xyz, vec3(0.0, 0.0, 0.0), fogval), 1.0);
    }
    else
    {
	    gl_FragColor=vec4(full_value.xyz, 1.0);
    }
}
