#version 100
uniform mat4 projection_matrix;
uniform mat4 viewtransform_matrix;
uniform mat4 modeltransform_matrix;
uniform vec4 viewport;
attribute vec4 vertex_info1;
attribute vec4 vertex_info2;
attribute vec3 vertex_color_for_selection;
attribute vec3 vertex_color_for_display;
attribute vec3 vertex_adjunct;
varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;
void main()
{
    vec4 vertex_position_in_gl=projection_matrix*viewtransform_matrix*modeltransform_matrix*vec4(vec3(vertex_info1), 1.0);
    
    fragment_color_for_selection=vertex_color_for_selection;
    fragment_color_for_display=vertex_color_for_display;
    fragment_adjunct=vertex_adjunct;
    
    gl_PointSize=vertex_info1[3]*min(viewport[2], viewport[3])*0.005;
    gl_Position=vertex_position_in_gl;
}
