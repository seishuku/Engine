#ifndef __OBJ_H__
#define __OBJ_H__

#include "math.h"

typedef struct
{
	char Name[255];
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	vec3 Emission;
	float Shininess;
	char Texture[255];
	unsigned int TexBaseID, TexNormalID, TexSpecularID;
} MaterialOBJ_t;

typedef struct
{
	char Name[255];
	char MaterialName[255];
	int MaterialNumber;

	unsigned long NumFace;
	unsigned long *Face;

	unsigned int VAO, ElemID;
} MeshOBJ_t;

typedef struct
{
	char MaterialFilename[255];
	int NumMaterial;
	MaterialOBJ_t *Material;

	unsigned long NumVertex;
	float *Vertex, *UV, *Normal, *Tangent, *Binormal;
	unsigned int VertID;

	int NumMesh;
	MeshOBJ_t *Mesh;
} ModelOBJ_t;

int LoadOBJ(ModelOBJ_t *Model, const char *Filename);
void FreeOBJ(ModelOBJ_t *Model);

#endif
