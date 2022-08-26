#version 100
precision highp float;

uniform int mode_number;
uniform vec4 viewport;

varying vec2 v_texcoord;

uniform sampler2D screen_texture;

// Some info about the reused ideas and code:
//   Pseudorandom numbers generator code (rand function) idea is likely from
//     the paper "On generating random numbers, with help of y= [(a+x)sin(bx)] mod 1", W.J.J. Rey, 22nd European Meeting of Statisticians and the 7th Vilnius Conference on Probability Theory and Mathematical Statistics, August 1998
//   SSAO code (ssao_* functions) idea is based on the description of the first SSAO that was developed by Vladimir Kajalin while working at Crytek and was used for the first time in 2007 by the video game Crysis
//     as written in https://en.wikipedia.org/wiki/Screen_space_ambient_occlusion
//   FXAA code (fxaa_* functions) is adapted from
//     https://github.com/molstar/molstar/blob/master/src/mol-gl/shader/fxaa.frag.ts (The MIT License Copyright (c) 2017 - now, Mol* contributors)
//     who adapted it from
//     https://github.com/kosua20/Rendu/blob/master/resources/common/shaders/screens/fxaa.frag (MIT License Copyright (c) 2017 Simon Rodriguez)

float rand(vec2 coords)
{
	return fract(sin(dot(coords.xy, vec2(12.9898,78.233)))*43758.5453);
}

