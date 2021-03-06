#ifndef __IMAGE_H__
#define __IMAGE_H__

// Image flags
#define IMAGE_NONE										0x00000000
#define IMAGE_AUTOMIPMAP								0x00000001
#define IMAGE_MIPMAP									0x00000002
#define IMAGE_NEAREST									0x00000004
#define IMAGE_BILINEAR									0x00000008
#define IMAGE_TRILINEAR									0x00000010
#define IMAGE_NORMALMAP									0x00000020
#define IMAGE_NORMALIZE									0x00000040
#define IMAGE_RGBE										0x00000080
#define IMAGE_CUBEMAP_ANGULAR							0x00000100
#define IMAGE_RECTANGLE									0x00000200
#define IMAGE_CLAMP_U									0x00000400
#define IMAGE_CLAMP_V									0x00000800
#define IMAGE_CLAMP_W									0x00001000
#define IMAGE_CLAMP										(IMAGE_CLAMP_U|IMAGE_CLAMP_V|IMAGE_CLAMP_W)
#define IMAGE_REPEAT_U									0x00004000
#define IMAGE_REPEAT_V									0x00008000
#define IMAGE_REPEAT_W									0x00010000
#define IMAGE_REPEAT									(IMAGE_REPEAT_U|IMAGE_REPEAT_V|IMAGE_REPEAT_W)

typedef struct
{
	uint32_t Width, Height;
	uint32_t Depth;
	uint8_t *Data;
} Image_t;

bool TGA_Load(const char *Filename, Image_t *Image);
bool TGA_Write(const char *filename, Image_t *Image, bool rle);
bool QOI_Load(const char *Filename, Image_t *Image);
bool QOI_Write(const char *Filename, Image_t *Image);

uint32_t Image_Upload(const char *Filename, uint32_t Flags);

#endif
