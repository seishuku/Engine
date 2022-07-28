#version 450

// This controls detail level, this could be dynamic
#define MAXSEG 100

layout(lines_adjacency) in;
layout(line_strip, max_vertices=MAXSEG+1) out;

uniform mat4 proj;
uniform mat4 mv;
uniform mat4 local;

// Cubic Bezier curve evaluation function
vec3 Bezier(float t, vec3 p0, vec3 p1, vec3 p2, vec3 p3)
{
	vec3 c=3.0*(p1-p0);
	vec3 b=3.0*(p2-p1)-c;
	vec3 a=p3-p0-c-b;

	return (a*(t*t*t))+(b*(t*t))+(c*t)+p0;
}

void main()
{
	for(int i=0;i<=MAXSEG;i++)
	{
		float t=float(i)/MAXSEG;

		gl_Position=proj*mv*local*vec4(Bezier(t, gl_in[0].gl_Position.xyz,
												 gl_in[1].gl_Position.xyz,
												 gl_in[2].gl_Position.xyz,
												 gl_in[3].gl_Position.xyz), 1.0);
		EmitVertex();
	}

	EndPrimitive();                                                                 
}
