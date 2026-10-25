#version 440
layout(location=0) in vec4 vColor;
layout(location=1) in vec2 vPosition;
layout(location=0) out vec4 fragColor;
layout(std140,binding=0) uniform Uniforms {
    mat4 projection; mat4 paintMatrix;
    vec4 options; vec4 p0; vec4 p1; vec4 p2;
    vec4 c0; vec4 c1; vec4 c2; vec4 region;
} u;
layout(binding=1) uniform sampler2D sourceTexture;
layout(binding=2) uniform sampler2D auxTexture;
layout(binding=3) uniform sampler2D originalTexture;
const float TAU=6.283185307179586;
vec2 screenUV(vec2 p) {
    vec2 uv=p/u.p2.xy;
    if (u.options.w>0.5) uv.y=1.0-uv.y;
    return uv;
}
vec4 sourceAt(vec2 p) {
    if (any(lessThan(p,u.region.xy)) || any(greaterThanEqual(p,u.region.zw))) return vec4(0);
    return texture(sourceTexture,screenUV(p));
}
float lum(vec3 c) { return dot(c,vec3(0.2126,0.7152,0.0722)); }
float blendLum(vec3 c) { return dot(c,vec3(0.3,0.59,0.11)); }
float sat(vec3 c) { return max(max(c.r,c.g),c.b)-min(min(c.r,c.g),c.b); }
vec3 clipColor(vec3 c) {
    float l=blendLum(c), n=min(min(c.r,c.g),c.b), x=max(max(c.r,c.g),c.b);
    if(n<0.0) c=l+(c-l)*l/max(l-n,0.000001);
    if(x>1.0) c=l+(c-l)*(1.0-l)/max(x-l,0.000001);
    return c;
}
vec3 setLum(vec3 c,float l) { return clipColor(c+l-blendLum(c)); }
vec3 setSat(vec3 c,float s) {
    float n=min(min(c.r,c.g),c.b), x=max(max(c.r,c.g),c.b);
    return x>n ? (c-n)*s/(x-n) : vec3(0);
}
vec3 blendColor(vec3 s,vec3 d,int mode) {
    if(mode==1) return s*d;
    if(mode==2) return s+d-s*d;
    if(mode==3) return mix(2.0*s*d,1.0-2.0*(1.0-s)*(1.0-d),step(vec3(0.5),d));
    if(mode==4) return min(s,d);
    if(mode==5) return max(s,d);
    if(mode==6) {
        vec3 c=mix(min(vec3(1),d/max(vec3(0.000001),1.0-s)),vec3(1),step(vec3(1),s));
        return mix(c,vec3(0),step(d,vec3(0)));
    }
    if(mode==7) {
        vec3 c=mix(1.0-min(vec3(1),(1.0-d)/max(vec3(0.000001),s)),vec3(0),step(s,vec3(0)));
        return mix(c,vec3(1),step(vec3(1),d));
    }
    if(mode==8) return mix(2.0*s*d,1.0-2.0*(1.0-s)*(1.0-d),step(vec3(0.5),s));
    if(mode==9) {
        vec3 g=mix(((16.0*d-12.0)*d+4.0)*d,sqrt(max(d,vec3(0))),step(vec3(0.25),d));
        return mix(d-(1.0-2.0*s)*d*(1.0-d),d+(2.0*s-1.0)*(g-d),step(vec3(0.5),s));
    }
    if(mode==10) return abs(d-s);
    if(mode==11) return s+d-2.0*s*d;
    if(mode==12) return setLum(setSat(s,sat(d)),blendLum(d));
    if(mode==13) return setLum(setSat(d,sat(s)),blendLum(d));
    if(mode==14) return setLum(s,blendLum(d));
    if(mode==15) return setLum(d,blendLum(s));
    if(mode==16) return min(vec3(1),s+d);
    return s;
}
vec4 advanced(vec4 s,vec4 d,int mode) {
    vec3 cs=s.a>0.0 ? s.rgb/s.a : vec3(0), cd=d.a>0.0 ? d.rgb/d.a : vec3(0);
    if(mode==16) return min(vec4(1),s+d);
    return vec4((1.0-s.a)*d.rgb+(1.0-d.a)*s.rgb+s.a*d.a*blendColor(cs,cd,mode),s.a+d.a-s.a*d.a);
}
void main() {
    int mode=int(u.options.x+0.5);
    if(mode==0) { fragColor=vColor; return; }
    if(mode<=4) {
        vec2 p=(u.paintMatrix*vec4(vPosition,0,1)).xy;
        if(mode==4) {
            fragColor=texture(sourceTexture,(p-vec2(0.5*u.p0.z))/u.p0.xy)*u.options.y;
            return;
        }
        float t=0.0;
        if(mode==1) {
            vec2 d=u.p0.zw-u.p0.xy;
            t=dot(p-u.p0.xy,d)/max(dot(d,d),0.000001);
        } else if(mode==2) {
            vec2 delta=u.p0.xy-u.p1.xy, v=p-u.p1.xy;
            float dr=u.p0.z-u.p1.z;
            float a=dot(delta,delta)-dr*dr;
            float b=-2.0*(dot(v,delta)+u.p1.z*dr);
            float c=dot(v,v)-u.p1.z*u.p1.z;
            float det=b*b-4.0*a*c;
            if(abs(a)<0.000001) t=abs(b)>0.000001 ? -c/b : 0.0;
            else if(det>=0.0) t=max((-b+sqrt(det))/(2.0*a),(-b-sqrt(det))/(2.0*a));
            else { fragColor=vec4(0); return; }
        } else t=fract((atan(p.y-u.p0.y,p.x-u.p0.x)-u.p0.z)/TAU);
        if(mode!=3) {
            if(u.options.z<0.5) t=clamp(t,0.0,1.0);
            else if(u.options.z<1.5) t=1.0-abs(mod(t,2.0)-1.0);
            else t=fract(t);
        }
        float n=float(textureSize(sourceTexture,0).x);
        fragColor=textureLod(sourceTexture,vec2((t*(n-1.0)+0.5)/n,0.5),0.0)*u.options.y;
        return;
    }
    vec4 src=texture(sourceTexture,screenUV(vPosition));
    if(mode==5) { fragColor=src*u.options.y; return; }
    if(mode==6) {
        vec4 mask=texture(auxTexture,screenUV(vPosition));
        int method=int(u.options.z+0.5);
        float alpha=mask.a;
        if(method==3 || method==4) alpha=dot(mask.rgb,vec3(54,182,19)/256.0);
        if(method==2 || method==4) alpha=1.0-alpha;
        if(method<5) fragColor=src*alpha;
        else {
            float a=src.a;
            if(method==5) a=src.a+mask.a*(1.0-src.a);
            if(method==6) a=src.a*(1.0-mask.a);
            if(method==7) a=src.a*mask.a;
            if(method==8) a=src.a*(1.0-mask.a)+mask.a*(1.0-src.a);
            if(method==9) a=max(src.a,mask.a);
            if(method==10) a=min(src.a,mask.a);
            fragColor=vec4(src.a>0.0 ? src.rgb*(a/src.a) : vec3(0),a);
        }
        return;
    }
    if(mode==7) { fragColor=advanced(src,texture(auxTexture,screenUV(vPosition)),int(u.options.z+0.5)); return; }
    if(mode==10) {
        float sigma=max(u.p0.z,0.0001);
        int radius=int(min(ceil(3.0*sigma),64.0));
        float stepSize=max(1.0,ceil(3.0*sigma)/64.0);
        vec4 sum=vec4(0); float weights=0.0;
        for(int i=-radius;i<=radius;++i) {
            float x=float(i)*stepSize;
            float weight=exp(-0.5*x*x/(sigma*sigma));
            sum+=sourceAt(vPosition+u.p0.xy*x)*weight; weights+=weight;
        }
        fragColor=sum/weights; return;
    }
    if(mode==11) {
        vec4 original=texture(originalTexture,screenUV(vPosition));
        float a=sourceAt(vPosition-u.p1.xy).a*u.c0.a;
        fragColor=original+vec4(u.c0.rgb*a,a)*(1.0-original.a); return;
    }
    if(mode==12) { float a=src.a*u.c0.a; fragColor=vec4(u.c0.rgb*a,a); return; }
    if(mode==13) {
        vec3 color=mix(src.rgb,mix(u.c0.rgb,u.c1.rgb,lum(src.rgb)),u.c0.a);
        fragColor=vec4(color*src.a,src.a); return;
    }
    if(mode==14) {
        float l=lum(src.rgb);
        vec3 color=l<0.5 ? mix(u.c0.rgb,u.c1.rgb,l*2.0) : mix(u.c1.rgb,u.c2.rgb,(l-0.5)*2.0);
        fragColor=vec4(mix(color,src.rgb,u.c0.a)*src.a,src.a); return;
    }
    if(mode==15) {
        int n=max(1,int(u.p0.z)); vec4 sum=vec4(0);
        for(int i=0;i<n;++i) sum+=sourceAt(vPosition+u.p0.xy*((float(i)+0.5)/float(n)-0.5));
        fragColor=sum/float(n); return;
    }
    fragColor=vec4(0);
}
