#version 430

in vec3 Position;
in vec3 UV;
in mat3 Tangent;

layout(binding=0) uniform sampler3D Volume;
layout(binding=1) uniform sampler1D Transfer;
uniform mat4 mv;
uniform mat4 local;

layout(location=0) out vec4 Output;

const int steps=500;
const float step_length=1.0/steps;

bool intersectBox(vec3 r_o, vec3 r_d, vec3 boxmin, vec3 boxmax, out float tnear, out float tfar)
{
    // compute intersection of ray with all six bbox planes
    vec3 invR = vec3(1.0, 1.0, 1.0)/r_d;

    vec3 tbot=invR*(boxmin-r_o);
    vec3 ttop=invR*(boxmax-r_o);

    // re-order intersections to find smallest and largest on each axis
    vec3 tmin=min(ttop, tbot);
    vec3 tmax=max(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin=max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax=min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

	tnear=largest_tmin;
	tfar=smallest_tmax;

    if(smallest_tmax>largest_tmin)
        return true;
    else
        return false;
}

const vec3 top=vec3(1.0, 1.0, 1.0);
const vec3 bottom=vec3(-1.0, -1.0, -1.0);

uniform vec3 fSize;

void main()
{
    float tnear, tfar;
    vec3 ray_direction=normalize(vec4((2.0*gl_FragCoord.xy/fSize.xy-1.0)*vec2(fSize.x/fSize.y, 1.0), -1.0/tan(radians(fSize.z)/2.0), 0.0)*mv*local).xyz;
    vec3 ray_origin=-UV;

    if(!intersectBox(ray_origin, ray_direction, top, bottom, tnear, tfar))
    {
        Output=vec4(0.0);
        return;
    }

    vec4 color=vec4(0.0);

    vec3 ray_start=(ray_origin+ray_direction*tnear-bottom)/(top-bottom);
    vec3 ray_stop=(ray_origin+ray_direction*tfar-bottom)/(top-bottom);

    vec3 ray=ray_stop-ray_start;
    float ray_length=length(ray);
    vec3 step_vector=step_length*ray/ray_length;

    vec3 pos=ray_start;

    while(ray_length>0.0&&ray_length<steps)
    {
        vec4 c=texture(Transfer, pow(texture(Volume, pos).r, 0.5));

        color=mix(color, vec4(c.xyz, 1.0), c.a*0.25);

        ray_length-=step_length;
        pos+=step_vector;
    }

    Output=color;
}
