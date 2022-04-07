#version 430

in vec3 Position;
in vec2 UV;
in vec3 TangentX;
in vec3 TangentY;
in vec3 TangentZ;

layout(binding=0) uniform sampler2D TexBase;
layout(binding=1) uniform sampler2D TexSpecular;
layout(binding=2) uniform sampler2D TexNormal;
layout(binding=3) uniform samplerCube TexDistance0;
	
uniform mat4 mvinv;
uniform mat4 mvp;

uniform vec4 Light0_Pos;
uniform vec4 Light0_Kd;

layout(location=0) out vec4 Output;

int samples=20;

vec3 sampleOffsetDirections[20]=vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);  

void main()
{
	vec4 temp=2.0*texture(TexNormal, UV)-1.0;
	vec4 Base=texture2D(TexBase, UV);
	vec3 Specular=texture(TexSpecular, UV).xyz;
//	vec3 n=normalize(vec3(TangentX.z, TangentY.z, TangentZ.z));
	vec3 n=normalize(vec3(dot(TangentX, temp.xyz), dot(TangentY, temp.xyz), dot(TangentZ, temp.xyz)));
	vec3 e=mvinv[3].xyz-Position, r;

	vec3 l0=Light0_Pos.xyz-Position;

	// Light volume, distance attenuation, and shadows need to be done before light and eye vector normalization

	// Volume
	vec4 l0_volume=vec4(clamp(dot(l0, e)/dot(e, e), 0.0, 1.0)*e-l0, 0.0);
	l0_volume.w=1.0/(pow(Light0_Pos.w*0.125*dot(l0_volume.xyz, l0_volume.xyz), 2.0)+1.0);

	// Attenuation = 1.0-(Light_Position*(1/Light_Radius))^2
	float l0_atten=max(0.0, 1.0-length(l0*Light0_Pos.w));

	float Shadow0=0.0;
	float Radius0=length(l0)*Light0_Pos.w;
	for(int i=0;i<samples;i++)
	{
		if((texture(TexDistance0, -l0+sampleOffsetDirections[i]*Radius0).x+0.01)>=Radius0)
			Shadow0+=1.0/samples;
	}

	e=normalize(e);
	l0=normalize(l0);

	//reflect=normalize((2.0*dot(e, n))*n-e);
	r=reflect(-e, n);

	// Diffuse = Kd*(N.L)
	vec3 l0_diffuse=Light0_Kd.rgb*max(0.0, dot(l0, n));

	// Specular = Ks*((R.L)^n)*(N.L)*Gloss
	vec3 l0_specular=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(l0, r), 16.0)*dot(l0, n)*Base.a);

	// I=(base*diffuse+specular)*shadow*attenuation+volumelight
	temp.xyz =(Base.xyz*l0_diffuse+l0_specular*Specular)*Shadow0*l0_atten*(1.0-l0_volume.w)+(l0_volume.w*Light0_Kd.xyz);

	Output=vec4(temp.xyz, 1.0);
}
