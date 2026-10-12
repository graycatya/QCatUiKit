// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location = 0) in vec2 corner;
layout(location = 1) in vec3 position;
layout(location = 2) in vec4 color;
layout(location = 0) out vec2 localPosition;
layout(location = 1) out vec4 ledColor;
layout(std140, binding = 0) uniform Params {
    mat4 mvp;
    vec4 viewport; // physical width, height, physical diameter, brightness
    vec4 origin; // layout center XYZ, physical Y diameter
};
void main() {
    // Preserve the camera distance when world coordinates are large and coincident.
    vec4 center = mvp * vec4(position - origin.xyz, 1.0);
    // Expanding in clip space makes every quad face the camera, with constant pixel size.
    center.xy += corner * vec2(viewport.z, origin.w) / viewport.xy * center.w;
    gl_Position = center;
    localPosition = corner;
    ledColor = vec4(color.rgb * viewport.w, color.a);
}
