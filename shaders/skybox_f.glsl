#version 430

layout(binding=0) uniform samplerCube Texture;

in vec3 UVW;

layout(location=0) out vec4 Output;

const float PI=3.1415926;

vec2 getUVLatLon(const vec3 v)
{
    float r=1.0/length(v);
    float phi=acos(v.y*r);
    float theta=atan(v.z*r, v.x*r);

    return vec2(0.5*(theta/PI)+0.5, -phi/PI);
}

vec2 getUVAngularMap(const vec3 v)
{
    float r=1.0/length(v);
    float phi=acos(v.z*r);
	float theta=atan(v.y*r, v.x*r);

	return 0.5*vec2((phi/PI)*cos(theta), (phi/PI)*sin(theta))+0.5;
}

void main()
{
/*    vec3 N = normalize(UVW);
    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;

    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(Texture, toLatLong(sampleVec)).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
//    Output=vec4(irradiance, 1.0);//pow(pow(2.0, 0.0)*texture(Texture, getUVLatLon(UVW)),  vec4(0.45, 0.45, 0.45, 0.0));*/
    Output=pow(2.0, 5.0)*texture(Texture, -UVW);
}
