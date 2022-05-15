#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../opengl/opengl.h"
#include "../system/system.h"
#include "../math/math.h"
#include "../image/image.h"

void _MakeNormalMap(Image_t *Image)
{
	int32_t x, y, xx, yy;
	int32_t Channels=Image->Depth>>3;
	uint16_t *Buffer=NULL;
	const float OneOver255=1.0f/255.0f;
	float KernelX[9]={ 1.0f, 0.0f, -1.0f, 2.0f, 0.0f, -2.0f, 1.0f, 0.0f, -1.0f };
	float KernelY[9]={ -1.0f, -2.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 1.0f };

	if(!((Image->Depth==32)||(Image->Depth==24)||(Image->Depth==8)))
		return;

	Buffer=(uint16_t *)malloc(sizeof(uint16_t)*Image->Width*Image->Height*4);

	if(Buffer==NULL)
		return;

	for(y=0;y<Image->Height;y++)
	{
		for(x=0;x<Image->Width;x++)
		{
			float n[3]={ 0.0f, 0.0f, 1.0f }, mag;

			for(yy=0;yy<3;yy++)
			{
				int32_t oy=min(Image->Height-1, y+yy);

				for(xx=0;xx<3;xx++)
				{
					int32_t ox=min(Image->Width-1, x+xx);

					n[0]+=KernelX[yy*3+xx]*(float)(Image->Data[Channels*(oy*Image->Width+ox)]*OneOver255);
					n[1]+=KernelY[yy*3+xx]*(float)(Image->Data[Channels*(oy*Image->Width+ox)]*OneOver255);
				}
			}

			mag=1.0f/sqrtf(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
			n[0]*=mag;
			n[1]*=mag;
			n[2]*=mag;

			Buffer[4*(y*Image->Width+x)+0]=(uint16_t)(65535.0f*(0.5f*n[0]+0.5f));
			Buffer[4*(y*Image->Width+x)+1]=(uint16_t)(65535.0f*(0.5f*n[1]+0.5f));
			Buffer[4*(y*Image->Width+x)+2]=(uint16_t)(65535.0f*(0.5f*n[2]+0.5f));

			if(Channels==4)
				Buffer[4*(y*Image->Width+x)+3]=(uint16_t)(Image->Data[4*(y*Image->Width+x)+3]<<8);
			else
				Buffer[4*(y*Image->Width+x)+3]=65535;
		}
	}

	Image->Depth=64;

	FREE(Image->Data);
	Image->Data=(uint8_t *)Buffer;
}

void _Normalize(Image_t *Image)
{
	int32_t i, Channels=Image->Depth>>3;
	uint16_t *Buffer=NULL;
	const float OneOver255=1.0f/255.0f;

	if(!((Image->Depth==32)||(Image->Depth==24)))
		return;

	Buffer=(uint16_t *)malloc(sizeof(uint16_t)*Image->Width*Image->Height*4);

	if(Buffer==NULL)
		return;

	for(i=0;i<Image->Width*Image->Height;i++)
	{
		float n[3], mag;

		n[0]=2.0f*((float)Image->Data[Channels*i+2]*OneOver255)-1.0f;
		n[1]=2.0f*((float)Image->Data[Channels*i+1]*OneOver255)-1.0f;
		n[2]=2.0f*((float)Image->Data[Channels*i+0]*OneOver255)-1.0f;

		mag=1.0f/sqrtf(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
		n[0]*=mag;
		n[1]*=mag;
		n[2]*=mag;

		Buffer[4*i+0]=(uint16_t)(65535.0f*(0.5f*n[0]+0.5f));
		Buffer[4*i+1]=(uint16_t)(65535.0f*(0.5f*n[1]+0.5f));
		Buffer[4*i+2]=(uint16_t)(65535.0f*(0.5f*n[2]+0.5f));

		if(Channels==4)
			Buffer[4*i+3]=(uint16_t)(Image->Data[4*i+3]<<8);
		else
			Buffer[4*i+3]=65535;
	}

	Image->Depth=64;

	FREE(Image->Data);
	Image->Data=(uint8_t *)Buffer;
}

void _RGBE2Float(Image_t *Image)
{
	int32_t i;
	float *Buffer=NULL;

	Buffer=(float *)malloc(sizeof(float)*Image->Width*Image->Height*3);

	if(Buffer==NULL)
		return;

	for(i=0;i<Image->Width*Image->Height;i++)
	{
		uint8_t *rgbe=&Image->Data[4*i];
		float *rgb=&Buffer[3*i];

		if(rgbe[3])
		{
			float f=1.0f;
			int32_t i, e=rgbe[3]-(128+8);

			if(e>0)
			{
				for(i=0;i<e;i++)
					f*=2.0f;
			}
			else
			{
				for(i=0;i<-e;i++)
					f/=2.0f;
			}

			rgb[0]=((float)rgbe[0]+0.5f)*f;
			rgb[1]=((float)rgbe[1]+0.5f)*f;
			rgb[2]=((float)rgbe[2]+0.5f)*f;
		}
		else
			rgb[0]=rgb[1]=rgb[2]=0.0f;
	}

	Image->Depth=96;

	FREE(Image->Data);
	Image->Data=(uint8_t *)Buffer;
}

void _Resample(Image_t *Src, Image_t *Dst)
{
	float fx, fy, hx, hy, lx, ly, sx, sy;
	float xPercent, yPercent, Percent;
	float Total[4], Sum;
	int32_t x, y, iy, ix, Index;

	if(Dst->Data==NULL)
		return;

	sx=(float)Src->Width/Dst->Width;
	sy=(float)Src->Height/Dst->Height;

	for(y=0;y<Dst->Height;y++)
	{
		if(Src->Height>Dst->Height)
		{
			fy=((float)y+0.5f)*sy;
			hy=fy+(sy*0.5f);
			ly=fy-(sy*0.5f);
		}
		else
		{
			fy=(float)y*sy;
			hy=fy+0.5f;
			ly=fy-0.5f;
		}

		for(x=0;x<Dst->Width;x++)
		{
			if(Src->Width>Dst->Width)
			{
				fx=((float)x+0.5f)*sx;
				hx=fx+(sx*0.5f);
				lx=fx-(sx*0.5f);
			}
			else
			{
				fx=(float)x*sx;
				hx=fx+0.5f;
				lx=fx-0.5f;
			}

			Total[0]=Total[1]=Total[2]=Total[3]=Sum=0.0f;

			fy=ly;
			iy=(int32_t)fy;

			while(fy<hy)
			{
				if(hy<iy+1)
					yPercent=hy-fy;
				else
					yPercent=(iy+1)-fy;

				fx=lx;
				ix=(int32_t)fx;

				while(fx<hx)
				{
					if(hx<ix+1)
						xPercent=hx-fx;
					else
						xPercent=(ix+1)-fx;

					Percent=xPercent*xPercent;
					Sum+=Percent;

					Index=min(Src->Height-1, iy)*Src->Width+min(Src->Width-1, ix);

					switch(Src->Depth)
					{
						case 128:
							Total[0]+=((float *)Src->Data)[4*Index+0]*Percent;
							Total[1]+=((float *)Src->Data)[4*Index+1]*Percent;
							Total[2]+=((float *)Src->Data)[4*Index+2]*Percent;
							Total[3]+=((float *)Src->Data)[4*Index+3]*Percent;
							break;

						case 96:
							Total[0]+=((float *)Src->Data)[3*Index+0]*Percent;
							Total[1]+=((float *)Src->Data)[3*Index+1]*Percent;
							Total[2]+=((float *)Src->Data)[3*Index+2]*Percent;
							break;

						case 64:
							Total[0]+=((uint16_t *)Src->Data)[4*Index+0]*Percent;
							Total[1]+=((uint16_t *)Src->Data)[4*Index+1]*Percent;
							Total[2]+=((uint16_t *)Src->Data)[4*Index+2]*Percent;
							Total[3]+=((uint16_t *)Src->Data)[4*Index+3]*Percent;
							break;

						case 48:
							Total[0]+=((uint16_t *)Src->Data)[3*Index+0]*Percent;
							Total[1]+=((uint16_t *)Src->Data)[3*Index+1]*Percent;
							Total[2]+=((uint16_t *)Src->Data)[3*Index+2]*Percent;
							break;

						case 32:
							Total[0]+=(float)Src->Data[4*Index+0]*Percent;
							Total[1]+=(float)Src->Data[4*Index+1]*Percent;
							Total[2]+=(float)Src->Data[4*Index+2]*Percent;
							Total[3]+=(float)Src->Data[4*Index+3]*Percent;
							break;

						case 24:
							Total[0]+=Src->Data[3*Index+0]*Percent;
							Total[1]+=Src->Data[3*Index+1]*Percent;
							Total[2]+=Src->Data[3*Index+2]*Percent;
							break;

						case 16:
							Total[0]+=((((uint16_t *)Src->Data)[Index]>>0x0)&0x1F)*Percent;
							Total[1]+=((((uint16_t *)Src->Data)[Index]>>0x5)&0x1F)*Percent;
							Total[2]+=((((uint16_t *)Src->Data)[Index]>>0xA)&0x1F)*Percent;
							break;

						case 8:
							Total[0]+=Src->Data[Index]*Percent;
					}

					ix++;
					fx=(float)ix;
				}

				iy++;
				fy=(float)iy;
			}

			Index=y*Dst->Width+x;
			Sum=1.0f/Sum;

			switch(Dst->Depth)
			{
				case 128:
					((float *)Dst->Data)[4*Index+0]=(float)(Total[0]*Sum);
					((float *)Dst->Data)[4*Index+1]=(float)(Total[1]*Sum);
					((float *)Dst->Data)[4*Index+2]=(float)(Total[2]*Sum);
					((float *)Dst->Data)[4*Index+3]=(float)(Total[3]*Sum);
					break;

				case 96:
					((float *)Dst->Data)[3*Index+0]=(float)(Total[0]*Sum);
					((float *)Dst->Data)[3*Index+1]=(float)(Total[1]*Sum);
					((float *)Dst->Data)[3*Index+2]=(float)(Total[2]*Sum);
					break;

				case 64:
					((uint16_t *)Dst->Data)[4*Index+0]=(uint16_t)(Total[0]*Sum);
					((uint16_t *)Dst->Data)[4*Index+1]=(uint16_t)(Total[1]*Sum);
					((uint16_t *)Dst->Data)[4*Index+2]=(uint16_t)(Total[2]*Sum);
					((uint16_t *)Dst->Data)[4*Index+3]=(uint16_t)(Total[3]*Sum);
					break;

				case 48:
					((uint16_t *)Dst->Data)[3*Index+0]=(uint16_t)(Total[0]*Sum);
					((uint16_t *)Dst->Data)[3*Index+1]=(uint16_t)(Total[1]*Sum);
					((uint16_t *)Dst->Data)[3*Index+2]=(uint16_t)(Total[2]*Sum);
					break;

				case 32:
					((uint8_t *)Dst->Data)[4*Index+0]=(uint8_t)(Total[0]*Sum);
					((uint8_t *)Dst->Data)[4*Index+1]=(uint8_t)(Total[1]*Sum);
					((uint8_t *)Dst->Data)[4*Index+2]=(uint8_t)(Total[2]*Sum);
					((uint8_t *)Dst->Data)[4*Index+3]=(uint8_t)(Total[3]*Sum);
					break;

				case 24:
					((uint8_t *)Dst->Data)[3*Index+0]=(uint8_t)(Total[0]*Sum);
					((uint8_t *)Dst->Data)[3*Index+1]=(uint8_t)(Total[1]*Sum);
					((uint8_t *)Dst->Data)[3*Index+2]=(uint8_t)(Total[2]*Sum);
					break;

				case 16:
					((uint16_t *)Dst->Data)[Index]=((uint16_t)((Total[0]*Sum))&0x1F)<<0x0|((uint16_t)(Total[1]*Sum)&0x1F)<<0x5|((uint16_t)(Total[2]*Sum)&0x1F)<<0xA;
					break;

				case 8:
					((uint8_t *)Dst->Data)[Index]=(uint8_t)(Total[0]*Sum);
					break;
			}
		}
	}
}

void _BuildMipmaps(Image_t *Image, uint32_t Target)
{
	int32_t i=0, levels;
	uint32_t MaxSize;
	Image_t Dst;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxSize);

	Dst.Depth=Image->Depth;
	Dst.Width=min(MaxSize, NextPower2(Image->Width));
	Dst.Height=min(MaxSize, NextPower2(Image->Height));

	if(Dst.Height>Dst.Width)
		levels=ComputeLog(Dst.Height);
	else
		levels=ComputeLog(Dst.Width);

	while(i<=levels)
	{
		Dst.Data=(uint8_t *)malloc(Dst.Width*Dst.Height*(Dst.Depth>>3));

		_Resample(Image, &Dst);

		switch(Dst.Depth)
		{
			case 128:
				glTexImage2D(Target, i, GL_RGBA16, Dst.Width, Dst.Height, 0, GL_RGBA, GL_FLOAT, Dst.Data);
				break;

			case 96:
				glTexImage2D(Target, i, GL_RGB16, Dst.Width, Dst.Height, 0, GL_RGB, GL_FLOAT, Dst.Data);
				break;

			case 64:
				glTexImage2D(Target, i, GL_RGBA16, Dst.Width, Dst.Height, 0, GL_RGBA, GL_UNSIGNED_SHORT, Dst.Data);
				break;

			case 48:
				glTexImage2D(Target, i, GL_RGB16, Dst.Width, Dst.Height, 0, GL_RGB, GL_UNSIGNED_SHORT, Dst.Data);
				break;

			case 32:
				glTexImage2D(Target, i, GL_RGBA8, Dst.Width, Dst.Height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, Dst.Data);
				break;

			case 24:
				glTexImage2D(Target, i, GL_RGB8, Dst.Width, Dst.Height, 0, GL_BGR, GL_UNSIGNED_BYTE, Dst.Data);
				break;

			case 16:
				glTexImage2D(Target, i, GL_RGB5, Dst.Width, Dst.Height, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Dst.Data);
				break;

			case 8:
				glTexImage2D(Target, i, GL_R8, Dst.Width, Dst.Height, 0, GL_RED, GL_UNSIGNED_BYTE, Dst.Data);
				break;
		}

		FREE(Dst.Data);

		Dst.Width=(Dst.Width>1)?Dst.Width>>1:Dst.Width;
		Dst.Height=(Dst.Height>1)?Dst.Height>>1:Dst.Height;
		i++;
	}
}

void _GetPixelBilinear(Image_t *Image, float x, float y, uint8_t *Out)
{
	int32_t ix=(int32_t)x, iy=(int32_t)y;
	int32_t ox=ix+1, oy=iy+1;
	float fx=x-ix, fy=y-iy;
	float w00, w01, w10, w11;

	if(ox>=Image->Width)
		ox=Image->Width-1;

	if(oy>=Image->Height)
		oy=Image->Height-1;

	if(fx<0.0f)
		ix=ox=0;

	if(fy<0.0f)
		iy=oy=0;

	w11=fx*fy;
	w00=1.0f-fx-fy+w11;
	w10=fx-w11;
	w01=fy-w11;

	switch(Image->Depth)
	{
		case 128:
			((float *)Out)[0]=((float *)Image->Data)[4*(iy*Image->Width+ix)+0]*w00+((float *)Image->Data)[4*(iy*Image->Width+ox)+0]*w10+((float *)Image->Data)[4*(oy*Image->Width+ix)+0]*w01+((float *)Image->Data)[4*(oy*Image->Width+ox)+0]*w11;
			((float *)Out)[1]=((float *)Image->Data)[4*(iy*Image->Width+ix)+1]*w00+((float *)Image->Data)[4*(iy*Image->Width+ox)+1]*w10+((float *)Image->Data)[4*(oy*Image->Width+ix)+1]*w01+((float *)Image->Data)[4*(oy*Image->Width+ox)+1]*w11;
			((float *)Out)[2]=((float *)Image->Data)[4*(iy*Image->Width+ix)+2]*w00+((float *)Image->Data)[4*(iy*Image->Width+ox)+2]*w10+((float *)Image->Data)[4*(oy*Image->Width+ix)+2]*w01+((float *)Image->Data)[4*(oy*Image->Width+ox)+2]*w11;
			((float *)Out)[3]=((float *)Image->Data)[4*(iy*Image->Width+ix)+3]*w00+((float *)Image->Data)[4*(iy*Image->Width+ox)+3]*w10+((float *)Image->Data)[4*(oy*Image->Width+ix)+3]*w01+((float *)Image->Data)[4*(oy*Image->Width+ox)+3]*w11;
			break;

		case 96:
			((float *)Out)[0]=((float *)Image->Data)[3*(iy*Image->Width+ix)+0]*w00+((float *)Image->Data)[3*(iy*Image->Width+ox)+0]*w10+((float *)Image->Data)[3*(oy*Image->Width+ix)+0]*w01+((float *)Image->Data)[3*(oy*Image->Width+ox)+0]*w11;
			((float *)Out)[1]=((float *)Image->Data)[3*(iy*Image->Width+ix)+1]*w00+((float *)Image->Data)[3*(iy*Image->Width+ox)+1]*w10+((float *)Image->Data)[3*(oy*Image->Width+ix)+1]*w01+((float *)Image->Data)[3*(oy*Image->Width+ox)+1]*w11;
			((float *)Out)[2]=((float *)Image->Data)[3*(iy*Image->Width+ix)+2]*w00+((float *)Image->Data)[3*(iy*Image->Width+ox)+2]*w10+((float *)Image->Data)[3*(oy*Image->Width+ix)+2]*w01+((float *)Image->Data)[3*(oy*Image->Width+ox)+2]*w11;
			break;

		case 64:
			((uint16_t *)Out)[0]=(uint16_t)(((uint16_t *)Image->Data)[4*(iy*Image->Width+ix)+0]*w00+((uint16_t *)Image->Data)[4*(iy*Image->Width+ox)+0]*w10+((uint16_t *)Image->Data)[4*(oy*Image->Width+ix)+0]*w01+((uint16_t *)Image->Data)[4*(oy*Image->Width+ox)+0]*w11);
			((uint16_t *)Out)[1]=(uint16_t)(((uint16_t *)Image->Data)[4*(iy*Image->Width+ix)+1]*w00+((uint16_t *)Image->Data)[4*(iy*Image->Width+ox)+1]*w10+((uint16_t *)Image->Data)[4*(oy*Image->Width+ix)+1]*w01+((uint16_t *)Image->Data)[4*(oy*Image->Width+ox)+1]*w11);
			((uint16_t *)Out)[2]=(uint16_t)(((uint16_t *)Image->Data)[4*(iy*Image->Width+ix)+2]*w00+((uint16_t *)Image->Data)[4*(iy*Image->Width+ox)+2]*w10+((uint16_t *)Image->Data)[4*(oy*Image->Width+ix)+2]*w01+((uint16_t *)Image->Data)[4*(oy*Image->Width+ox)+2]*w11);
			((uint16_t *)Out)[3]=(uint16_t)(((uint16_t *)Image->Data)[4*(iy*Image->Width+ix)+3]*w00+((uint16_t *)Image->Data)[4*(iy*Image->Width+ox)+3]*w10+((uint16_t *)Image->Data)[4*(oy*Image->Width+ix)+3]*w01+((uint16_t *)Image->Data)[4*(oy*Image->Width+ox)+3]*w11);
			break;

		case 48:
			((uint16_t *)Out)[0]=(uint16_t)(((uint16_t *)Image->Data)[3*(iy*Image->Width+ix)+0]*w00+((uint16_t *)Image->Data)[3*(iy*Image->Width+ox)+0]*w10+((uint16_t *)Image->Data)[3*(oy*Image->Width+ix)+0]*w01+((uint16_t *)Image->Data)[3*(oy*Image->Width+ox)+0]*w11);
			((uint16_t *)Out)[1]=(uint16_t)(((uint16_t *)Image->Data)[3*(iy*Image->Width+ix)+1]*w00+((uint16_t *)Image->Data)[3*(iy*Image->Width+ox)+1]*w10+((uint16_t *)Image->Data)[3*(oy*Image->Width+ix)+1]*w01+((uint16_t *)Image->Data)[3*(oy*Image->Width+ox)+1]*w11);
			((uint16_t *)Out)[2]=(uint16_t)(((uint16_t *)Image->Data)[3*(iy*Image->Width+ix)+2]*w00+((uint16_t *)Image->Data)[3*(iy*Image->Width+ox)+2]*w10+((uint16_t *)Image->Data)[3*(oy*Image->Width+ix)+2]*w01+((uint16_t *)Image->Data)[3*(oy*Image->Width+ox)+2]*w11);
			break;

		case 32:
			Out[0]=(uint8_t)(Image->Data[4*(iy*Image->Width+ix)+0]*w00+Image->Data[4*(iy*Image->Width+ox)+0]*w10+Image->Data[4*(oy*Image->Width+ix)+0]*w01+Image->Data[4*(oy*Image->Width+ox)+0]*w11);
			Out[1]=(uint8_t)(Image->Data[4*(iy*Image->Width+ix)+1]*w00+Image->Data[4*(iy*Image->Width+ox)+1]*w10+Image->Data[4*(oy*Image->Width+ix)+1]*w01+Image->Data[4*(oy*Image->Width+ox)+1]*w11);
			Out[2]=(uint8_t)(Image->Data[4*(iy*Image->Width+ix)+2]*w00+Image->Data[4*(iy*Image->Width+ox)+2]*w10+Image->Data[4*(oy*Image->Width+ix)+2]*w01+Image->Data[4*(oy*Image->Width+ox)+2]*w11);
			Out[3]=(uint8_t)(Image->Data[4*(iy*Image->Width+ix)+3]*w00+Image->Data[4*(iy*Image->Width+ox)+3]*w10+Image->Data[4*(oy*Image->Width+ix)+3]*w01+Image->Data[4*(oy*Image->Width+ox)+3]*w11);
			break;

		case 24:
			Out[0]=(uint8_t)(Image->Data[3*(iy*Image->Width+ix)+0]*w00+Image->Data[3*(iy*Image->Width+ox)+0]*w10+Image->Data[3*(oy*Image->Width+ix)+0]*w01+Image->Data[3*(oy*Image->Width+ox)+0]*w11);
			Out[1]=(uint8_t)(Image->Data[3*(iy*Image->Width+ix)+1]*w00+Image->Data[3*(iy*Image->Width+ox)+1]*w10+Image->Data[3*(oy*Image->Width+ix)+1]*w01+Image->Data[3*(oy*Image->Width+ox)+1]*w11);
			Out[2]=(uint8_t)(Image->Data[3*(iy*Image->Width+ix)+2]*w00+Image->Data[3*(iy*Image->Width+ox)+2]*w10+Image->Data[3*(oy*Image->Width+ix)+2]*w01+Image->Data[3*(oy*Image->Width+ox)+2]*w11);
			break;

		case 16:
		{
			uint16_t p0=((uint16_t *)Image->Data)[iy*Image->Width+ix];
			uint16_t p1=((uint16_t *)Image->Data)[iy*Image->Width+ox];
			uint16_t p2=((uint16_t *)Image->Data)[oy*Image->Width+ix];
			uint16_t p3=((uint16_t *)Image->Data)[oy*Image->Width+ox];

			*((uint16_t *)Out) =(uint16_t)(((p0>>0x0)&0x1F)*w00+((p1>>0x0)&0x1F)*w10+((p2>>0x0)&0x1F)*w01+((p3>>0x0)&0x1F)*w11)<<0x0;
			*((uint16_t *)Out)|=(uint16_t)(((p0>>0x5)&0x1F)*w00+((p1>>0x5)&0x1F)*w10+((p2>>0x5)&0x1F)*w01+((p3>>0x5)&0x1F)*w11)<<0x5;
			*((uint16_t *)Out)|=(uint16_t)(((p0>>0xA)&0x1F)*w00+((p1>>0xA)&0x1F)*w10+((p2>>0xA)&0x1F)*w01+((p3>>0xA)&0x1F)*w11)<<0xA;
			break;
		}

		case 8:
			*Out=(uint8_t)(Image->Data[iy*Image->Width+ix]*w00+Image->Data[iy*Image->Width+ox]*w10+Image->Data[oy*Image->Width+ix]*w01+Image->Data[oy*Image->Width+ox]*w11);
			break;
	}
}

void _GetUVAngularMap(float xyz[3], float *uv)
{
	float phi=-(float)acos(xyz[2]), theta=(float)atan2(xyz[1], xyz[0]);

	uv[0]=0.5f*((phi/3.1415926f)*(float)cos(theta))+0.5f;
	uv[1]=0.5f*((phi/3.1415926f)*(float)sin(theta))+0.5f;
}

void _GetXYZFace(float uv[2], float *xyz, int32_t face)
{
	float mag;

	switch(face)
	{
		// +X
		case 0:
			xyz[0]=1.0f;
			xyz[1]=(uv[1]-0.5f)*2.0f;
			xyz[2]=(0.5f-uv[0])*2.0f;
			break;

		// -X
		case 1:
			xyz[0]=-1.0f;
			xyz[1]=(uv[1]-0.5f)*2.0f;
			xyz[2]=(uv[0]-0.5f)*2.0f;
			break;

		// +Y
		case 2:
			xyz[0]=(uv[0]-0.5f)*2.0f;
			xyz[1]=-1.0f;
			xyz[2]=(uv[1]-0.5f)*2.0f;
			break;

		// -Y
		case 3:
			xyz[0]=(uv[0]-0.5f)*2.0f;
			xyz[1]=1.0f;
			xyz[2]=(0.5f-uv[1])*2.0f;
			break;

		// +Z
		case 4:
			xyz[0]=(uv[0]-0.5f)*2.0f;
			xyz[1]=(uv[1]-0.5f)*2.0f;
			xyz[2]=1.0f;
			break;

		// -Z
		case 5:
			xyz[0]=(0.5f-uv[0])*2.0f;
			xyz[1]=(uv[1]-0.5f)*2.0f;
			xyz[2]=-1.0f;
			break;
	}

	mag=sqrtf(xyz[0]*xyz[0]+xyz[1]*xyz[1]+xyz[2]*xyz[2]);

	if(mag)
	{
		mag=1.0f/mag;
		xyz[0]*=mag;
		xyz[1]*=mag;
		xyz[2]*=mag;
	}
}

void _AngularMapFace(Image_t *In, int32_t Face, int32_t Mipmap)
{
	Image_t Out;
	int32_t x, y;
	uint32_t Internal, External, Type;

	switch(In->Depth)
	{
		case 128:
			Internal=GL_RGBA16;
			External=GL_RGBA;
			Type=GL_FLOAT;
			break;

		case 96:
			Internal=GL_RGB16;
			External=GL_RGB;
			Type=GL_FLOAT;
			break;

		case 64:
			Internal=GL_RGBA16;
			External=GL_BGRA;
			Type=GL_UNSIGNED_SHORT;
			break;

		case 48:
			Internal=GL_RGB16;
			External=GL_BGR;
			Type=GL_UNSIGNED_SHORT;
			break;

		case 32:
			Internal=GL_RGBA8;
			External=GL_BGRA;
			Type=GL_UNSIGNED_INT_8_8_8_8_REV;
			break;

		case 24:
			Internal=GL_RGB8;
			External=GL_BGR;
			Type=GL_UNSIGNED_BYTE;
			break;

 		case 16:
			Internal=GL_RGB5;
			External=GL_BGRA;
			Type=GL_UNSIGNED_SHORT_1_5_5_5_REV;
			break;

		case 8:
			Internal=GL_R8;
			External=GL_RED;
			Type=GL_UNSIGNED_BYTE;
			break;

		default:
			return;
	}

	Out.Depth=In->Depth;
	Out.Width=NextPower2(In->Width>>1);
	Out.Height=NextPower2(In->Height>>1);
	Out.Data=(uint8_t *)malloc(Out.Width*Out.Height*(Out.Depth>>3));

	for(y=0;y<Out.Height;y++)
	{
		float fy=(float)y/(Out.Height-1);

		for(x=0;x<Out.Width;x++)
		{
			float fx=(float)x/(Out.Width-1);
			float uv[2]={ fx, fy }, xyz[3];

			_GetXYZFace(uv, xyz, Face);
			_GetUVAngularMap(xyz, uv);

			_GetPixelBilinear(In, uv[0]*In->Width, uv[1]*In->Height, &Out.Data[(Out.Depth>>3)*(y*Out.Width+x)]);
		}
	}

	if(Mipmap)
		_BuildMipmaps(&Out, GL_TEXTURE_CUBE_MAP_POSITIVE_X+Face);
	else
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+Face, 0, Internal, Out.Width, Out.Height, 0, External, Type, Out.Data);

	FREE(Out.Data);
}

