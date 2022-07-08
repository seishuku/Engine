#version 430

in vec3 Position;
in vec3 UV;
in mat3 Tangent;

layout(binding=0) uniform sampler3D Volume;
layout(binding=1) uniform sampler1D Transfer;
uniform mat4 mv;
uniform mat4 local;

layout(location=0) out vec4 Output;

vec2 intersectBox(vec3 r_o, vec3 r_d)
{
    const vec3 boxmin=vec3(-1.0, -1.0, -1.0);
    const vec3 boxmax=vec3(1.0, 1.0, 1.0);

    // compute intersection of ray with all six bbox planes
    vec3 invR = 1.0/r_d;

    vec3 tbot=invR*(boxmin-r_o);
    vec3 ttop=invR*(boxmax-r_o);

    // re-order intersections to find smallest and largest on each axis
    vec3 tmin=min(ttop, tbot);
    vec3 tmax=max(ttop, tbot);

    // find the largest tmin and the smallest tmax
    return vec2(max(max(tmin.x, tmin.y), max(tmin.x, tmin.z)), min(min(tmax.x, tmax.y), min(tmax.x, tmax.z)));
}

const int numSteps=500;

void main()
{
    vec3 ray_origin=UV;
    vec3 ray_direction=normalize(inverse(mv)[3].xyz-Position);

    vec2 hit=intersectBox(ray_origin, ray_direction);

    if(hit.x<0.0)
        hit.x=0.0;

	vec3 rayStart=ray_origin+ray_direction*hit.x;
	vec3 rayStop=ray_origin+ray_direction*hit.y;

	float dist=distance(rayStop, rayStart);
	float stepSize=dist/numSteps;
	vec3 ds=normalize(rayStop-rayStart)*stepSize;

	vec4 color=vec4(0.0);
	vec3 start=rayStart;

	for(int i=0;i<numSteps;i++)
	{
        vec4 c=texture(Transfer, pow(texture(Volume, start*0.5+0.5).r, 0.5));

        color=mix(color, vec4(c.xyz, 1.0), c.a*1.0);
        start+=ds;
	}

	Output=color;
}
