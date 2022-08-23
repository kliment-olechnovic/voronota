#version 100
precision highp float;

uniform int mode_number;

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

	    vec3 color=mix(full_value.xyz, vec3(0.0, 0.0, 0.0), fogval);
	    gl_FragColor=vec4(color, 1.0);
	    //gl_FragColor=vec4(1.0-fogval, 1.0-fogval, 1.0-fogval, 1.0);
    }
    else
    {
	    gl_FragColor=vec4(full_value.xyz, 1.0);
    }
}
