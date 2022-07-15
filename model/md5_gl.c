#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../gl_objects.h"
#include "../image/image.h"
#include "../system/system.h"
#include "md5.h"
#include "md5_gl.h"

void DrawModelMD5(MD5_Model_t *Model)
{
	for(int32_t i=0;i<Model->num_meshes;i++)
	{
		glBindVertexArray(Model->meshes[i].VAO);
		glDrawElements(GL_TRIANGLES, Model->meshes[i].num_tris*3, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	}

	glBindVertexArray(0);
}

void BuildVBOMD5(MD5_Model_t *Model)
{
	for(int32_t i=0;i<Model->num_meshes;i++)
	{
		glCreateBuffers(1, &Model->meshes[i].WeightID);
		glNamedBufferData(Model->meshes[i].WeightID, sizeof(MD5_Weight_t)*Model->meshes[i].num_weights, Model->meshes[i].weights, GL_STATIC_COPY);

		glCreateBuffers(1, &Model->meshes[i].VertID);
		glNamedBufferData(Model->meshes[i].VertID, sizeof(MD5_Vertex_t)*Model->meshes[i].num_verts, Model->meshes[i].vertices, GL_STATIC_COPY);

		// Create the vertex array object
		glCreateVertexArrays(1, &Model->meshes[i].VAO);

		// Create vertex buffer object
		glCreateBuffers(1, &Model->meshes[i].FinalVertID);

		// Assign the vertex buffer to the vertex array on binding point 0, and set buffer stride
		glVertexArrayVertexBuffer(Model->meshes[i].VAO, 0, Model->meshes[i].FinalVertID, 0, sizeof(float)*20);

		// Allocate vertex buffer data
		glNamedBufferData(Model->meshes[i].FinalVertID, sizeof(float)*20*Model->meshes[i].num_verts, NULL, GL_STATIC_COPY);

		// Vertex
		glVertexArrayAttribFormat(Model->meshes[i].VAO, 0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*0);
		glVertexArrayAttribBinding(Model->meshes[i].VAO, 0, 0);
		glEnableVertexArrayAttrib(Model->meshes[i].VAO, 0);

		// UV
		glVertexArrayAttribFormat(Model->meshes[i].VAO, 1, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4);
		glVertexArrayAttribBinding(Model->meshes[i].VAO, 1, 0);
		glEnableVertexArrayAttrib(Model->meshes[i].VAO, 1);

		// Tangent
		glVertexArrayAttribFormat(Model->meshes[i].VAO, 2, 4, GL_FLOAT, GL_FALSE, sizeof(float)*(4+4));
		glVertexArrayAttribBinding(Model->meshes[i].VAO, 2, 0);
		glEnableVertexArrayAttrib(Model->meshes[i].VAO, 2);

		// Binormal
		glVertexArrayAttribFormat(Model->meshes[i].VAO, 3, 4, GL_FLOAT, GL_FALSE, sizeof(float)*(4+4+4));
		glVertexArrayAttribBinding(Model->meshes[i].VAO, 3, 0);
		glEnableVertexArrayAttrib(Model->meshes[i].VAO, 3);

		// Normal
		glVertexArrayAttribFormat(Model->meshes[i].VAO, 4, 4, GL_FLOAT, GL_FALSE, sizeof(float)*(4+4+4+4));
		glVertexArrayAttribBinding(Model->meshes[i].VAO, 4, 0);
		glEnableVertexArrayAttrib(Model->meshes[i].VAO, 4);

		// Create index buffer object and assign it
		glCreateBuffers(1, &Model->meshes[i].ElemID);
		glNamedBufferData(Model->meshes[i].ElemID, sizeof(uint32_t)*Model->meshes[i].num_tris*3, Model->meshes[i].triangles, GL_STATIC_DRAW);
		glVertexArrayElementBuffer(Model->meshes[i].VAO, Model->meshes[i].ElemID);

		glBindVertexArray(0);
	}
}

void UpdateAnimation(Model_t *Model, float dt)
{
	Model->frameTime+=dt;

	if(Model->frameTime>=(1.0f/Model->Anim.frameRate))
	{
		Model->frame++;
		Model->nextframe++;

		Model->frameTime=0.0f;

		Model->frame%=Model->Anim.num_frames-1;
		Model->nextframe%=Model->Anim.num_frames-1;
	}

	// Do mesh skinning on GPU
	// Build interpolated skeleton to feed into compute shader
	if(Model->Skel)
	{
		float *fPtr=(float *)Model->Skel;

		for(int32_t i=0;i<Model->Anim.num_joints;i++)
		{
			float interp=Model->frameTime*Model->Anim.frameRate;
			vec3 pos;
			vec4 orient;

			// Liear interpolate position
			Vec3_Lerp(
				Model->Anim.skelFrames[Model->frame][i].pos,
				Model->Anim.skelFrames[Model->nextframe][i].pos,
				interp, pos
			);

			*fPtr++=pos[0];
			*fPtr++=pos[1];
			*fPtr++=pos[2];
			*fPtr++=0.0f;	// Padding

			// Spherical interpolate rotation
			QuatSlerp(
				Model->Anim.skelFrames[Model->frame][i].orient,
				Model->Anim.skelFrames[Model->nextframe][i].orient,
				orient, interp
			);

			*fPtr++=orient[0];
			*fPtr++=orient[1];
			*fPtr++=orient[2];
			*fPtr++=orient[3];
		}
	}

	glNamedBufferSubData(Model->SkelSSBO, 0, sizeof(float)*8*Model->Anim.num_joints, Model->Skel);

	for(int32_t i=0;i<Model->Model.num_meshes;i++)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Model->Model.meshes[i].VertID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Model->Model.meshes[i].WeightID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, Model->Model.meshes[i].FinalVertID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Model->SkelSSBO);

		glUseProgram(Objects[GLSL_MD5_GENVERTS_COMPUTE]);

		glDispatchCompute(Model->Model.meshes[i].num_verts, 1, 1);
	}
}

