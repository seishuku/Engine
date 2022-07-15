#include <stdlib.h>
#include <stdint.h>
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../gl_objects.h"
#include "../system/system.h"
#include "beam_gl.h"

// Bring in matrices from main render code
extern matrix Projection, ModelView;

// Local handles for buffer objects
GLuint BeamVAO, BeamVBO, BeamEBO;
GLuint BeamShader;

float *BeamVB=NULL;

int32_t InitBeam(void)
{
	const int32_t segments=10, rings=9;
	const int32_t vertexCount=(rings+1)*(segments+1);
	const int32_t triangleCount=(rings*segments-segments)*2;

	BeamShader=CreateShaderProgram((ProgNames_t) { "./shaders/beam_v.glsl", "./shaders/beam_f.glsl", NULL, NULL });

	glCreateVertexArrays(1, &BeamVAO);

	glCreateBuffers(1, &BeamVBO);
	glNamedBufferData(BeamVBO, sizeof(vec4)*vertexCount, NULL, GL_STREAM_DRAW);

	BeamVB=(float *)malloc(sizeof(vec4)*vertexCount);

	if(!BeamVB)
		return 0;

	glVertexArrayVertexBuffer(BeamVAO, 0, BeamVBO, 0, sizeof(vec4));

	glVertexArrayAttribFormat(BeamVAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(BeamVAO, 0, 0);
	glEnableVertexArrayAttrib(BeamVAO, 0);

	glCreateBuffers(1, &BeamEBO);
	glNamedBufferData(BeamEBO, sizeof(uint16_t)*3*triangleCount, NULL, GL_STATIC_DRAW);

	glVertexArrayElementBuffer(BeamVAO, BeamEBO);

	uint16_t *pTris=glMapNamedBuffer(BeamEBO, GL_WRITE_ONLY);

	if(pTris)
	{
		for(int32_t i=0;i<rings;i++)
		{
			int32_t k1=i*(segments+1);
			int32_t k2=k1+segments+1;

			for(int32_t j=0;j<segments;j++, k1++, k2++)
			{
				if(i!=0)
				{
					*pTris++=k1;
					*pTris++=k1+1;
					*pTris++=k2;
				}

				// k1+1 => k2 => k2+1
				if(i!=(rings-1))
				{
					*pTris++=k1+1;
					*pTris++=k2+1;
					*pTris++=k2;
				}
			}
		}

		glUnmapNamedBuffer(BeamEBO);
	}
	else
		return 0;

	return 1;
}

void DrawBeam(const vec3 start, const vec3 end, const vec3 color, const float radius)
{
	matrix local;
	float vec[3]={ end[0]-start[0], end[1]-start[1], end[2]-start[2] };
	float length=Vec3_Length(vec);
	const int32_t segments=10, rings=9;
	const int32_t vertexCount=(rings+1)*(segments+1);
	const int32_t triangleCount=(rings*segments-segments)*2;

	vec4 *pVerts=(vec4 *)BeamVB;

	if(pVerts)
	{
		float sectorStep=2.0f*PI/segments;
		float stackStep=PI/rings;

		for(int32_t i=0;i<=rings;i++)
		{
			float stackAngle=0.5f*PI-i*stackStep;

			float xy=radius*cosf(stackAngle);
			float z=radius*sinf(stackAngle);

			for(int32_t j=0;j<=segments;j++)
			{
				float sectorAngle=j*sectorStep;

				float x=xy*cosf(sectorAngle);
				float y=xy*sinf(sectorAngle);

				// This is just a sphere that gets stretched halfway through
				if(i>rings/2)
					Vec4_Set(*pVerts++, x, y, z, 1.0f);
				else
					Vec4_Set(*pVerts++, x, y, z+length, 1.0f);
			}
		}

		glNamedBufferSubData(BeamVBO, 0, sizeof(vec4)*vertexCount, BeamVB);
	}

	glUseProgram(BeamShader);
	glUniformMatrix4fv(0, 1, GL_FALSE, Projection);
	glUniformMatrix4fv(1, 1, GL_FALSE, ModelView);

	MatrixIdentity(local);
	MatrixAlignPoints(start, end, (vec3) { 0.0f, 0.0f, -1.0f }, local);
	glUniformMatrix4fv(2, 1, GL_FALSE, local);

	glUniform4f(3, color[0], color[1], color[2], 1.0f/radius);
	glUniform3fv(4, 1, (vec3) { 0.0f, 0.0f, 0.0f });
	glUniform3fv(5, 1, (vec3) { 0.0f, 0.0f, length });

	glBindVertexArray(BeamVAO);
	glDrawElements(GL_TRIANGLES, triangleCount*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glUniform4f(3, 999.0f, 999.0f, 999.0f, 1.0f/(radius*4));
	//glDrawElements(GL_TRIANGLES, triangleCount*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void DestroyBeam(void)
{
	FREE(BeamVB);
}