#ifndef __3DS_H__
#define __3DS_H__

typedef struct
{
	char Name[32];
	float Ambient[3];
	float Diffuse[3];
	float Specular[3];
	float Emission[3];
	float Shininess;
	char Texture[32];
	unsigned int TexBaseID, TexNormalID, TexSpecularID;
} Material3DS_t;

typedef struct
{
	char Name[32];
	char MaterialName[32];
	int MaterialNumber;

	unsigned short NumVertex;
	float *Vertex, *UV, *Normal, *Tangent, *Binormal;

	unsigned short NumFace;
	unsigned short *Face;

	unsigned int VAO, VertID, ElemID;
} Mesh3DS_t;

typedef struct
{
	int NumMaterial;
	Material3DS_t *Material;

	int NumMesh;
	Mesh3DS_t *Mesh;
} Model3DS_t;

int Load3DS(Model3DS_t *Model, char *Filename);
void Free3DS(Model3DS_t *Model);

#endif
