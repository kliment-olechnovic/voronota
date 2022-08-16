#version 100
precision highp float;

varying vec2 v_texcoord;

uniform sampler2D screen_texture;

void main()
{
    vec3 color=texture2D(screen_texture, v_texcoord).rgb;
    gl_FragColor=vec4(color, 1.0);
}
