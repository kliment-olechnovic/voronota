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
    if(mode_number==10 && full_value.w<1.0)
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
    else if((mode_number==21 || mode_number==22) && full_value.w<1.0)
    {
    	vec2 texel_size=vec2(1.0/viewport.z, 1.0/viewport.w)*2.0;
    	
    	vec2 direction=texel_size;
    	
    	if(mode_number==21)
    	{
    		direction.x=0.0;
    	}
    	else
    	{
    		direction.y=0.0;
    	}
    	
		float coeffs[33];
    	coeffs[0]=0.012318109844189502;                                                                                                                      
		coeffs[1]=0.014381474814203989;                                                                                                                      
		coeffs[2]=0.016623532195728208;                                                                                                                      
		coeffs[3]=0.019024086115486723;                                                                                                                      
		coeffs[4]=0.02155484948872149;                                                                                                                       
		coeffs[5]=0.02417948052890078;                                                                                                                       
		coeffs[6]=0.02685404941667096;                                                                                                                       
		coeffs[7]=0.0295279624870386;                                                                                                                        
		coeffs[8]=0.03214534135442581;
		coeffs[9]=0.03464682117793548;
		coeffs[10]=0.0369716985390341;
		coeffs[11]=0.039060328279673276;
		coeffs[12]=0.040856643282313365;
		coeffs[13]=0.04231065439216247;
		coeffs[14]=0.043380781642569775;
		coeffs[15]=0.044035873841196206;
		coeffs[16]=0.04425662519949865;
		coeffs[17]=0.044035873841196206;
		coeffs[18]=0.043380781642569775;
		coeffs[19]=0.04231065439216247;
		coeffs[20]=0.040856643282313365;
		coeffs[21]=0.039060328279673276;
		coeffs[22]=0.0369716985390341;
		coeffs[23]=0.03464682117793548;
		coeffs[24]=0.03214534135442581;
		coeffs[25]=0.0295279624870386;
		coeffs[26]=0.02685404941667096;
		coeffs[27]=0.02417948052890078;
		coeffs[28]=0.02155484948872149;
		coeffs[29]=0.019024086115486723;
		coeffs[30]=0.016623532195728208;
		coeffs[31]=0.014381474814203989;
		coeffs[32]=0.012318109844189502;
    	
    	float sum_values=0.0;
    	float sum_weights=0.001;
    	for(int i=0;i<33;i++)
    	{
    		vec2 coord=v_texcoord+(direction*float(i-16));
			if(coord.x>=-1.0 && coord.x<=1.0 && coord.y>=-1.0 && coord.y<=1.0)
    		{
    			float value=texture2D(screen_texture, coord).w;
    			if(value<1.0)
    			{
		    		sum_values+=value*coeffs[i];
		    		sum_weights+=coeffs[i];
	    		}
    		}
    	}
    	
    	float fogval=sum_values/sum_weights;
    	    	    	
    	if(mode_number==21)
    	{
    		gl_FragColor=vec4(full_value.xyz, fogval);
    	}
    	else
    	{
    		fogval=pow(fogval, 2.0);
    		gl_FragColor=vec4(mix(full_value.xyz, vec3(0.0, 0.0, 0.0), fogval), 1.0);
    	}
    }
    else
    {
	    gl_FragColor=vec4(full_value.xyz, 1.0);
    }
}
