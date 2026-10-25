#version 440
layout(location=0) in vec2 position;
layout(location=1) in vec4 color;
layout(location=0) out vec4 vColor;
layout(location=1) out vec2 vPosition;
layout(std140,binding=0) uniform Uniforms {
    mat4 projection; mat4 paintMatrix;
    vec4 options; vec4 p0; vec4 p1; vec4 p2;
    vec4 c0; vec4 c1; vec4 c2; vec4 region;
} u;
void main() {
    vColor=color; vPosition=position;
    gl_Position=u.projection*vec4(position,0,1);
}
