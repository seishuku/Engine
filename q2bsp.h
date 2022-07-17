#ifndef __Q2BSP_H__
#define __Q2BSP_H__

typedef struct
{
	uint32_t NumTris;
} Q2BSP_Model_t;

bool LoadQ2BSP(Q2BSP_Model_t *Model, const char *Filename);
void DrawQ2BSP(Q2BSP_Model_t *Model);

#endif
