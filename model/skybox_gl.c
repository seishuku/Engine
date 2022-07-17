#include <stdint.h>
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../system/system.h"

GLuint SkyboxVAO, SkyboxVBO, SkyboxEBO;

void DrawSkybox(void)
{
	glBindVertexArray(SkyboxVAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
}

void BuildSkyboxVBO(void)
{
	float scale=10000.0f, w=1.0f, tex_scale=1.0f;
	vec4 SkyboxVerts[]=
	{
		{ +scale, -scale, -scale, w }, { +tex_scale, -tex_scale, -tex_scale, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, //0 Right
		{ +scale, -scale, +scale, w }, { +tex_scale, -tex_scale, +tex_scale, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, //1
		{ +scale, +scale, +scale, w }, { +tex_scale, +tex_scale, +tex_scale, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, //2
		{ +scale, +scale, -scale, w }, { +tex_scale, +tex_scale, -tex_scale, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, //3

		{ -scale, -scale, +scale, w }, { -tex_scale, -tex_scale, +tex_scale, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, //4 Left
		{ -scale, -scale, -scale, w }, { -tex_scale, -tex_scale, -tex_scale, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, //5
		{ -scale, +scale, -scale, w }, { -tex_scale, +tex_scale, -tex_scale, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, //6
		{ -scale, +scale, +scale, w }, { -tex_scale, +tex_scale, +tex_scale, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, //7

		{ -scale, +scale, -scale, w }, { -tex_scale, +tex_scale, -tex_scale, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f,-1.0f, 0.0f, 0.0f }, //8 Top
		{ +scale, +scale, -scale, w }, { +tex_scale, +tex_scale, -tex_scale, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f,-1.0f, 0.0f, 0.0f }, //9
		{ +scale, +scale, +scale, w }, { +tex_scale, +tex_scale, +tex_scale, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f,-1.0f, 0.0f, 0.0f }, //10
		{ -scale, +scale, +scale, w }, { -tex_scale, +tex_scale, +tex_scale, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f,-1.0f, 0.0f, 0.0f }, //11

		{ -scale, -scale, +scale, w }, { -tex_scale, -tex_scale, +tex_scale, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, //12 Bottom
		{ +scale, -scale, +scale, w }, { +tex_scale, -tex_scale, +tex_scale, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, //13
		{ +scale, -scale, -scale, w }, { +tex_scale, -tex_scale, -tex_scale, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, //14
		{ -scale, -scale, -scale, w }, { -tex_scale, -tex_scale, -tex_scale, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, //15

		{ +scale, -scale, +scale, w }, { +tex_scale, -tex_scale, +tex_scale, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, //16 Front
		{ -scale, -scale, +scale, w }, { -tex_scale, -tex_scale, +tex_scale, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, //17
		{ -scale, +scale, +scale, w }, { -tex_scale, +tex_scale, +tex_scale, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, //18
		{ +scale, +scale, +scale, w }, { +tex_scale, +tex_scale, +tex_scale, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f }, //19

		{ -scale, -scale, -scale, w }, { -tex_scale, -tex_scale, -tex_scale, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, //20 Back
		{ +scale, -scale, -scale, w }, { +tex_scale, -tex_scale, -tex_scale, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, //21
		{ +scale, +scale, -scale, w }, { +tex_scale, +tex_scale, -tex_scale, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, //22
		{ -scale, +scale, -scale, w }, { -tex_scale, +tex_scale, -tex_scale, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }  //23
	};
	uint16_t SkyboxTris[]=
	{
		 0,  1,  2,  3,  0, 2,	// Right
		 4,  5,  6,  7,  4, 6,	// Left
		 8,  9, 10, 11,  8, 10,	// Top
		12, 13, 14, 15, 12, 14,	// Bottom
		16, 17, 18, 19, 16, 18, // Front
		20, 21, 22, 23, 20, 22	// Back
	};

	// Set vertex attribute pointer layouts
	glCreateVertexArrays(1, &SkyboxVAO);

	// Vertex
	glVertexArrayAttribFormat(SkyboxVAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(SkyboxVAO, 0, 0);
	glEnableVertexArrayAttrib(SkyboxVAO, 0);

	// UV
	glVertexArrayAttribFormat(SkyboxVAO, 1, 4, GL_FLOAT, GL_FALSE, sizeof(vec4));
	glVertexArrayAttribBinding(SkyboxVAO, 1, 0);
	glEnableVertexArrayAttrib(SkyboxVAO, 1);

	// Tangent
	glVertexArrayAttribFormat(SkyboxVAO, 2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4)*2);
	glVertexArrayAttribBinding(SkyboxVAO, 2, 0);
	glEnableVertexArrayAttrib(SkyboxVAO, 2);

	// Binormal
	glVertexArrayAttribFormat(SkyboxVAO, 3, 4, GL_FLOAT, GL_FALSE, sizeof(vec4)*3);
	glVertexArrayAttribBinding(SkyboxVAO, 3, 0);
	glEnableVertexArrayAttrib(SkyboxVAO, 3);

	// Normal
	glVertexArrayAttribFormat(SkyboxVAO, 4, 4, GL_FLOAT, GL_FALSE, sizeof(vec4)*4);
	glVertexArrayAttribBinding(SkyboxVAO, 4, 0);
	glEnableVertexArrayAttrib(SkyboxVAO, 4);

	// Create vertex buffer object
	glCreateBuffers(1, &SkyboxVBO);
	glNamedBufferData(SkyboxVBO, sizeof(vec4)*5*24, SkyboxVerts, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(SkyboxVAO, 0, SkyboxVBO, 0, sizeof(vec4)*5);

	// Create element buffer object
	glCreateBuffers(1, &SkyboxEBO);
	glNamedBufferData(SkyboxEBO, sizeof(uint16_t)*36, SkyboxTris, GL_STATIC_DRAW);
	glVertexArrayElementBuffer(SkyboxVAO, SkyboxEBO);

	glBindVertexArray(0);
}
