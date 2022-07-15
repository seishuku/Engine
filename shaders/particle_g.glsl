#version 450

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

layout(location=0) uniform mat4 proj;
layout(location=1) uniform mat4 mv;

in vec4 Color[1];

out vec2 vUV;
out vec4 vColor;

void main()
{
	float Scale=gl_in[0].gl_Position.w*min(1.0, Color[0].w);		// Quad size.
	vec3 Pos=gl_in[0].gl_Position.xyz;	// Incoming vertex data.
	vec3 Right=vec3(mv[0].x, mv[1].x, mv[2].x);
	vec3 Up=vec3(mv[0].y, mv[1].y, mv[2].y);

	/* Quad as a single triangle strip:

		0 *----* 2
		  |   /|
		  |  / |
		  | /  |
		  |/   |
		1 *----* 3
	*/

	gl_Position=proj*mv*vec4(Pos-Right*Scale+Up*Scale, 1.0);
	vUV=vec2(0.0, 1.0);
	vColor=Color[0];
	EmitVertex();

	gl_Position=proj*mv*vec4(Pos-Right*Scale-Up*Scale, 1.0);
	vUV=vec2(0.0, 0.0);
	vColor=Color[0];
	EmitVertex();

	gl_Position=proj*mv*vec4(Pos+Right*Scale+Up*Scale, 1.0);
	vUV=vec2(1.0, 1.0);
	vColor=Color[0];
	EmitVertex();

	gl_Position=proj*mv*vec4(Pos+Right*Scale-Up*Scale, 1.0);
	vUV=vec2(1.0, 0.0);
	vColor=Color[0];
	EmitVertex();

	EndPrimitive();                                                                 
}
