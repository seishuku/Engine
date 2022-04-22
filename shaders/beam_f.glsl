#version 430

in vec3 Position;
in vec2 UV;

in vec3 startp;
in vec3 endp;

uniform mat4 proj;
uniform mat4 mv;
uniform mat4 local;

layout(location=0) uniform vec4 color;
layout(location=1) uniform vec3 start;
layout(location=2) uniform vec3 end;
layout(location=3) uniform bool i;

layout(location=0) out vec4 Output;

void main(void)
{
    const float radius=1.0;
    vec3 eye=(inverse(mv)[3]).xyz;

	vec3 startp=(local*vec4(start, 1.0)).xyz;
	vec3 endp=(local*vec4(end, 1.0)).xyz;

    vec3 u=endp-startp;
    vec3 v=Position-eye;
    vec3 w=startp-Position;

    float a=dot(u, u);
    float b=dot(u, v);
    float c=dot(v, v);
    float d=dot(u, w);
    float e=dot(v, w);

    float D=a*c-b*b;

    float sD=D;
    float tD=D;

    float sN=(b*e-c*d);
    float tN=(a*e-b*d);

    if(sN<0.0)
    {
        sN=0.0;
        tN=e;
        tD=c;
    }
    else if(sN>sD)
    {
        sN=D;
        tN=e+b;
        tD=c;
    }

    float sc=sN/sD;
    float tc=tN/tD;

    float factor=1.0-length(w+(sc*u)-(tc*v))*color.w;

    Output=vec4(color.xyz*factor, 1.0);
}
