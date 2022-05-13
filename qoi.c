#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "image.h"

#define QOI_MAGIC			(((uint32_t)'q')<<24|((uint32_t)'o')<<16|((uint32_t)'i')<<8|((uint32_t)'f'))

#define QOI_SRGB			0
#define QOI_LINEAR			1

#define QOI_OP_INDEX		0x00
#define QOI_OP_DIFF			0x40
#define QOI_OP_LUMA			0x80
#define QOI_OP_RUN			0xC0
#define QOI_OP_RGB			0xFE
#define QOI_OP_RGBA			0xFF
#define QOI_OP_MASK			0xC0

#define QOI_HASH(C)	(C[0]*3+C[1]*5+C[2]*7+C[3]*11)

int32_t QOI_Load(const char *Filename, Image_t *Image)
{
	FILE *stream=NULL;
	uint32_t magic;
	uint32_t width;
	uint32_t height;
	uint8_t channels;
	uint8_t colorspace;
	uint8_t b1=0, b2=0;
	uint8_t index[64][4], bytes[4];
	uint32_t run=0;

	if(!(stream=fopen(Filename, "rb")))
		return 0;

	fread(bytes, 1, 4, stream);
	magic=bytes[0]<<24|bytes[1]<<16|bytes[2]<<8|bytes[3];

	fread(bytes, 1, 4, stream);
	width=bytes[0]<<24|bytes[1]<<16|bytes[2]<<8|bytes[3];

	fread(bytes, 1, 4, stream);
	height=bytes[0]<<24|bytes[1]<<16|bytes[2]<<8|bytes[3];

	fread(&channels, 1, 1, stream);
	fread(&colorspace, 1, 1, stream);

	if(width==0||height==0||channels<3||channels>4||colorspace>1||magic!=QOI_MAGIC)
		return 0;

	Image->Width=width;
	Image->Height=height;
	Image->Depth=channels<<3;
	Image->Data=(uint8_t *)malloc(width*height*channels);

	if(!Image->Data)
		return 0;

	memset(index, 0, 64*4);
	bytes[0]=0;
	bytes[1]=0;
	bytes[2]=0;
	bytes[3]=255;

	for(uint32_t i=0;i<width*height*channels;i+=channels)
	{
		if(run>0)
			run--;
		else
		{
			fread(&b1, 1, 1, stream);

			if(b1==QOI_OP_RGB)
				fread(bytes, 1, 3, stream);
			else if(b1==QOI_OP_RGBA)
				fread(bytes, 1, 4, stream);
			else if((b1&QOI_OP_MASK)==QOI_OP_INDEX)
			{
				bytes[0]=index[b1][0];
				bytes[1]=index[b1][1];
				bytes[2]=index[b1][2];
				bytes[3]=index[b1][3];
			}
			else if((b1&QOI_OP_MASK)==QOI_OP_DIFF)
			{
				bytes[0]+=((b1>>4)&0x03)-2;
				bytes[1]+=((b1>>2)&0x03)-2;
				bytes[2]+=((b1>>0)&0x03)-2;
			}
			else if((b1&QOI_OP_MASK)==QOI_OP_LUMA)
			{
				fread(&b2, 1, 1, stream);

				int vg=(b1&0x3f)-32;

				bytes[0]+=vg-8+((b2>>4)&0x0f);
				bytes[1]+=vg;
				bytes[2]+=vg-8+(b2&0x0f);
			}
			else if((b1&QOI_OP_MASK)==QOI_OP_RUN)
				run=(b1&0x3f);

			index[QOI_HASH(bytes)%64][0]=bytes[0];
			index[QOI_HASH(bytes)%64][1]=bytes[1];
			index[QOI_HASH(bytes)%64][2]=bytes[2];
			index[QOI_HASH(bytes)%64][3]=bytes[3];
		}

		Image->Data[i+0]=bytes[0];
		Image->Data[i+1]=bytes[1];
		Image->Data[i+2]=bytes[2];

		if(channels==4)
			Image->Data[i+3]=bytes[3];
	}

	fclose(stream);

	return 1;
}