#version 100

precision highp float;

uniform mat4 projection_matrix;
uniform mat4 viewtransform_matrix;
uniform mat4 modeltransform_matrix;
uniform vec4 viewport;

attribute vec4 vertex_info1;
attribute vec4 vertex_info2;
attribute vec3 vertex_color_for_selection;
attribute vec3 vertex_color_for_display;
attribute vec3 vertex_adjunct;

varying mat4 VPMT_inverse;
varying mat4 VP_inverse;
varying vec3 centernormclip;
varying vec3 fragment_color_for_selection;
varying vec3 fragment_color_for_display;
varying vec3 fragment_adjunct;

// Some info about the reused ideas and code:
//   The sphere impostoring code is adapted from
//     https://github.com/ssloy/glsltuto/blob/master/shaders/ (by Dmitry V. Sokolov, no license stated)
//     who used the paper "GPU-Based Ray-Casting of Quadratic Surfaces" (http://dl.acm.org/citation.cfm?id=2386396) by Christian Sigg, Tim Weyrich, Mario Botsch, Markus Gross.

mat4 invert_mat(mat4 m)
{
    float
      a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3],
      a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3],
      a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3],
      a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3],

      b00 = a00 * a11 - a01 * a10,
      b01 = a00 * a12 - a02 * a10,
      b02 = a00 * a13 - a03 * a10,
      b03 = a01 * a12 - a02 * a11,
      b04 = a01 * a13 - a03 * a11,
      b05 = a02 * a13 - a03 * a12,
      b06 = a20 * a31 - a21 * a30,
      b07 = a20 * a32 - a22 * a30,
      b08 = a20 * a33 - a23 * a30,
      b09 = a21 * a32 - a22 * a31,
      b10 = a21 * a33 - a23 * a31,
      b11 = a22 * a33 - a23 * a32,

      det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

    return mat4(
      a11 * b11 - a12 * b10 + a13 * b09,
      a02 * b10 - a01 * b11 - a03 * b09,
      a31 * b05 - a32 * b04 + a33 * b03,
      a22 * b04 - a21 * b05 - a23 * b03,
      a12 * b08 - a10 * b11 - a13 * b07,
      a00 * b11 - a02 * b08 + a03 * b07,
      a32 * b02 - a30 * b05 - a33 * b01,
      a20 * b05 - a22 * b02 + a23 * b01,
      a10 * b10 - a11 * b08 + a13 * b06,
      a01 * b08 - a00 * b10 - a03 * b06,
      a30 * b04 - a31 * b02 + a33 * b00,
      a21 * b02 - a20 * b04 - a23 * b00,
      a11 * b07 - a10 * b09 - a12 * b06,
      a00 * b09 - a01 * b07 + a02 * b06,
      a31 * b01 - a30 * b03 - a32 * b00,
      a20 * b03 - a21 * b01 + a22 * b00) / det;
}

mat4 transpose_mat(mat4 m)
{
    return mat4(
      m[0][0], m[1][0], m[2][0], m[3][0],
      m[0][1], m[1][1], m[2][1], m[3][1],
      m[0][2], m[1][2], m[2][2], m[3][2],
      m[0][3], m[1][3], m[2][3], m[3][3]);
}

void main()
{
    mat4 ModelViewProjectionMatrix=projection_matrix*viewtransform_matrix*modeltransform_matrix;
    mat4 ModelViewProjectionMatrix_inverse=invert_mat(ModelViewProjectionMatrix);
    mat4 ProjectionMatrix_inverse=invert_mat(projection_matrix);
    mat4 ModelViewMatrix=viewtransform_matrix*modeltransform_matrix;
    
    vec4 vertex_position=vec4(vertex_info1.xyz, 1.0);
    float R=vertex_info1.w;
    
    vec4 glposition=ModelViewProjectionMatrix*vertex_position;
    
    mat4 T=mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            vertex_position.x/R, vertex_position.y/R, vertex_position.z/R, 1.0/R);
    
    mat4 PMTt=transpose_mat(ModelViewProjectionMatrix*T);
    
    vec4 r1=PMTt[0];
    vec4 r2=PMTt[1];
    vec4 r4=PMTt[3];
    float r1Dr4T=dot(r1.xyz,r4.xyz)-r1.w*r4.w;
    float r1Dr1T=dot(r1.xyz,r1.xyz)-r1.w*r1.w;
    float r4Dr4T=dot(r4.xyz,r4.xyz)-r4.w*r4.w;
    float r2Dr2T=dot(r2.xyz,r2.xyz)-r2.w*r2.w;
    float r2Dr4T=dot(r2.xyz,r4.xyz)-r2.w*r4.w;
    
    glposition=vec4(-r1Dr4T, -r2Dr4T, glposition.z/glposition.w*(-r4Dr4T), -r4Dr4T);
    
    float discriminant_x=r1Dr4T*r1Dr4T-r4Dr4T*r1Dr1T;
    float discriminant_y=r2Dr4T*r2Dr4T-r4Dr4T*r2Dr2T;
    float screen=max(float(viewport.z), float(viewport.w));
    
    float glpointsize=sqrt(max(discriminant_x, discriminant_y))*screen/(-r4Dr4T);
    
    mat4 T_inverse=mat4(
            1.0,          0.0,          0.0,         0.0,
            0.0,          1.0,          0.0,         0.0,
            0.0,          0.0,          1.0,         0.0,
            -vertex_position.x, -vertex_position.y, -vertex_position.z, R);
    
    mat4 V_inverse=mat4(
            2.0/float(viewport.z), 0.0, 0.0, 0.0,
            0.0, 2.0/float(viewport.w), 0.0, 0.0,
            0.0, 0.0,                   2.0/gl_DepthRange.diff, 0.0,
            -float(viewport.z+2.0*viewport.x)/float(viewport.z), -float(viewport.w+2.0*viewport.y)/float(viewport.w), -(gl_DepthRange.near+gl_DepthRange.far)/gl_DepthRange.diff, 1.0);
    
    VPMT_inverse=T_inverse*ModelViewProjectionMatrix_inverse*V_inverse;
    VP_inverse=ProjectionMatrix_inverse*V_inverse;
    
    vec4 centerclip=ModelViewMatrix*vertex_position;
    centernormclip=vec3(centerclip)/centerclip.w;
    
    fragment_color_for_selection=vertex_color_for_selection;
    fragment_color_for_display=vertex_color_for_display;
    fragment_adjunct=vertex_adjunct;
    
    gl_PointSize=glpointsize;
    gl_Position=glposition;
}
