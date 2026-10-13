// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 position;
layout(location = 2) in vec4 color;
layout(location = 3) in vec3 normal;
layout(location = 0) out vec2 localPosition;
layout(location = 1) out vec4 ledColor;
layout(location = 2) out vec3 surfaceNormal;
layout(location = 3) flat out int shape;
layout(std140, binding = 0) uniform Params {
    mat4 projection;
    mat4 view;
    vec4 viewport; // logical dimensions, pixel diameter, preview brightness
    vec4 options; // scene diameter, size mode, shape, projection Y scale
    vec4 gaps; // remaining X/Y fill, orthographic
};
void main() {
    vec4 center = view * vec4(position,1.0);
    shape = int(options.z);
    float radius = options.x * .5;
    if(options.y < .5)
        radius = viewport.z * (gaps.z > .5 ? 1.0 : -center.z) / (viewport.y * options.w);
    vec3 scale = vec3(gaps.xy,1.0);
    // Round shapes stay round; gap controls share the smaller fill fraction.
    if(shape==1 || shape==3) scale=vec3(min(gaps.x,gaps.y));
    vec3 offset=vertex * radius * scale;
    if(shape>=2) offset=mat3(view)*offset;
    gl_Position=projection*(center+vec4(offset,0.0));
    localPosition=vertex.xy;
    surfaceNormal=mat3(view)*normal;
    ledColor=vec4(color.rgb*viewport.w,color.a);
}
