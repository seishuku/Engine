#ifndef __Q2BSP_H__
#define __Q2BSP_H__

typedef struct
{
	GLuint VAO, VBO;
	GLuint TexBaseID;
	GLuint TexSpecularID;
	GLuint TexNormalID;
	uint32_t NumTris;
} Q2BSP_Mesh_t;

typedef struct
{
	uint32_t NumMesh;
	Q2BSP_Mesh_t *Mesh;
	vec3 PlayerOrigin;
	float PlayerDirection;
} Q2BSP_Model_t;

bool LoadQ2BSP(Q2BSP_Model_t *Model, const char *Filename);
void DrawQ2BSP(Q2BSP_Model_t *Model);
void DestroyQ2BSP(Q2BSP_Model_t *Model);

#endif
