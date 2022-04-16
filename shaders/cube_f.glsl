#version 430

in vec3 Position;
in vec3 UV;
in mat3 Tangent;

layout(binding=0) uniform sampler3D Volume;
uniform mat4 mv;
uniform mat4 local;

layout(location=0) out vec4 Output;

const int steps=512;
const float step_length=1.0/steps;

void ray_box_intersection(vec3 origin, vec3 direction, vec3 top, vec3 bottom, out float t_0, out float t_1)
{
    vec3 direction_inv=1.0/direction;

    vec3 t_top=direction_inv*(top-origin);
    vec3 t_bottom=direction_inv*(bottom-origin);

    vec3 t_min=min(t_top, t_bottom);

    vec2 t=max(t_min.xx, t_min.yz);
    t_0=max(0.0, max(t.x, t.y));
    vec3 t_max=max(t_top, t_bottom);

    t=min(t_max.xx, t_max.yz);
    t_1=min(t.x, t.y);
}

vec4 transfer(float intensity)
{
    vec3 high=vec3(1.0, 1.0, 1.0);
    vec3 low=vec3(0.0, 0.0, 0.0);
    float alpha=(exp(intensity)-1.0)/(exp(1.0)-1.0);
    return vec4(intensity*high+(1.0-intensity)*low, alpha);
}

vec4 ramp(float t)
{
	t*=2.0f;

	return clamp(vec4(sin(t*3.1415927*0.5), sin(t*3.1415927), abs(sin(t*3.1415927*2.0)), t/2), 0.0, 1.0);
}

const vec3 top=vec3(1.0, 1.0, 1.0);
const vec3 bottom=vec3(-1.0, -1.0, -1.0);

layout(location=0) uniform vec3 fSize;

void main()
{
    vec3 ray_direction=vec3((2.0*gl_FragCoord.xy/fSize.xy-1.0)*vec2(fSize.x/fSize.y, 1.0), -1.0/tan(radians(fSize.z)/2.0));
    ray_direction=normalize(vec4(-ray_direction, 0)*mv*local).xyz;
    vec3 ray_origin=UV;

    float t_0, t_1;

    ray_box_intersection(ray_origin, ray_direction, top, bottom, t_0, t_1);

    vec3 ray_start=(ray_origin+ray_direction*t_0-bottom)/(top-bottom);
    vec3 ray_stop=(ray_origin+ray_direction*t_1-bottom)/(top-bottom);

    vec3 ray=ray_stop-ray_start;
    float ray_length=length(ray);
    vec3 step_vector=step_length*ray/ray_length;

    vec3 position=ray_start;
    vec4 color=vec4(0.0);

    while(ray_length>0.0&&ray_length<steps)
    {
        float intensity=pow(texture(Volume, position).r, 1.0);

        vec4 c=ramp(intensity);

        color.rgb=c.a*c.rgb+(1.0-c.a)*color.a*color.rgb;
        color.a=c.a+(1.0-c.a)*color.a;

        ray_length-=step_length;
        position+=step_vector;
    }

    Output=color;
}
