// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec2 scalarTexture;
layout(binding = 1) uniform sampler2D materialTexture;
layout(location = 0) out vec4 fragColor;
void main() {
    vec4 value = texture(materialTexture, uv);
    if (scalarTexture.x > 0.0) {
        if (value.a < 0.5) discard;
        float t = (value.r * 65280.0 + value.g * 255.0) / 65535.0;
        vec4 mapped = texture(materialTexture, vec2((floor(t * 255.0 + 0.5) + 0.5) * scalarTexture.x, 1.0 - 0.5 * scalarTexture.y));
        value = vec4(mapped.rgb * mapped.a, mapped.a);
    }
    fragColor = vertexColor * value;
}
