#version 430

layout (triangles) in;
layout (line_strip, max_vertices=6) out;

in vec3 Position[];
in mat3 Tangent[];

out vec3 Color;

uniform mat4 proj;
uniform mat4 mv;

void main()
{
    gl_Position=proj*mv*vec4(Position[0], 1.0);
    Color=vec3(1.0, 0.0, 0.0);
    EmitVertex();
    gl_Position=proj*mv*(vec4(Position[0], 1.0)+vec4(Tangent[0][0], 0.0));
    Color=vec3(1.0, 0.0, 0.0);
    EmitVertex();
    EndPrimitive();
    gl_Position=proj*mv*vec4(Position[0], 1.0);
    Color=vec3(0.0, 1.0, 0.0);
    EmitVertex();
    gl_Position=proj*mv*(vec4(Position[0], 1.0)+vec4(Tangent[0][1], 0.0));
    Color=vec3(0.0, 1.0, 0.0);
    EmitVertex();
    EndPrimitive();
    gl_Position=proj*mv*vec4(Position[0], 1.0);
    Color=vec3(0.0, 0.0, 1.0);
    EmitVertex();
    gl_Position=proj*mv*(vec4(Position[0], 1.0)+vec4(Tangent[0][2], 0.0));
    Color=vec3(0.0, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
}  