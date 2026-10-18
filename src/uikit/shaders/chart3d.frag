// SPDX-License-Identifier: Apache-2.0
#version 440
layout(location=0) in vec3 worldPosition;
layout(location=1) in vec3 worldNormal;
layout(location=2) in vec3 color;
layout(location=3) in vec2 uv;
layout(std140,binding=0) uniform Scene {
    mat4 viewProjection;
    vec4 eye;
    vec4 light;
    vec4 material;
    vec4 lighting; // model, ambient ground ratio, exposure, reserved
};
layout(binding=1) uniform sampler2D colorTexture;
layout(location=0) out vec4 fragColor;
vec3 srgbToLinear(vec3 c) {
    return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}
vec3 linearToSrgb(vec3 c) {
    return mix(12.92*c, 1.055*pow(c, vec3(1.0/2.4))-0.055, step(vec3(0.0031308), c));
}
void main() {
    vec3 n=normalize(worldNormal);
    if(!gl_FrontFacing) n=-n;
    vec3 l=normalize(light.xyz);
    vec3 v=normalize(eye.xyz-worldPosition);
    float d=max(dot(n,l),0.0);
    vec3 texel=texture(colorTexture,uv).rgb;
    if (lighting.x < 0.5) {
        float s=d>0.0 ? pow(max(dot(reflect(-l,n),v),0.0),material.w) : 0.0;
        fragColor=vec4(clamp(color*texel*(material.x+material.y*d)+vec3(material.z*s),0.0,1.0),1.0);
        return;
    }
    vec3 halfVector=l+v;
    float halfLength=dot(halfVector,halfVector);
    // Four times the Phong exponent approximately preserves highlight width.
    // Opposing light/view vectors have no defined half vector or visible highlight.
    float s=0.0;
    if (d>0.0 && halfLength>1e-8)
        s=pow(max(dot(n,halfVector*inversesqrt(halfLength)),0.0),material.w*4.0);
    float hemisphere=mix(lighting.y,1.0,n.y*0.5+0.5);
    vec3 base=color*srgbToLinear(texel);
    vec3 lit=(base*(material.x*hemisphere+material.y*d)+vec3(material.z*s))*lighting.z;
    // Preserve midtones and RGB ratios; smoothly compress peaks above .8
    // instead of clipping bright colored surfaces and highlights to white.
    float peak=max(lit.r,max(lit.g,lit.b));
    if (peak>0.8) {
        float excess=peak-0.8;
        lit*= (0.8+excess/(1.0+excess/0.2))/peak;
    }
    // QQuickRhiItem's RGBA8 target is not sRGB: encode once for Qt Quick composition.
    fragColor=vec4(linearToSrgb(max(lit,vec3(0.0))),1.0);
}
