#ifndef __MD5_H__
#define __MD5_H__

#include "math.h"

// Joint
typedef struct
{
	char name[64];
	int parent;
	vec3 pos;
	vec4 orient;
} MD5_Joint_t;

// Vertex
typedef struct
{
	vec2 st;	// texture coordinates
	int start;		// start weight
	int count;		// weight count
} MD5_Vertex_t;

// Weight
typedef struct
{
	int joint;			// joint for this weight
	float bias;			// vertex weight
	vec2 pad;
	vec4 pos;		// vertex position (in joint space)
	vec4 tangent;	// tangent vector ("")
	vec4 binormal;	// binormal vector ("")
	vec4 normal;	// normal vector ("")
} MD5_Weight_t;

// Bounding box
typedef struct
{
	vec3 min;
	vec3 max;
} MD5_BBox_t;

// MD5 mesh
typedef struct
{
	int num_verts;
	MD5_Vertex_t *vertices;

	int num_tris;
	unsigned int *triangles;

	int num_weights;
	MD5_Weight_t *weights;

	char shader[256];

	//float *vertexArray;
	unsigned int VAO;
	unsigned int WeightID, VertID;
	unsigned int FinalVertID, ElemID;
} MD5_Mesh_t;

// MD5 model structure
typedef struct
{
	int num_joints;
	MD5_Joint_t *baseSkel;	// binding pose skeleton

	int num_meshes;
	MD5_Mesh_t *meshes;		// meshes in model
} MD5_Model_t;

// Animation data
typedef struct
{
	int frameRate;				// frame rate for the animation

	int num_frames;				// number of frames in animation
	int num_joints;				// number of joints in animation (must match number of joints for model)
	MD5_Joint_t **skelFrames;	// frames of joints for the animation

	MD5_BBox_t *bboxes;			// bounding boxes for each frame
} MD5_Anim_t;

int LoadMD5(MD5_Model_t *Model, const char *filename);
void FreeMD5(MD5_Model_t *Model);
void PrepareMesh(MD5_Mesh_t *mesh, const MD5_Joint_t *skeleton, float *vertexArray);
int LoadAnim(MD5_Anim_t *anim, const char *filename);
void FreeAnim(MD5_Anim_t *anim);
void InterpolateSkeletons(const MD5_Anim_t *Anim, const MD5_Joint_t *skelA, const MD5_Joint_t *skelB, float interp, MD5_Joint_t *out);

#endif
