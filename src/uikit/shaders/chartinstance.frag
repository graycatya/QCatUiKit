// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec2 localPoint;
layout(location = 2) in vec4 shapeData;
layout(binding = 1) uniform sampler2D materialTexture;
layout(location = 0) out vec4 fragColor;
void main() {
    vec2 halfSize = shapeData.xy * 0.5;
    float radius = min(halfSize.x, halfSize.y);
    float distanceToEdge;
    if (shapeData.z < 0.5) distanceToEdge = length(localPoint) - radius;
    else if (shapeData.z < 1.5 || shapeData.z > 3.5)
        distanceToEdge = max(abs(localPoint.x) - halfSize.x, abs(localPoint.y) - halfSize.y);
    else if (shapeData.z < 2.5)
        distanceToEdge = max(localPoint.y - radius, (2.0 * abs(localPoint.x) - localPoint.y - radius) / sqrt(5.0));
    else distanceToEdge = (abs(localPoint.x) + abs(localPoint.y) - radius) / sqrt(2.0);
    float coverage = shapeData.w > 0.0 ? clamp(0.5 - distanceToEdge / shapeData.w, 0.0, 1.0) : step(distanceToEdge, 0.0);
    fragColor = vertexColor * texture(materialTexture, localPoint / shapeData.xy + vec2(0.5)) * coverage;
}
