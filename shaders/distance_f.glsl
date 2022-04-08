#version 330

in vec3 Position;

uniform vec4 Light_Pos;

void main()
{
	// Instead of dividing by the far plane, I prefer using the set radius of the point light.
	gl_FragDepth=length(Light_Pos.xyz-Position)*Light_Pos.w;
}
