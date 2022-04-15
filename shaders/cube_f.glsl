#version 430

in vec3 Position;
in vec3 UV;
in mat3 Tangent;

layout(binding=0) uniform sampler3D Volume;
uniform mat4 mv;
uniform mat4 local;
layout(location=0) uniform vec3 eye;

layout(location=0) out vec4 Output;

const int steps=256;
const float sample_step=1.0/steps;

vec4 getRamp(float t)
{
	t*=2.0f;

	return vec4(sin(t*3.1415927*0.5), sin(t*3.1415927), abs(sin(t*3.1415927*2.0)), sin(t*3.1415927*0.5));
}

void main()
{
	vec3 ray_dir=normalize(inverse(mv)[3].xyz-Position);
    vec4 frag_color=vec4(0.0);

    for(int i=0;i<steps;i++)
    {
        float fi=float(i)/steps;
        vec3 ray_pos=UV+ray_dir*fi;

        float density=texture(Volume, mat3(1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0)*ray_pos+vec3(0.0, 0.0, 1.0)).r;
        density=pow(density, 2.0);

        frag_color=mix(frag_color, vec4(getRamp(density).xyz, 1.0), density);
    }

    if(frag_color==vec4(0.0))
        discard;

    Output=frag_color;
}
