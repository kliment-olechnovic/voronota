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
varying vec3 fragment_center_in_ndc;
varying float fragment_radius_in_ndc;
varying vec2 fragment_center_in_screen;
varying float fragment_radius_in_screen;
varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;
void main()
{
    vec4 vertex_position_in_clip=projection_matrix*viewtransform_matrix*modeltransform_matrix*vec4(vec3(vertex_info1), 1.0);
    vec3 vertex_position_in_ndc=vertex_position_in_clip.xyz/vertex_position_in_clip.w;
    vec2 vertex_position_in_screen=(vertex_position_in_ndc.xy*0.5+0.5)*vec2(viewport[2], viewport[3])+vec2(viewport[0], viewport[1]);
    
    vec4 sa100=projection_matrix*viewtransform_matrix*modeltransform_matrix*vec4(vec3(vertex_info1)+vec3(1.0, 0.0, 0.0), 1.0);
    vec3 sb100=sa100.xyz/sa100.w;
    vec2 sc100=(sb100.xy*0.5+0.5)*vec2(viewport[2], viewport[3])+vec2(viewport[0], viewport[1]);
    
    vec4 sa010=projection_matrix*viewtransform_matrix*modeltransform_matrix*vec4(vec3(vertex_info1)+vec3(0.0, 1.0, 0.0), 1.0);
    vec3 sb010=sa010.xyz/sa010.w;
    vec2 sc010=(sb010.xy*0.5+0.5)*vec2(viewport[2], viewport[3])+vec2(viewport[0], viewport[1]);
    
    vec4 sa001=projection_matrix*viewtransform_matrix*modeltransform_matrix*vec4(vec3(vertex_info1)+vec3(0.0, 0.0, 1.0), 1.0);
    vec3 sb001=sa001.xyz/sa001.w;
    vec2 sc001=(sb001.xy*0.5+0.5)*vec2(viewport[2], viewport[3])+vec2(viewport[0], viewport[1]);
    
    vec2 ob100=sb100.xy-vertex_position_in_ndc.xy;
    vec2 ob010=sb010.xy-vertex_position_in_ndc.xy;
    vec2 ob001=sb001.xy-vertex_position_in_ndc.xy;
    float scale_in_ndc=sqrt((dot(ob100, ob100)+dot(ob010, ob010)+dot(ob001, ob001))/2.0);
    
    vec2 oc100=sc100-vertex_position_in_screen;
    vec2 oc010=sc010-vertex_position_in_screen;
    vec2 oc001=sc001-vertex_position_in_screen;
    float scale_in_screen=sqrt((dot(oc100, oc100)+dot(oc010, oc010)+dot(oc001, oc001))/2.0);

    fragment_center_in_ndc=vertex_position_in_ndc;
    fragment_radius_in_ndc=scale_in_ndc*vertex_info1[3];
    fragment_center_in_screen=vertex_position_in_screen;
    fragment_radius_in_screen=scale_in_screen*vertex_info1[3];
    fragment_color_for_selection=vertex_color_for_selection;
    fragment_color_for_display=vertex_color_for_display;
    fragment_adjunct=vertex_adjunct;
    
    gl_PointSize=fragment_radius_in_screen*2.0;
    gl_Position=vertex_position_in_clip;
}