uint32_t Image_Upload(const char *Filename, uint32_t Flags)
{
	uint32_t TextureID=0;
	uint32_t Target=GL_TEXTURE_2D;
	Image_t Image;
	char *Extension=strrchr(Filename, '.');

	if(Extension!=NULL)
	{
		if(!strcmp(Extension, ".dds"))
		{
			if(!DDS_Load(Filename, &Image))
				return 0;
		}
		else
		if(!strcmp(Extension, ".tga"))
		{
			if(!TGA_Load(Filename, &Image))
				return 0;
		}
		else
		if(!strcmp(Extension, ".qoi"))
		{
			if(!QOI_Load(Filename, &Image))
				return 0;
		}
		else
			return 0;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if(Flags&IMAGE_CUBEMAP_ANGULAR)
		Target=GL_TEXTURE_CUBE_MAP;

	if(Flags&IMAGE_RECTANGLE)
	{
		Target=GL_TEXTURE_RECTANGLE;

		if(Flags&IMAGE_AUTOMIPMAP)
			Flags^=IMAGE_AUTOMIPMAP;

		if(Flags&IMAGE_MIPMAP)
			Flags^=IMAGE_MIPMAP;

		if(Flags&IMAGE_REPEAT)
		{
			Flags^=IMAGE_REPEAT;
			Flags|=IMAGE_CLAMP;
		}
	}

	glGenTextures(1, &TextureID);
	glBindTexture(Target, TextureID);

//	if(Flags&IMAGE_AUTOMIPMAP)
//	{
//		if(GL_SGIS_generate_mipmap_Flag)
//			glTexParameteri(Target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
//	}

	glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if(Flags&IMAGE_NEAREST)
	{
		glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if(Flags&IMAGE_MIPMAP)
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		else
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	if(Flags&IMAGE_BILINEAR)
	{
		glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if(Flags&IMAGE_MIPMAP)
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		else
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	if(Flags&IMAGE_TRILINEAR)
	{
		glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if(Flags&IMAGE_MIPMAP)
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		else
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	if(Flags&IMAGE_CLAMP_U)
		glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	if(Flags&IMAGE_CLAMP_V)
		glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if(Flags&IMAGE_REPEAT_U)
		glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_REPEAT);

	if(Flags&IMAGE_REPEAT_V)
		glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(Flags&IMAGE_RGBE)
		_RGBE2Float(&Image);

	if(Flags&IMAGE_NORMALMAP)
		_MakeNormalMap(&Image);

	if(Flags&IMAGE_NORMALIZE)
		_Normalize(&Image);

	if(Flags&IMAGE_CUBEMAP_ANGULAR)
	{
		_AngularMapFace(&Image, 0, Flags&IMAGE_MIPMAP);
		_AngularMapFace(&Image, 1, Flags&IMAGE_MIPMAP);
		_AngularMapFace(&Image, 2, Flags&IMAGE_MIPMAP);
		_AngularMapFace(&Image, 3, Flags&IMAGE_MIPMAP);
		_AngularMapFace(&Image, 4, Flags&IMAGE_MIPMAP);
		_AngularMapFace(&Image, 5, Flags&IMAGE_MIPMAP);

		FREE(Image.Data);

		return TextureID;
	}

	switch(Image.Depth)
	{
		case IMAGE_DXT5:
			if(Flags&IMAGE_MIPMAP)
			{
//				if(GL_SGIS_generate_mipmap_Flag)
//					glTexParameteri(Target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			}

			glCompressedTexImage2D(Target, 0, GL_COMPRESSED_RGBA, Image.Width, Image.Height, 0, ((Image.Width+3)/4)*((Image.Height+3)/4)*16, Image.Data);
			break;

		case IMAGE_DXT3:
			if(Flags&IMAGE_MIPMAP)
			{
//				if(GL_SGIS_generate_mipmap_Flag)
//					glTexParameteri(Target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			}

			glCompressedTexImage2D(Target, 0, GL_COMPRESSED_RGBA, Image.Width, Image.Height, 0, ((Image.Width+3)/4)*((Image.Height+3)/4)*16, Image.Data);
			break;

		case IMAGE_DXT1:
			if(Flags&IMAGE_MIPMAP)
			{
//				if(GL_SGIS_generate_mipmap_Flag)
//					glTexParameteri(Target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			}

			glCompressedTexImage2D(Target, 0, GL_COMPRESSED_RGBA, Image.Width, Image.Height, 0, ((Image.Width+3)/4)*((Image.Height+3)/4)*8, Image.Data);
			break;

		case 128:
			if(Flags&IMAGE_MIPMAP)
				_BuildMipmaps(&Image, Target);
			else
				glTexImage2D(Target, 0, GL_RGBA16, Image.Width, Image.Height, 0, GL_RGBA, GL_FLOAT, Image.Data);
			break;

		case 96:
			if(Flags&IMAGE_MIPMAP)
				_BuildMipmaps(&Image, Target);
			else
				glTexImage2D(Target, 0, GL_RGB16, Image.Width, Image.Height, 0, GL_RGB, GL_FLOAT, Image.Data);
			break;

		case 64:
			if(Flags&IMAGE_MIPMAP)
				_BuildMipmaps(&Image, Target);
			else
				glTexImage2D(Target, 0, GL_RGBA16, Image.Width, Image.Height, 0, GL_RGBA, GL_UNSIGNED_SHORT, Image.Data);
			break;

		case 48:
			if(Flags&IMAGE_MIPMAP)
				_BuildMipmaps(&Image, Target);
			else
				glTexImage2D(Target, 0, GL_RGB16, Image.Width, Image.Height, 0, GL_RGB, GL_UNSIGNED_SHORT, Image.Data);
			break;

		case 32:
			if(Flags&IMAGE_MIPMAP)
				_BuildMipmaps(&Image, Target);
			else
				glTexImage2D(Target, 0, GL_RGBA8, Image.Width, Image.Height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, Image.Data);
			break;

		case 24:
			if(Flags&IMAGE_MIPMAP)
				_BuildMipmaps(&Image, Target);
			else
				glTexImage2D(Target, 0, GL_RGB8, Image.Width, Image.Height, 0, GL_BGR, GL_UNSIGNED_BYTE, Image.Data);
			break;

		case 16:
			if(Flags&IMAGE_MIPMAP)
				_BuildMipmaps(&Image, Target);
			else
				glTexImage2D(Target, 0, GL_RGB5, Image.Width, Image.Height, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Image.Data);
			break;

		case 8:
			if(Flags&IMAGE_MIPMAP)
				_BuildMipmaps(&Image, Target);
			else
				glTexImage2D(Target, 0, GL_R8, Image.Width, Image.Height, 0, GL_RED, GL_UNSIGNED_BYTE, Image.Data);
			break;

		default:
			FREE(Image.Data);
			return 0;
	}

	FREE(Image.Data);

	return TextureID;
}
