#version 100
precision highp float;

uniform int fog_enabled;

varying vec2 v_texcoord;

uniform sampler2D screen_texture;

#define PI 3.1415926535897932384626433832795

void main()
{
    vec4 full_value=texture2D(screen_texture, v_texcoord);
    if(fog_enabled==1 && full_value.w<1.0)
    {
    	float central_d=full_value.w;
    	float angle_step_counts=16.0;
    	float angle_step=(2.0*PI)/angle_step_counts;
    	float pit_sum=0.0;
    	for(float i=0.0;i<angle_step_counts;i+=1.0)
    	{
    		float x=cos(angle_step*i);
    		float y=sin(angle_step*i);
    		float min_d=1.0;
    		for(float z=0.0;z<0.1;z+=0.01)
    		{
    			float sx=v_texcoord.x+(x*z);
    			if(sx>=0.0 && sx<=1.0)
    			{
    				float sy=v_texcoord.y+(y*z);
    				if(sy>=0.0 && sy<=1.0)
    				{
    					float d=texture2D(screen_texture, vec2(sx,sy)).w;
    					min_d=min(min_d, d);
    				}
    			}
    		}
    		if(central_d>min_d)
    		{
    			pit_sum+=1.0;
    		}
    	}
	    float pit_val=pit_sum/angle_step_counts;
	    //pit_val=max(0.0, pit_val-0.6);
	    pit_val=1.0/(1.0+exp(10.0-pit_val*10.0));
	    vec3 color=mix(full_value.xyz, vec3(0, 0, 0), pit_val);
	    gl_FragColor=vec4(color, 1.0);
    }
    else
    {
	    gl_FragColor=vec4(full_value.xyz, 1.0);
    }
}
