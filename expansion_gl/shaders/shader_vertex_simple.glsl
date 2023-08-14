#version 100

precision highp float;

uniform mat4 projection_matrix;
uniform mat4 viewtransform_matrix;
uniform mat4 modeltransform_matrix;
uniform vec4 viewport;

attribute vec3 vertex_position;
attribute vec3 vertex_normal;
attribute vec3 vertex_color_for_selection;
attribute vec3 vertex_color_for_display;
attribute vec3 vertex_adjunct;

varying vec3 fragment_position;
varying vec3 fragment_normal;
varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;

void main()
{
	vec4 vertex_position_in_world=vec4(vertex_position, 1.0);
	
	if(vertex_adjunct[1]>0.5)
	{
		vec3 transformed_center=(modeltransform_matrix*vec4(vertex_normal, 1.0)).xyz;
		mat4 translation_only_matrix=mat4(1.0);
		translation_only_matrix[3].xyz=(transformed_center-vertex_normal)+vec3(0.0, 0.0, vertex_adjunct[2]);
		
		vertex_position_in_world=translation_only_matrix*vec4(vertex_position, 1.0);
		
        fragment_normal=vec3(0.0, 0.0, 1.0);
    }
    else
    {
        vertex_position_in_world=modeltransform_matrix*vec4(vertex_position, 1.0);
        
        fragment_normal=mat3(modeltransform_matrix)*vertex_normal;
    }
    
    fragment_position=vec3(vertex_position_in_world);
    fragment_color_for_selection=vertex_color_for_selection;
    fragment_color_for_display=vertex_color_for_display;
    fragment_adjunct=vertex_adjunct;
    
    gl_Position=projection_matrix*viewtransform_matrix*vertex_position_in_world;
}
