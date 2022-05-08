#ifndef __MD5_GL_H__
#define __MD5_GL_H__

typedef struct
{
	MD5_Model_t Model;

	int32_t frame, nextframe;
	float frameTime;
	MD5_Anim_t Anim;

	uint32_t SkelSSBO;
	MD5_Joint_t *Skel;
} Model_t;

void DrawModelMD5(MD5_Model_t *Model);
void BuildVBOMD5(MD5_Model_t *Model);
void UpdateAnimation(Model_t *Model, float dt);
int32_t LoadMD5Model(const char *Filename, Model_t *Model);
void DestroyMD5Model(Model_t *Model);

#endif
