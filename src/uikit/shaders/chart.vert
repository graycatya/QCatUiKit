// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec2 scalar;
layout(location = 0) out vec4 vertexColor;
layout(location = 1) out vec2 uv;
layout(location = 2) out vec2 scalarTexture;
layout(std140, binding = 0) uniform Uniforms { mat4 projection; } ubuf;
void main()
{
    vertexColor = color;
    uv = texCoord;
    scalarTexture = scalar;
    gl_Position = ubuf.projection * vec4(position, 0.0, 1.0);
}
