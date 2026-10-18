// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 vertexColor;
layout(location=3) in vec2 texcoord;
layout(location=4) in vec4 model0;
layout(location=5) in vec4 model1;
layout(location=6) in vec4 model2;
layout(location=7) in vec4 model3;
layout(location=8) in vec4 instanceColor;
layout(std140,binding=0) uniform Scene {
    mat4 viewProjection;
    vec4 eye;
    vec4 light;
    vec4 material;
    vec4 lighting; // model, ambient ground ratio, exposure, reserved
};
layout(location=0) out vec3 worldPosition;
layout(location=1) out vec3 worldNormal;
layout(location=2) out vec3 color;
layout(location=3) out vec2 uv;
vec3 srgbToLinear(vec3 c) {
    return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}
void main() {
    mat4 model=mat4(model0,model1,model2,model3);
    vec4 p=model*vec4(position,1.0);
    worldPosition=p.xyz;
    worldNormal=transpose(inverse(mat3(model)))*normal;
    // Decode before interpolation. Legacy Phong retains its encoded-color path.
    color=lighting.x > 0.5 ? srgbToLinear(vertexColor)*srgbToLinear(instanceColor.rgb)
                          : vertexColor*instanceColor.rgb;
    uv=texcoord;
    gl_Position=viewProjection*p;
}
