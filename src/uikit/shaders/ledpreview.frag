// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location = 0) in vec2 localPosition;
layout(location = 1) in vec4 ledColor;
layout(location = 0) out vec4 fragColor;
void main() {
    if (dot(localPosition, localPosition) > 1.0 || ledColor.a <= 0.0)
        discard;
    fragColor = vec4(ledColor.rgb * ledColor.a, ledColor.a);
}
