#version 450

layout(lines_adjacency) in;
layout(line_strip, max_vertices=128) out;

uniform mat4 proj;
uniform mat4 mv;
uniform mat4 local;

// This controls detail level
uniform uint numSegments;

in vec4 gColor[];
out vec4 Color;

// Cubic Bezier curve evaluation function
vec4 Bezier(float t, vec4 p0, vec4 p1, vec4 p2, vec4 p3)
{
	vec4 c=3.0*(p1-p0);
	vec4 b=3.0*(p2-p1)-c;
	vec4 a=p3-p0-c-b;

	return (a*(t*t*t))+(b*(t*t))+(c*t)+p0;
}

void main()
{
	for(int i=0;i<=numSegments;i++)
	{
		float t=float(i)/numSegments;

		Color=Bezier(t, gColor[0], gColor[1], gColor[2], gColor[3]);
		gl_Position=proj*mv*local*Bezier(t, gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);

		EmitVertex();
	}

	EndPrimitive();                                                                 
}
