#version 430

in vec3 Position;
in vec2 UV;
in mat3 Tangent;
in vec3 Color;

layout(binding=0) uniform sampler2D TexBase;
layout(binding=1) uniform sampler2D TexSpecular;
layout(binding=2) uniform sampler2D TexNormal;
layout(binding=3) uniform samplerCubeArrayShadow TexDistance;
	
uniform mat4 mv;
uniform mat4 local;

struct Light_t
{
	int ID;

	uint Pad[3];

	vec4 Position;
	vec4 Kd;
};

layout(std430, binding=0) buffer layoutLights
{
	Light_t Lights[];
};

uniform int NumLights;

uniform vec3 Beam_Start0;
uniform vec3 Beam_End0;

uniform vec3 Beam_Start1;
uniform vec3 Beam_End1;

layout(location=0) out vec4 Output;

vec3 ClosestPointOnSegment(vec3 point, vec3 start, vec3 end)
{
	// Find the slope of the line and length
	vec3 slope=end-start;
	float slopeLen=dot(slope, slope);

	// Project the point-start direction onto the line
	// and divie by the length to normalize.
	// Clamp to 0-1 range to limit to bounds of start and end points.
	float dist=clamp(dot(point-start, slope)/slopeLen, 0.0, 1.0);

	return start+dist*slope;
}

float SpotLight(vec3 pos, vec3 dir, float innerCutOff, float outerCutOff, float exponent)
{
	float outerCutOffAngle=cos(radians(outerCutOff));
	float innercutOffAngle=cos(radians(innerCutOff));

	float spot=dot(normalize(dir), -pos);

	if(spot<outerCutOffAngle)
		return 0.0;
	else
		return pow(smoothstep(outerCutOffAngle, innercutOffAngle, spot), exponent);
}

void main()
{
	vec4 Base=texture(TexBase, UV);
	vec3 Specular=texture(TexSpecular, UV).xyz;

	// Use this for vertex normals
//	vec3 n=normalize(mat3(local)*Tangent[2]);			// Vertex normals
	// Use this for normal mapped normals
	vec3 n=normalize(mat3(local)*Tangent*(2*texture(TexNormal, UV)-1).xyz);
	vec3 uE=inverse(mv)[3].xyz-Position;
	vec3 e=normalize(uE);
	vec3 r=reflect(-e, n);

	// zero out the lighting terms accumulator
	vec3 temp=vec3(0.0);

	for(int i=0;i<NumLights;i++)
	{
		vec3 lPos=Lights[i].Position.xyz-Position;

		// Light volume, distance attenuation, and shadows need to be done before light and eye vector normalization

		// Volume
		vec4 lVolume=vec4(clamp(dot(lPos, uE)/dot(uE, uE), 0.0, 1.0)*uE-lPos, 0.0);
		lVolume.w=1.0/(pow(Lights[i].Position.w*0.5*dot(lVolume.xyz, lVolume.xyz), 2.0)+1.0);

		// Attenuation = 1.0-(Light_Position*(1/Light_Radius))^2
		float lAtten=max(0.0, 1.0-length(lPos*Lights[i].Position.w));

		// Shadow map compare, divide the light distance by the radius to match the depth map distance space
		float Shadow=texture(TexDistance, vec4(-lPos, i), length(lPos)*Lights[i].Position.w);

		// Now we can normalize the light position vector
		lPos=normalize(lPos);

		// Diffuse = Kd*(N.L)
		vec3 lDiffuse=Lights[i].Kd.rgb*max(0.0, dot(lPos, n));

		// Specular = Ks*((R.L)^n)*(N.L)*Gloss
		vec3 lSpecular=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(lPos, r), 16.0)*dot(lPos, n));

		// Light 0 is the only that is a spotlight
		// Multiply it with attenuation, so it mixes in with everything else correctly
		if(i==0)
			lAtten*=SpotLight(lPos, vec3(0.0, -0.5, -0.5), 22.5, 90.0, 64.0);

		// I=(base*diffuse+specular)*shadow*attenuation*lightvolumeatten+volumelight
		temp+=(Base.xyz*lDiffuse+(lSpecular*Specular))*Shadow*lAtten*(1.0-lVolume.w)+(lVolume.w*Lights[i].Kd.xyz);
	}

	// Beam area light stuff
	vec3 Line=ClosestPointOnSegment(Position, Beam_Start0, Beam_End0)-Position;
	vec3 Light=normalize(Line);

	vec3 NdotL=vec3(1.0, 1.0, 1.0)*max(0.0, dot(n, Light));
	vec3 RdotL=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(r, Light), 32.0)*dot(n, Light));

	float radius=1.0/75.0;
	float falloff=max(0.0, 1.0-length(Line*radius));

	temp+=(Base.xyz*NdotL+(RdotL*Specular))*falloff;

	Line=ClosestPointOnSegment(Position, Beam_Start1, Beam_End1)-Position;
	Light=normalize(Line);

	NdotL=vec3(1.0, 1.0, 1.0)*max(0.0, dot(n, Light));
	RdotL=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(r, Light), 32.0)*dot(n, Light));

	radius=1.0/75.0;
	falloff=max(0.0, 1.0-length(Line*radius));

	temp+=(Base.xyz*NdotL+(RdotL*Specular))*falloff;
	////

	Output=vec4(temp, 1.0);
}