float ssao_noisy(vec4 full_value)
{
    float central_d=full_value.w;
	
	float area_fogval_sum=0.0;
	float area_fogval_count=0.1;
	float span=0.1;
	
	vec2 coord=v_texcoord;
	for(int i=0;i<10;i++)
	{
		float xv=(rand(vec2(coord.x, coord.y))-0.5)*2.0;
		float yv=(rand(vec2(0.5*(coord.x+xv), coord.y))-0.5)*2.0;
		float x=span*xv;
		float y=span*yv;
		float sx=v_texcoord.x+x;
		float sy=v_texcoord.y+y;
		//if(sx>=0.0 && sx<=1.0 && sy>=0.0 && sy<=1.0)
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

    return fogval;
}

float ssao_blur_pass(vec4 full_value, int pass_num)
{
	vec2 texel_size=vec2(1.0/viewport.z, 1.0/viewport.w)*2.0;
	
	vec2 direction=texel_size;
	
	if(pass_num==1)
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
		//if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
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

	return fogval;
}

float fxaa_quality(int q)
{
	return (q<5 ? 1.0 : (q>5 ? (q<10 ? 2.0 : (q<11 ? 4.0 : 8.0)) : 1.5));
}

float fxaa_rgb_to_luma(vec3 rgb)
{
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

float fxaa_sample_luma(vec2 coords)
{
    return fxaa_rgb_to_luma(texture2D(screen_texture, coords).rgb);
}

float fxaa_sample_luma(vec2 coords, float uOffset, float vOffset)
{
    coords+=vec2(1.0/viewport.z, 1.0/viewport.w)*vec2(uOffset, vOffset);
    return fxaa_sample_luma(coords);
}

vec4 fxaa_compute_color(vec4 colorCenter)
{
	float dEdgeThresholdMin=0.0312;
	float dEdgeThresholdMax=0.063;
	float dSubpixelQuality=0.30;

	vec2 coords = v_texcoord;

	// Luma at the current fragment
	float lumaCenter = fxaa_rgb_to_luma(colorCenter.rgb);

	// Luma at the four direct neighbours of the current fragment.
	float lumaDown = fxaa_sample_luma(coords, 0.0, -1.0);
	float lumaUp = fxaa_sample_luma(coords, 0.0, 1.0);
	float lumaLeft = fxaa_sample_luma(coords, -1.0, 0.0);
	float lumaRight = fxaa_sample_luma(coords, 1.0, 0.0);

	// Find the maximum and minimum luma around the current fragment.
	float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
	float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));

	// Compute the delta.
	float lumaRange = lumaMax - lumaMin;

	// If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.
	if (lumaRange < max(dEdgeThresholdMin, lumaMax * dEdgeThresholdMax))
	{
		return colorCenter;
	}

	// Query the 4 remaining corners lumas.
	float lumaDownLeft = fxaa_sample_luma(coords, -1.0, -1.0);
	float lumaUpRight = fxaa_sample_luma(coords, 1.0, 1.0);
	float lumaUpLeft = fxaa_sample_luma(coords, -1.0, 1.0);
	float lumaDownRight = fxaa_sample_luma(coords, 1.0, -1.0);

	// Combine the four edges lumas (using intermediary variables for future computations with the same values).
	float lumaDownUp = lumaDown + lumaUp;
	float lumaLeftRight = lumaLeft + lumaRight;

	// Same for corners
	float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
	float lumaDownCorners = lumaDownLeft + lumaDownRight;
	float lumaRightCorners = lumaDownRight + lumaUpRight;
	float lumaUpCorners = lumaUpRight + lumaUpLeft;

	// Compute an estimation of the gradient along the horizontal and vertical axis.
	float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
	float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 + abs(-2.0 * lumaDown + lumaDownCorners);

	// Is the local edge horizontal or vertical ?
	bool isHorizontal = (edgeHorizontal >= edgeVertical);

	vec2 inverseScreenSize=vec2(1.0/viewport.z, 1.0/viewport.w);

	// Choose the step size (one pixel) accordingly.
	float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

	// Select the two neighboring texels lumas in the opposite direction to the local edge.
	float luma1 = isHorizontal ? lumaDown : lumaLeft;
	float luma2 = isHorizontal ? lumaUp : lumaRight;

	// Compute gradients in this direction.
	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;

	// Which direction is the steepest ?
	bool is1Steepest = abs(gradient1) >= abs(gradient2);

	// Gradient in the corresponding direction, normalized.
	float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));

	// Average luma in the correct direction.
	float lumaLocalAverage = 0.0;
	if(is1Steepest)
	{
		// Switch the direction
		stepLength = -stepLength;
		lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
	}
	else
	{
		lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
	}

	// Shift UV in the correct direction by half a pixel.
	vec2 currentUv = coords;
	if(isHorizontal)
	{
		currentUv.y += stepLength * 0.5;
	}
	else
	{
		currentUv.x += stepLength * 0.5;
	}

	// Compute offset (for each iteration step) in the right direction.
	vec2 offset = isHorizontal ? vec2(inverseScreenSize.x, 0.0) : vec2(0.0, inverseScreenSize.y);

	// Compute UVs to explore on each side of the edge, orthogonally.
	// The fxaa_quality allows us to step faster.

	vec2 uv1 = currentUv - offset * fxaa_quality(0);
	vec2 uv2 = currentUv + offset * fxaa_quality(0);

	// Read the lumas at both current extremities of the exploration segment,
	// and compute the delta wrt to the local average luma.
	float lumaEnd1 = fxaa_sample_luma(uv1);
	float lumaEnd2 = fxaa_sample_luma(uv2);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;

	// If the luma deltas at the current extremities is larger than the local gradient,
	// we have reached the side of the edge.
	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;

	// If the side is not reached, we continue to explore in this direction.
	if(!reached1)
	{
		uv1 -= offset * fxaa_quality(1);
	}
	if(!reached2)
	{
		uv2 += offset * fxaa_quality(1);
	}

	// If both sides have not been reached, continue to explore.
	if(!reachedBoth)
	{
		for(int i = 2; i < 12; i++)
		{
			// If needed, read luma in 1st direction, compute delta.
			if(!reached1)
			{
				lumaEnd1 = fxaa_sample_luma(uv1);
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}

			// If needed, read luma in opposite direction, compute delta.
			if(!reached2)
			{
				lumaEnd2 = fxaa_sample_luma(uv2);
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}

			// If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			// If the side is not reached, we continue to explore in this direction, with a variable quality.
			if(!reached1)
			{
				uv1 -= offset * fxaa_quality(i);
			}
			if(!reached2){
				uv2 += offset * fxaa_quality(i);
			}

			// If both sides have been reached, stop the exploration.
			if(reachedBoth)
			{
				break;
			}
		}
	}

	// Compute the distances to each side edge of the edge (!).
	float distance1 = isHorizontal ? (coords.x - uv1.x) : (coords.y - uv1.y);
	float distance2 = isHorizontal ? (uv2.x - coords.x) : (uv2.y - coords.y);

	// In which direction is the side of the edge closer ?
	bool isDirection1 = distance1 < distance2;
	float distanceFinal = min(distance1, distance2);

	// Thickness of the edge.
	float edgeThickness = (distance1 + distance2);

	// Is the luma at center smaller than the local average ?
	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;

	// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
	bool correctVariation1 = (lumaEnd1 < 0.0) != isLumaCenterSmaller;
	bool correctVariation2 = (lumaEnd2 < 0.0) != isLumaCenterSmaller;

	// Only keep the result in the direction of the closer side of the edge.
	bool correctVariation = isDirection1 ? correctVariation1 : correctVariation2;

	// UV offset: read in the direction of the closest side of the edge.
	float pixelOffset = - distanceFinal / edgeThickness + 0.5;

	// If the luma variation is incorrect, do not offset.
	float finalOffset = correctVariation ? pixelOffset : 0.0;

	// Sub-pixel shifting
	// Full weighted average of the luma over the 3x3 neighborhood.
	float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);

	// Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;

	// Compute a sub-pixel offset based on this delta.
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * float(dSubpixelQuality);

	// Pick the biggest of the two offsets.
	finalOffset = max(finalOffset, subPixelOffsetFinal);

	// Compute the final UV coordinates.
	vec2 finalUv = coords;
	if(isHorizontal)
	{
		finalUv.y += finalOffset * stepLength;
	} 
	else
	{
		finalUv.x += finalOffset * stepLength;
	}

	// Read the color at the new UV coordinates, and use it.
	return texture2D(screen_texture, finalUv);
}

void main()
{
    vec4 full_value=texture2D(screen_texture, v_texcoord);
    if((mode_number==11 || mode_number==12) && full_value.w<1.0)
    {
    	float result=ssao_noisy(full_value);
    	if(mode_number==11)
    	{
    		gl_FragColor=vec4(full_value.xyz, result);
    	}
    	else
    	{
    		gl_FragColor=vec4(mix(full_value.xyz, vec3(0.0, 0.0, 0.0), result), 1.0);
    	}
    }
    else if((mode_number==21 || mode_number==22) && full_value.w<1.0)
    {
    	float result=ssao_blur_pass(full_value, mode_number-20);
    	if(mode_number==21)
    	{
    		gl_FragColor=vec4(full_value.xyz, result);
    	}
    	else
    	{
    		result=pow(result, 2.0);
    		gl_FragColor=vec4(mix(full_value.xyz, vec3(0.0, 0.0, 0.0), result), 1.0);
    	}
    }
    else if(mode_number==30)
    {
    	gl_FragColor=fxaa_compute_color(full_value);
    }
    else
    {
	    gl_FragColor=vec4(full_value.xyz, 1.0);
    }
}
