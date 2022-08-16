#version 100
precision highp float;

attribute vec2 a_position;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;

void main()
{
	v_texcoord=a_texcoord;
	gl_Position=vec4(a_position.x, a_position.y, 0.0, 1.0);
}
