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
	
uniform mat4 mv;
uniform mat4 local;

uniform vec4 Light0_Pos;
uniform vec4 Light0_Kd;
uniform vec4 Light1_Pos;
uniform vec4 Light1_Kd;
uniform vec4 Light2_Pos;
uniform vec4 Light2_Kd;
uniform vec4 Light3_Pos;
uniform vec4 Light3_Kd;
uniform vec4 Light4_Pos;
uniform vec4 Light4_Kd;

layout(location=0) out vec4 Output;

void main()
{
	vec4 temp=2.0*texture(TexNormal, UV)-1.0;
	vec4 Base=texture2D(TexBase, UV);
	vec3 Specular=texture(TexSpecular, UV).xyz;
//	vec3 n=normalize(mat3(local)*vec3(TangentX.z, TangentY.z, TangentZ.z));
	vec3 n=normalize(mat3(local)*vec3(dot(TangentX, temp.xyz), dot(TangentY, temp.xyz), dot(TangentZ, temp.xyz)));
	vec3 e=inverse(mv)[3].xyz-Position, r;

	vec3 l0=Light0_Pos.xyz-Position;
	vec3 l1=Light1_Pos.xyz-Position;
	vec3 l2=Light2_Pos.xyz-Position;
	vec3 l3=Light3_Pos.xyz-Position;
	vec3 l4=Light4_Pos.xyz-Position;

	// Light volume, distance attenuation, and shadows need to be done before light and eye vector normalization

	// Volume
	vec4 l0_volume=vec4(clamp(dot(l0, e)/dot(e, e), 0.0, 1.0)*e-l0, 0.0);
	l0_volume.w=1.0/(pow(Light0_Pos.w*0.5*dot(l0_volume.xyz, l0_volume.xyz), 2.0)+1.0);

	vec4 l1_volume=vec4(clamp(dot(l1, e)/dot(e, e), 0.0, 1.0)*e-l1, 0.0);
	l1_volume.w=1.0/(pow(Light1_Pos.w*0.5*dot(l1_volume.xyz, l1_volume.xyz), 2.0)+1.0);

	vec4 l2_volume=vec4(clamp(dot(l2, e)/dot(e, e), 0.0, 1.0)*e-l2, 0.0);
	l2_volume.w=1.0/(pow(Light2_Pos.w*0.5*dot(l2_volume.xyz, l2_volume.xyz), 2.0)+1.0);

	vec4 l3_volume=vec4(clamp(dot(l3, e)/dot(e, e), 0.0, 1.0)*e-l3, 0.0);
	l3_volume.w=1.0/(pow(Light3_Pos.w*0.5*dot(l3_volume.xyz, l3_volume.xyz), 2.0)+1.0);

	vec4 l4_volume=vec4(clamp(dot(l4, e)/dot(e, e), 0.0, 1.0)*e-l4, 0.0);
	l4_volume.w=1.0/(pow(Light4_Pos.w*0.5*dot(l4_volume.xyz, l4_volume.xyz), 2.0)+1.0);

	// Attenuation = 1.0-(Light_Position*(1/Light_Radius))^2
	float l0_atten=max(0.0, 1.0-length(l0*Light0_Pos.w));
	float l1_atten=max(0.0, 1.0-length(l1*Light1_Pos.w));
	float l2_atten=max(0.0, 1.0-length(l2*Light2_Pos.w));
	float l3_atten=max(0.0, 1.0-length(l3*Light3_Pos.w));
	float l4_atten=max(0.0, 1.0-length(l4*Light4_Pos.w));

	// Shadow map compare, divide the light distance by the radius to match the depth map distance space
	float Shadow0=(texture(TexDistance0, -l0).x+0.01)>(length(l0)*Light0_Pos.w)?1.0:0.0;

	e=normalize(e);
	l0=normalize(l0);
	l1=normalize(l1);
	l2=normalize(l2);
	l3=normalize(l3);
	l4=normalize(l4);

	//reflect=normalize((2.0*dot(e, n))*n-e);
	r=reflect(-e, n);

	// Diffuse = Kd*(N.L)
	vec3 l0_diffuse=Light0_Kd.rgb*max(0.0, dot(l0, n));
	vec3 l1_diffuse=Light1_Kd.rgb*max(0.0, dot(l1, n));
	vec3 l2_diffuse=Light2_Kd.rgb*max(0.0, dot(l2, n));
	vec3 l3_diffuse=Light3_Kd.rgb*max(0.0, dot(l3, n));
	vec3 l4_diffuse=Light4_Kd.rgb*max(0.0, dot(l4, n));

	// Specular = Ks*((R.L)^n)*(N.L)*Gloss
	vec3 l0_specular=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(l0, r), 16.0)*dot(l0, n)*Base.a);
	vec3 l1_specular=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(l1, r), 16.0)*dot(l1, n)*Base.a);
	vec3 l2_specular=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(l2, r), 16.0)*dot(l2, n)*Base.a);
	vec3 l3_specular=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(l3, r), 16.0)*dot(l3, n)*Base.a);
	vec3 l4_specular=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(l4, r), 16.0)*dot(l4, n)*Base.a);

	// I=(base*diffuse+specular)*shadow*attenuation+volumelight
	temp =vec4((Base.xyz*l0_diffuse+l0_specular*Specular)*Shadow0*l0_atten*(1.0-l0_volume.w)+(l0_volume.w*Light0_Kd.xyz), 1.0);
	temp+=vec4((Base.xyz*l1_diffuse+l1_specular*Specular)*l1_atten*(1.0-l1_volume.w)+(l1_volume.w*Light1_Kd.xyz), 1.0);
	temp+=vec4((Base.xyz*l2_diffuse+l2_specular*Specular)*l2_atten*(1.0-l2_volume.w)+(l2_volume.w*Light2_Kd.xyz), 1.0);
	temp+=vec4((Base.xyz*l3_diffuse+l3_specular*Specular)*l3_atten*(1.0-l3_volume.w)+(l3_volume.w*Light3_Kd.xyz), 1.0);
	temp+=vec4((Base.xyz*l4_diffuse+l4_specular*Specular)*l4_atten*(1.0-l4_volume.w)+(l4_volume.w*Light4_Kd.xyz), 1.0);

	Output=vec4(temp.xyz, 1.0);
}