int32_t LoadMD5Model(const char *Filename, Model_t *Model)
{
	char Mesh[256]="\0";
	char Anim[256]="\0";
	char Base[256]="\0";
	char Specular[256]="\0";
	char Normal[256]="\0";

	strcat(strcpy(Mesh, Filename), ".md5mesh");
	strcat(strcpy(Anim, Filename), ".md5anim");
	strcat(strcpy(Base, Filename), ".qoi");
	strcat(strcpy(Specular, Filename), "_s.qoi");
	strcat(strcpy(Normal, Filename), "_n.qoi");

	if(LoadMD5(&Model->Model, Mesh))
		BuildVBOMD5(&Model->Model);
	else
		return 0;

	Model->Base=Image_Upload(Base, IMAGE_MIPMAP|IMAGE_TRILINEAR);
	Model->Specular=Image_Upload(Specular, IMAGE_MIPMAP|IMAGE_TRILINEAR);
	Model->Normal=Image_Upload(Normal, IMAGE_MIPMAP|IMAGE_TRILINEAR|IMAGE_NORMALIZE);

	// Load MD5 animation frames
	if(LoadAnim(&Model->Anim, Anim))
	{
		// Generate an SSBO to store the interpolated skeleton
		glCreateBuffers(1, &Model->SkelSSBO);
		glNamedBufferData(Model->SkelSSBO, sizeof(float)*8*Model->Anim.num_joints, NULL, GL_STREAM_DRAW);

		// Allocate system memory to store interpolated skeleton
		Model->Skel=(float *)malloc(sizeof(float)*8*Model->Anim.num_joints);

		if(Model->Skel==NULL)
			return 0;

		// Initial frame setting
		Model->frame=0;
		Model->nextframe=1;
	}
	else
		return 0;

	return 1;
}

void DestroyMD5Model(Model_t *Model)
{
	for(int32_t i=0;i<Model->Model.num_meshes;i++)
	{
		glDeleteBuffers(1, &Model->Model.meshes[i].WeightID);
		glDeleteBuffers(1, &Model->Model.meshes[i].VertID);
		glDeleteBuffers(1, &Model->Model.meshes[i].FinalVertID);
		glDeleteBuffers(1, &Model->Model.meshes[i].ElemID);
		glDeleteVertexArrays(1, &Model->Model.meshes[i].VAO);
	}

	glDeleteBuffers(1, &Model->SkelSSBO);

	FREE(Model->Skel);
	FreeMD5(&Model->Model);
	FreeAnim(&Model->Anim);
}
