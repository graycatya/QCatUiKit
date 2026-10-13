// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location = 0) in vec2 localPosition;
layout(location = 1) in vec4 ledColor;
layout(location = 2) in vec3 surfaceNormal;
layout(location = 3) flat in int shape;
layout(location = 0) out vec4 fragColor;
void main() {
    if ((shape==1 && dot(localPosition,localPosition)>1.0) || ledColor.a<=0.0) discard;
    float light=shape>=2 ? .35+.65*max(0.0,dot(normalize(surfaceNormal),normalize(vec3(.3,.5,1.0)))) : 1.0;
    fragColor=vec4(ledColor.rgb*light*ledColor.a,ledColor.a);
}
