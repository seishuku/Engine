#include "opengl.h"
#include "gl_objects.h"
#include "md5.h"
#include "md5_gl.h"
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define DBGPRINTF(...) { char buf[512]; snprintf(buf, sizeof(buf), __VA_ARGS__); OutputDebugString(buf); }
#else
#define DBGPRINTF(...) { fprintf(stderr, __VA_ARGS__); }
#endif

#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(x) ((char *)NULL+(x))
#endif

#ifndef FREE
#define FREE(p) { if(p) { free(p); p=NULL; } }
#endif

void DrawModelMD5(MD5_Model_t *Model)
{
	for(int i=0;i<Model->num_meshes;i++)
	{
		glBindVertexArray(Model->meshes[i].VAO);
		glDrawElements(GL_TRIANGLES, Model->meshes[i].num_tris*3, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	}

	glBindVertexArray(0);
}

void BuildVBOMD5(MD5_Model_t *Model)
{
	for(int i=0;i<Model->num_meshes;i++)
	{
		glGenBuffers(1, &Model->meshes[i].WeightID);
		glBindBuffer(GL_ARRAY_BUFFER, Model->meshes[i].WeightID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MD5_Weight_t)*Model->meshes[i].num_weights, Model->meshes[i].weights, GL_STATIC_COPY);

		glGenBuffers(1, &Model->meshes[i].VertID);
		glBindBuffer(GL_ARRAY_BUFFER, Model->meshes[i].VertID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MD5_Vertex_t)*Model->meshes[i].num_verts, Model->meshes[i].vertices, GL_STATIC_COPY);

		glGenVertexArrays(1, &Model->meshes[i].VAO);
		glBindVertexArray(Model->meshes[i].VAO);

		glGenBuffers(1, &Model->meshes[i].FinalVertID);
		glBindBuffer(GL_ARRAY_BUFFER, Model->meshes[i].FinalVertID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*20*Model->meshes[i].num_verts, NULL, GL_STATIC_COPY);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*20, BUFFER_OFFSET(sizeof(float)*0));	//Vertex
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float)*20, BUFFER_OFFSET(sizeof(float)*4));	//UV
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(float)*20, BUFFER_OFFSET(sizeof(float)*(4+4)));	//TANGENT
		glEnableVertexAttribArray(2);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(float)*20, BUFFER_OFFSET(sizeof(float)*(4+4+4)));	//BINORMAL
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float)*20, BUFFER_OFFSET(sizeof(float)*(4+4+4+4)));	//NORMAL
		glEnableVertexAttribArray(4);

		glGenBuffers(1, &Model->meshes[i].ElemID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Model->meshes[i].ElemID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*Model->meshes[i].num_tris*3, Model->meshes[i].triangles, GL_STATIC_DRAW);

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

	InterpolateSkeletons(&Model->Anim, Model->Anim.skelFrames[Model->frame], Model->Anim.skelFrames[Model->nextframe], Model->frameTime*Model->Anim.frameRate, Model->Skel);

#if 0
	// Do mesh skinning on CPU
	for(int i=0;i<Model->Model.num_meshes;i++)
	{
		PrepareMesh(&Model->Model.meshes[i], Model->Skel, Model->Model.meshes[i].vertexArray);

		glBindBuffer(GL_ARRAY_BUFFER, Model->Model.meshes[i].FinalVertID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*Model->Model.meshes[i].num_verts*20, Model->Model.meshes[i].vertexArray);
	}
#else
	// Do mesh skinning on GPU
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, Model->SkelSSBO);
	GLvoid *p=glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

	if(p)
	{
		float *fPtr=(float *)p;

		for(int i=0;i<Model->Model.num_joints;i++)
		{
			*fPtr++=Model->Skel[i].pos[0];
			*fPtr++=Model->Skel[i].pos[1];
			*fPtr++=Model->Skel[i].pos[2];
			*fPtr++=0.0f;	// Padding
			*fPtr++=Model->Skel[i].orient[0];
			*fPtr++=Model->Skel[i].orient[1];
			*fPtr++=Model->Skel[i].orient[2];
			*fPtr++=Model->Skel[i].orient[3];
		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	for(int i=0;i<Model->Model.num_meshes;i++)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Model->Model.meshes[i].VertID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Model->Model.meshes[i].WeightID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, Model->Model.meshes[i].FinalVertID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Model->SkelSSBO);

		glUseProgram(Objects[GLSL_MD5_GENVERTS_COMPUTE]);

		glDispatchCompute(Model->Model.meshes[i].num_verts, 1, 1);
	}
#endif
}

int LoadMD5Model(const char *Filename, Model_t *Model)
{
	char Mesh[256]="\0";
	char Anim[256]="\0";

	strcpy(Mesh, Filename);
	strcpy(Anim, Filename);

	strcat(Mesh, ".md5mesh");
	strcat(Anim, ".md5anim");

	if(LoadMD5(&Model->Model, Mesh))
		BuildVBOMD5(&Model->Model);
	else
		return 0;

	// Load MD5 animation frames
	if(LoadAnim(&Model->Anim, Anim))
	{
		// Generate an SSBO to store the interpolated skeleton
		glGenBuffers(1, &Model->SkelSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, Model->SkelSSBO);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(float)*8*Model->Anim.num_joints, NULL, GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT|GL_MAP_WRITE_BIT);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// Allocate system memory to store interpolated skeleton
		Model->Skel=(MD5_Joint_t *)malloc(sizeof(MD5_Joint_t)*Model->Anim.num_joints);

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
	for(int i=0;i<Model->Model.num_meshes;i++)
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
