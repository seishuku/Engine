#version 450

layout(points) in;
layout(line_strip, max_vertices=500) out;

uniform mat4 proj;
uniform mat4 mv;
uniform mat4 local;

uniform vec3 ControlPoints[4];

vec3 Bezier(float t, vec3 p0, vec3 p1, vec3 p2, vec3 p3)
{
	vec3 c=3.0*(p1-p0);
	vec3 b=3.0*(p2-p1)-c;
	vec3 a=p3-p0-c-b;

	return (a*(t*t*t))+(b*(t*t))+(c*t)+p0;
}

#define MAXSEG 100

void main()
{
	for(int i=0;i<MAXSEG;i++)
	{
		float t=float(i)/MAXSEG;

		gl_Position=proj*mv*local*vec4(Bezier(t, ControlPoints[0], ControlPoints[1], ControlPoints[2], ControlPoints[3]), 1.0);
		EmitVertex();
	}

	EndPrimitive();                                                                 
}
