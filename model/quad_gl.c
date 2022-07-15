#include <stdint.h>
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../system/system.h"

GLuint QuadVAO, QuadVBO;

void BuildQuadVBO(void)
{
	vec4 verts[]={
		{ -1.0f, +1.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, 0.0f, 1.0f },
		{ +1.0f, +1.0f, 0.0f, 1.0f },
		{ +1.0f, -1.0f, 0.0f, 1.0f }
	};

	glCreateVertexArrays(1, &QuadVAO);

	glCreateBuffers(1, &QuadVBO);
	glNamedBufferData(QuadVBO, sizeof(float)*4*4, verts, GL_STATIC_DRAW);

	glVertexArrayAttribFormat(QuadVAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(QuadVAO, 0, 0);
	glEnableVertexArrayAttrib(QuadVAO, 0);

	glVertexArrayVertexBuffer(QuadVAO, 0, QuadVBO, 0, sizeof(vec4));
}

void DrawQuad(void)
{
	glBindVertexArray(QuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
