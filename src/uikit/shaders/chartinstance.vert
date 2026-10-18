// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location = 0) in vec2 corner;
layout(location = 1) in vec4 bounds;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 style;
layout(location = 0) out vec4 vertexColor;
layout(location = 1) out vec2 localPoint;
layout(location = 2) out vec4 shapeData;
layout(std140, binding = 0) uniform Uniforms { mat4 projection; } ubuf;
void main() {
    localPoint = (corner - vec2(0.5)) * (bounds.zw + vec2(style.y));
    vertexColor = color;
    shapeData = vec4(bounds.zw, style);
    gl_Position = ubuf.projection * vec4(bounds.xy + bounds.zw * 0.5 + localPoint, 0.0, 1.0);
}
