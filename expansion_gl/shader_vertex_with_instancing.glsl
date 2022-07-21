#version 100
uniform mat4 projection_matrix;
uniform mat4 viewtransform_matrix;
uniform mat4 modeltransform_matrix;
uniform vec4 viewport;
attribute vec3 vertex_position;
attribute vec3 vertex_normal;
attribute vec3 vertex_color_for_selection;
attribute vec3 vertex_color_for_display;
attribute vec3 vertex_adjunct;
attribute vec4 vertex_transformation_0;
attribute vec4 vertex_transformation_1;
attribute vec4 vertex_transformation_2;
attribute vec4 vertex_transformation_3;
varying vec3 fragment_position;
varying vec3 fragment_normal;
varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;
void main()
{
    mat4 vertex_transformation_matrix=mat4(vertex_transformation_0, vertex_transformation_1, vertex_transformation_2, vertex_transformation_3);
    vec4 transformed_vertex_position=vertex_transformation_matrix*vec4(vertex_position, 1.0);
    vec3 transformed_vertex_normal=mat3(vertex_transformation_matrix)*vertex_normal;
    vec4 vertex_position_in_world=modeltransform_matrix*transformed_vertex_position;
    fragment_position=vec3(vertex_position_in_world);
    fragment_normal=mat3(modeltransform_matrix)*transformed_vertex_normal;
    fragment_color_for_selection=vertex_color_for_selection;
    fragment_color_for_display=vertex_color_for_display;
    fragment_adjunct=vertex_adjunct;
    gl_Position=projection_matrix*viewtransform_matrix*vertex_position_in_world;
}
