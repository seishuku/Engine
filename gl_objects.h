#ifndef __GL_OBJECTS_H__
#define __GL_OBJECTS_H__

typedef enum
{
	TEXTURE_HELLKNIGHT_BASE,
	TEXTURE_HELLKNIGHT_SPECULAR,
	TEXTURE_HELLKNIGHT_NORMAL,

	TEXTURE_FATTY_BASE,
	TEXTURE_FATTY_SPECULAR,
	TEXTURE_FATTY_NORMAL,

	TEXTURE_PINKY_BASE,
	TEXTURE_PINKY_SPECULAR,
	TEXTURE_PINKY_NORMAL,

	TEXTURE_TILE_BASE,
	TEXTURE_TILE_SPECULAR,
	TEXTURE_TILE_NORMAL,

	TEXTURE_3DVOLUME,

	GLSL_CUBE_SHADER,
	GLSL_CUBE_MVINV,
	GLSL_CUBE_MV,
	GLSL_CUBE_PROJ,
	GLSL_CUBE_LOCAL,

	GLSL_LIGHT_SHADER,
	GLSL_LIGHT_MVINV,
	GLSL_LIGHT_MV,
	GLSL_LIGHT_PROJ,
	GLSL_LIGHT_LOCAL,
	GLSL_LIGHT_LIGHT0_POS,
	GLSL_LIGHT_LIGHT0_KD,
	GLSL_LIGHT_LIGHT1_POS,
	GLSL_LIGHT_LIGHT1_KD,
	GLSL_LIGHT_LIGHT2_POS,
	GLSL_LIGHT_LIGHT2_KD,
	GLSL_LIGHT_LIGHT3_POS,
	GLSL_LIGHT_LIGHT3_KD,
	GLSL_LIGHT_LIGHT4_POS,
	GLSL_LIGHT_LIGHT4_KD,

	GLSL_GENERIC_SHADER,

	GLSL_DISTANCE_SHADER,
	GLSL_DISTANCE_PROJ,
	GLSL_DISTANCE_MV,
	GLSL_DISTANCE_LOCAL,
	GLSL_DISTANCE_LIGHTPOS,

	TEXTURE_DISTANCE0,
	BUFFER_DISTANCE0,

	GLSL_MD5_GENVERTS_COMPUTE,

	NUM_OBJECTS
} ObjectNames;

extern unsigned int Objects[NUM_OBJECTS];

#endif
