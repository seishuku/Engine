#ifndef __MD5_H__
#define __MD5_H__

// Joint
typedef struct
{
	char name[64];
	int parent;
	float pos[3];
	float orient[4];
} MD5_Joint_t;

// Vertex
typedef struct
{
	float st[2];	// texture coordinates
	int start;		// start weight
	int count;		// weight count
} MD5_Vertex_t;

// Weight
typedef struct
{
	int joint;			// joint for this weight
	float bias;			// vertex weight
	float pad[2];
	float pos[4];		// vertex position (in joint space)
	float tangent[4];	// tangent vector ("")
	float binormal[4];	// binormal vector ("")
	float normal[4];	// normal vector ("")
} MD5_Weight_t;

// Bounding box
typedef struct
{
	float min[3];
	float max[3];
} MD5_BBox_t;

// MD5 mesh
typedef struct
{
	int num_verts;
	MD5_Vertex_t *vertices;

	int num_tris;
	unsigned long *triangles;

	int num_weights;
	MD5_Weight_t *weights;

	char shader[256];

	float *vertexArray;
	unsigned long VAO;
	unsigned long WeightID, VertID;
	unsigned long FinalVertID, ElemID;
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
