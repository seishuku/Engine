#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../system/system.h"
#include "../opengl/opengl.h"
#include "../opencl/opencl.h"
#include "../gl_objects.h"
#include "fluid3d.h"

#define IX(x, y, z, sx, sy, sz) (min(sx-1, max(1, (x)))+min(sy-1, max(1, (y)))*sx+min(sz-1, max(1, (z)))*sx*sx)

bool Fluid3D_Init(Fluid3D_t *Fluid, int width, int height, int depth, float diffusion, float viscosity)
{
	Fluid->w=width;
	Fluid->h=height;
	Fluid->d=depth;
	Fluid->diff=diffusion;
	Fluid->visc=viscosity;

#ifdef FLUID3D_CPU
	Fluid->d0=calloc(width*height*depth, sizeof(float));

	if(Fluid->d0==NULL)
		return false;

	Fluid->d1=calloc(width*height*depth, sizeof(float));

	if(Fluid->d1==NULL)
		return false;

	Fluid->v0=calloc(width*height*depth, sizeof(float));

	if(Fluid->v0==NULL)
		return false;

	Fluid->v1=calloc(width*height*depth, sizeof(float));

	if(Fluid->v1==NULL)
		return false;

	Fluid->u0=calloc(width*height*depth, sizeof(float));

	if(Fluid->u0==NULL)
		return false;

	Fluid->u1=calloc(width*height*depth, sizeof(float));

	if(Fluid->u1==NULL)
		return false;

	Fluid->w0=calloc(width*height*depth, sizeof(float));

	if(Fluid->w0==NULL)
		return false;

	Fluid->w1=calloc(width*height*depth, sizeof(float));

	if(Fluid->w1==NULL)
		return false;
#else
	if(!OpenCL_Init(&Fluid->Context))
		return false;
	else
	{
		cl_int Error=0;

		Fluid->Program=OpenCL_LoadProgam(&Fluid->Context, "./fluid/fluid3d.cl");

		Fluid->SetDensityVelocity=clCreateKernel(Fluid->Program, "set_density_velocity", &Error);
		DBGPRINTF("clCreateKernel set_density_velocity %s\n", clGetErrorString(Error));

		Fluid->AddDensityVelocity=clCreateKernel(Fluid->Program, "add_density_velocity", &Error);
		DBGPRINTF("clCreateKernel add_density_velocity %s\n", clGetErrorString(Error));

		Fluid->Diffuse=clCreateKernel(Fluid->Program, "diffuse", &Error);
		DBGPRINTF("clCreateKernel diffuse %s\n", clGetErrorString(Error));

		Fluid->Advect=clCreateKernel(Fluid->Program, "advect", &Error);
		DBGPRINTF("clCreateKernel advect %s\n", clGetErrorString(Error));

		Fluid->Project1=clCreateKernel(Fluid->Program, "project1", &Error);
		DBGPRINTF("clCreateKernel project1 %s\n", clGetErrorString(Error));

		Fluid->Project2=clCreateKernel(Fluid->Program, "project2", &Error);
		DBGPRINTF("clCreateKernel project2 %s\n", clGetErrorString(Error));

		Fluid->Project3=clCreateKernel(Fluid->Program, "project3", &Error);
		DBGPRINTF("clCreateKernel project3 %s\n", clGetErrorString(Error));

		glGenTextures(1, &Objects[TEXTURE_FLUID]);
		glBindTexture(GL_TEXTURE_3D, Objects[TEXTURE_FLUID]);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, Fluid->w, Fluid->h, Fluid->d);

		Fluid->den=clCreateFromGLTexture(Fluid->Context.Context, CL_MEM_WRITE_ONLY, GL_TEXTURE_3D, 0, Objects[TEXTURE_FLUID], &Error);
		DBGPRINTF("clCreateFromGLTexture den %s\n", clGetErrorString(Error));

		Fluid->d0=clCreateBuffer(Fluid->Context.Context, CL_MEM_READ_WRITE, sizeof(float)*Fluid->w*Fluid->h*Fluid->d, NULL, &Error);
		DBGPRINTF("clCreateBuffer d0 %s\n", clGetErrorString(Error));

		Fluid->d1=clCreateBuffer(Fluid->Context.Context, CL_MEM_READ_WRITE, sizeof(float)*Fluid->w*Fluid->h*Fluid->d, NULL, &Error);
		DBGPRINTF("clCreateBuffer d1 %s\n", clGetErrorString(Error));

		Fluid->u0=clCreateBuffer(Fluid->Context.Context, CL_MEM_READ_WRITE, sizeof(float)*Fluid->w*Fluid->h*Fluid->d, NULL, &Error);
		DBGPRINTF("clCreateBuffer u0 %s\n", clGetErrorString(Error));

		Fluid->u1=clCreateBuffer(Fluid->Context.Context, CL_MEM_READ_WRITE, sizeof(float)*Fluid->w*Fluid->h*Fluid->d, NULL, &Error);
		DBGPRINTF("clCreateBuffer u1 %s\n", clGetErrorString(Error));

		Fluid->v0=clCreateBuffer(Fluid->Context.Context, CL_MEM_READ_WRITE, sizeof(float)*Fluid->w*Fluid->h*Fluid->d, NULL, &Error);
		DBGPRINTF("clCreateBuffer v0 %s\n", clGetErrorString(Error));

		Fluid->v1=clCreateBuffer(Fluid->Context.Context, CL_MEM_READ_WRITE, sizeof(float)*Fluid->w*Fluid->h*Fluid->d, NULL, &Error);
		DBGPRINTF("clCreateBuffer v1 %s\n", clGetErrorString(Error));

		Fluid->w0=clCreateBuffer(Fluid->Context.Context, CL_MEM_READ_WRITE, sizeof(float)*Fluid->w*Fluid->h*Fluid->d, NULL, &Error);
		DBGPRINTF("clCreateBuffer w0 %s\n", clGetErrorString(Error));

		Fluid->w1=clCreateBuffer(Fluid->Context.Context, CL_MEM_READ_WRITE, sizeof(float)*Fluid->w*Fluid->h*Fluid->d, NULL, &Error);
		DBGPRINTF("clCreateBuffer w1 %s\n", clGetErrorString(Error));
	}
#endif

	return true;
}

void Fluid3D_Destroy(Fluid3D_t *Fluid)
{
#ifdef FLUID3D_CPU
	free(Fluid->d0);
	free(Fluid->d1);
	free(Fluid->v0);
	free(Fluid->v1);
	free(Fluid->u0);
	free(Fluid->u1);
	free(Fluid->w0);
	free(Fluid->w1);
#else
	clReleaseMemObject(Fluid->den);
	clReleaseMemObject(Fluid->d0);
	clReleaseMemObject(Fluid->d1);
	clReleaseMemObject(Fluid->u0);
	clReleaseMemObject(Fluid->u1);
	clReleaseMemObject(Fluid->v0);
	clReleaseMemObject(Fluid->v1);
	clReleaseMemObject(Fluid->w0);
	clReleaseMemObject(Fluid->w1);

	clReleaseKernel(Fluid->SetDensityVelocity);
	clReleaseKernel(Fluid->AddDensityVelocity);
	clReleaseKernel(Fluid->Diffuse);
	clReleaseKernel(Fluid->Advect);
	clReleaseKernel(Fluid->Project1);
	clReleaseKernel(Fluid->Project2);
	clReleaseKernel(Fluid->Project3);

	clReleaseProgram(Fluid->Program);

	OpenCL_Destroy(&Fluid->Context);
#endif
}

#ifdef FLUID3D_CPU
static void lin_solve(Fluid3D_t *Fluid, float *x, float *x0, float a, float c, int iter)
{
	float cRecip=1.0f/c;

	for(int k=1;k<Fluid->d-1;k++)
	{
		for(int j=1;j<Fluid->h-1;j++)
		{
			for(int i=1;i<Fluid->w-1;i++)
			{
				for(int l=0;l<iter;l++)
				{
					x[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)]=
					(
						x0[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)]+
						a*
						(
							x[IX(i+1, j, k, Fluid->w, Fluid->h, Fluid->d)]+
							x[IX(i-1, j, k, Fluid->w, Fluid->h, Fluid->d)]+
							x[IX(i, j+1, k, Fluid->w, Fluid->h, Fluid->d)]+
							x[IX(i, j-1, k, Fluid->w, Fluid->h, Fluid->d)]+
							x[IX(i, j, k+1, Fluid->w, Fluid->h, Fluid->d)]+
							x[IX(i, j, k-1, Fluid->w, Fluid->h, Fluid->d)]
						)
					)*cRecip;
				}
			}
		}
	}
}

static void diffuse(Fluid3D_t *Fluid, float *x, float *x0, float diff, float dt, int iter)
{
	float a=dt*diff*(Fluid->w-2)*(Fluid->d-2);

	lin_solve(Fluid, x, x0, a, 1.0f+6.0f*a, iter);
}

static void advect(Fluid3D_t *Fluid, float *d, float *d0, float *velocX, float *velocY, float *velocZ, float dt)
{
	float dtx=dt*(Fluid->w-2);
	float dty=dt*(Fluid->h-2);
	float dtz=dt*(Fluid->d-2);

	for(int k=1;k<Fluid->d-1;k++)
	{
		for(int j=1;j<Fluid->h-1;j++)
		{
			for(int i=1;i<Fluid->w-1;i++)
			{
				float x=(float)i-dtx*velocX[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)];
				float y=(float)j-dty*velocY[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)];
				float z=(float)k-dtz*velocZ[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)];

				if(x<0.5f)
					x=0.5f;

				if(x>(float)Fluid->w+0.5f)
					x=(float)Fluid->w+0.5f;

				int i0=(int)floorf(x);
				int i1=i0+1;

				if(y<0.5f)
					y=0.5f;

				if(y>(float)Fluid->h+0.5f)
					y=(float)Fluid->h+0.5f;

				int j0=(int)floorf(y);
				int j1=j0+1;

				if(z<0.5f)
					z=0.5f;

				if(z>(float)Fluid->d+0.5f)
					z=(float)Fluid->d+0.5f;

				int k0=(int)floorf(z);
				int k1=k0+1;

				float s1=x-i0;
				float s0=1.0f-s1;
				float t1=y-j0;
				float t0=1.0f-t1;
				float r1=z-k0;
				float r0=1.0f-r1;

				d[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)]=
				s0*
				(
					t0*
					(
						r0*d0[IX(i0, j0, k0, Fluid->w, Fluid->h, Fluid->d)]+
						r1*d0[IX(i0, j0, k1, Fluid->w, Fluid->h, Fluid->d)]
					)+
					t1*
					(
						r0*d0[IX(i0, j1, k0, Fluid->w, Fluid->h, Fluid->d)]+
						r1*d0[IX(i0, j1, k1, Fluid->w, Fluid->h, Fluid->d)]
					)
				)+
				s1*
				(
					t0*
					(
						r0*d0[IX(i1, j0, k0, Fluid->w, Fluid->h, Fluid->d)]+
						r1*d0[IX(i1, j0, k1, Fluid->w, Fluid->h, Fluid->d)]
					)+
					t1*
					(
						r0*d0[IX(i1, j1, k0, Fluid->w, Fluid->h, Fluid->d)]+
						r1*d0[IX(i1, j1, k1, Fluid->w, Fluid->h, Fluid->d)]
					)
				);
			}
		}
	}
}

static void project(Fluid3D_t *Fluid, float *velocX, float *velocY, float *velocZ, float *p, float *div, int iter)
{
	for(int k=1;k<Fluid->d-1;k++)
	{
		for(int j=1;j<Fluid->h-1;j++)
		{
			for(int i=1;i<Fluid->w-1;i++)
			{
				div[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)]=
				-0.5f*
				(
					velocX[IX(i+1, j, k, Fluid->w, Fluid->h, Fluid->d)]-
					velocX[IX(i-1, j, k, Fluid->w, Fluid->h, Fluid->d)]+
					velocY[IX(i, j+1, k, Fluid->w, Fluid->h, Fluid->d)]-
					velocY[IX(i, j-1, k, Fluid->w, Fluid->h, Fluid->d)]+
					velocZ[IX(i, j, k+1, Fluid->w, Fluid->h, Fluid->d)]-
					velocZ[IX(i, j, k-1, Fluid->w, Fluid->h, Fluid->d)]
				)/Fluid->w;

				p[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)]=0.0f;
			}
		}
	}

	lin_solve(Fluid, p, div, 1, 6, iter);

	for(int k=1;k<Fluid->d-1;k++)
	{
		for(int j=1;j<Fluid->h-1;j++)
		{
			for(int i=1;i<Fluid->w-1;i++)
			{
				velocX[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)]-=0.5f*(p[IX(i+1, j, k, Fluid->w, Fluid->h, Fluid->d)]-p[IX(i-1, j, k, Fluid->w, Fluid->h, Fluid->d)])*Fluid->w;
				velocY[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)]-=0.5f*(p[IX(i, j+1, k, Fluid->w, Fluid->h, Fluid->d)]-p[IX(i, j-1, k, Fluid->w, Fluid->h, Fluid->d)])*Fluid->h;
				velocZ[IX(i, j, k, Fluid->w, Fluid->h, Fluid->d)]-=0.5f*(p[IX(i, j, k+1, Fluid->w, Fluid->h, Fluid->d)]-p[IX(i, j, k-1, Fluid->w, Fluid->h, Fluid->d)])*Fluid->d;
			}
		}
	}
}
#endif

void Fluid3D_Step(Fluid3D_t *Fluid, float dt)
{
#ifdef FLUID3D_CPU
	diffuse(Fluid, Fluid->u1, Fluid->u0, Fluid->visc, dt, 4);
	diffuse(Fluid, Fluid->v1, Fluid->v0, Fluid->visc, dt, 4);
	diffuse(Fluid, Fluid->w1, Fluid->w0, Fluid->visc, dt, 4);

	project(Fluid, Fluid->u1, Fluid->v1, Fluid->w1, Fluid->u0, Fluid->v0, 4);

	advect(Fluid, Fluid->u0, Fluid->u1, Fluid->u1, Fluid->v1, Fluid->w1, dt);
	advect(Fluid, Fluid->v0, Fluid->v1, Fluid->u1, Fluid->v1, Fluid->w1, dt);
	advect(Fluid, Fluid->w0, Fluid->w1, Fluid->u1, Fluid->v1, Fluid->w1, dt);

	project(Fluid, Fluid->u0, Fluid->v0, Fluid->w0, Fluid->u1, Fluid->v1, 4);

	diffuse(Fluid, Fluid->d1, Fluid->d0, Fluid->diff, dt, 4);

	advect(Fluid, Fluid->d0, Fluid->d1, Fluid->u0, Fluid->v0, Fluid->w0, dt);
#else
	int intr=4;
	size_t size[3]={ Fluid->w, Fluid->h, Fluid->d };

	clSetKernelArg(Fluid->Diffuse, 2, sizeof(cl_float), &Fluid->visc);
	clSetKernelArg(Fluid->Diffuse, 3, sizeof(cl_float), &dt);
	clSetKernelArg(Fluid->Diffuse, 4, sizeof(cl_int), &intr);

	clSetKernelArg(Fluid->Diffuse, 0, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Diffuse, 1, sizeof(cl_mem), &Fluid->u0);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Diffuse, 3, NULL, size, NULL, 0, NULL, NULL);

	clSetKernelArg(Fluid->Diffuse, 0, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Diffuse, 1, sizeof(cl_mem), &Fluid->v0);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Diffuse, 3, NULL, size, NULL, 0, NULL, NULL);

	clSetKernelArg(Fluid->Diffuse, 0, sizeof(cl_mem), &Fluid->w1);
	clSetKernelArg(Fluid->Diffuse, 1, sizeof(cl_mem), &Fluid->w0);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Diffuse, 3, NULL, size, NULL, 0, NULL, NULL);

	clSetKernelArg(Fluid->Project1, 0, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Project1, 1, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Project1, 2, sizeof(cl_mem), &Fluid->w1);
	clSetKernelArg(Fluid->Project1, 3, sizeof(cl_mem), &Fluid->u0);
	clSetKernelArg(Fluid->Project1, 4, sizeof(cl_mem), &Fluid->v0);
	clSetKernelArg(Fluid->Project1, 5, sizeof(cl_int), &intr);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Project1, 3, NULL, size, NULL, 0, NULL, NULL);
	clSetKernelArg(Fluid->Project2, 0, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Project2, 1, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Project2, 2, sizeof(cl_mem), &Fluid->w1);
	clSetKernelArg(Fluid->Project2, 3, sizeof(cl_mem), &Fluid->u0);
	clSetKernelArg(Fluid->Project2, 4, sizeof(cl_mem), &Fluid->v0);
	clSetKernelArg(Fluid->Project2, 5, sizeof(cl_int), &intr);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Project2, 3, NULL, size, NULL, 0, NULL, NULL);
	clSetKernelArg(Fluid->Project3, 0, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Project3, 1, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Project3, 2, sizeof(cl_mem), &Fluid->w1);
	clSetKernelArg(Fluid->Project3, 3, sizeof(cl_mem), &Fluid->u0);
	clSetKernelArg(Fluid->Project3, 4, sizeof(cl_mem), &Fluid->v0);
	clSetKernelArg(Fluid->Project3, 5, sizeof(cl_int), &intr);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Project3, 3, NULL, size, NULL, 0, NULL, NULL);

	clSetKernelArg(Fluid->Advect, 0, sizeof(cl_mem), &Fluid->u0);
	clSetKernelArg(Fluid->Advect, 1, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Advect, 2, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Advect, 3, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Advect, 4, sizeof(cl_mem), &Fluid->w1);
	clSetKernelArg(Fluid->Advect, 5, sizeof(cl_float), &dt);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Advect, 3, NULL, size, NULL, 0, NULL, NULL);
	clSetKernelArg(Fluid->Advect, 0, sizeof(cl_mem), &Fluid->v0);
	clSetKernelArg(Fluid->Advect, 1, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Advect, 2, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Advect, 3, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Advect, 4, sizeof(cl_mem), &Fluid->w1);
	clSetKernelArg(Fluid->Advect, 5, sizeof(cl_float), &dt);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Advect, 3, NULL, size, NULL, 0, NULL, NULL);
	clSetKernelArg(Fluid->Advect, 0, sizeof(cl_mem), &Fluid->w0);
	clSetKernelArg(Fluid->Advect, 1, sizeof(cl_mem), &Fluid->w1);
	clSetKernelArg(Fluid->Advect, 2, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Advect, 3, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Advect, 4, sizeof(cl_mem), &Fluid->w1);
	clSetKernelArg(Fluid->Advect, 5, sizeof(cl_float), &dt);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Advect, 3, NULL, size, NULL, 0, NULL, NULL);

	clSetKernelArg(Fluid->Project1, 0, sizeof(cl_mem), &Fluid->u0);
	clSetKernelArg(Fluid->Project1, 1, sizeof(cl_mem), &Fluid->v0);
	clSetKernelArg(Fluid->Project1, 2, sizeof(cl_mem), &Fluid->w0);
	clSetKernelArg(Fluid->Project1, 3, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Project1, 4, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Project1, 5, sizeof(cl_int), &intr);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Project1, 3, NULL, size, NULL, 0, NULL, NULL);
	clSetKernelArg(Fluid->Project2, 0, sizeof(cl_mem), &Fluid->u0);
	clSetKernelArg(Fluid->Project2, 1, sizeof(cl_mem), &Fluid->v0);
	clSetKernelArg(Fluid->Project2, 2, sizeof(cl_mem), &Fluid->w0);
	clSetKernelArg(Fluid->Project2, 3, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Project2, 4, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Project2, 5, sizeof(cl_int), &intr);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Project2, 3, NULL, size, NULL, 0, NULL, NULL);
	clSetKernelArg(Fluid->Project3, 0, sizeof(cl_mem), &Fluid->u0);
	clSetKernelArg(Fluid->Project3, 1, sizeof(cl_mem), &Fluid->v0);
	clSetKernelArg(Fluid->Project3, 2, sizeof(cl_mem), &Fluid->w0);
	clSetKernelArg(Fluid->Project3, 3, sizeof(cl_mem), &Fluid->u1);
	clSetKernelArg(Fluid->Project3, 4, sizeof(cl_mem), &Fluid->v1);
	clSetKernelArg(Fluid->Project3, 5, sizeof(cl_int), &intr);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Project3, 3, NULL, size, NULL, 0, NULL, NULL);

	clSetKernelArg(Fluid->Diffuse, 0, sizeof(cl_mem), &Fluid->d1);
	clSetKernelArg(Fluid->Diffuse, 1, sizeof(cl_mem), &Fluid->d0);
	clSetKernelArg(Fluid->Diffuse, 2, sizeof(cl_float), &Fluid->diff);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Diffuse, 3, NULL, size, NULL, 0, NULL, NULL);

	clSetKernelArg(Fluid->Advect, 0, sizeof(cl_mem), &Fluid->d0);
	clSetKernelArg(Fluid->Advect, 1, sizeof(cl_mem), &Fluid->d1);
	clSetKernelArg(Fluid->Advect, 2, sizeof(cl_mem), &Fluid->u0);
	clSetKernelArg(Fluid->Advect, 3, sizeof(cl_mem), &Fluid->v0);
	clSetKernelArg(Fluid->Advect, 4, sizeof(cl_mem), &Fluid->w0);
	clSetKernelArg(Fluid->Advect, 5, sizeof(cl_float), &dt);
	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->Advect, 3, NULL, size, NULL, 0, NULL, NULL);

	clEnqueueCopyBufferToImage(Fluid->Context.CommandQueue, Fluid->d0, Fluid->den, 0, (size_t[]) { 0, 0, 0 }, (size_t[]) { Fluid->w, Fluid->h, Fluid->d }, 0, NULL, NULL);
#endif
}

float Fluid3D_GetDensity(Fluid3D_t *Fluid, int x, int y, int z)
{
#ifdef FLUID3D_CPU
	return Fluid->d0[IX(x, y, z, Fluid->w, Fluid->h, Fluid->d)];
#else
	return 0.0f;
#endif
}

void Fluid3D_AddDensityVelocity(Fluid3D_t *Fluid, int x, int y, int z, float u, float v, float w, float d)
{
#ifdef FLUID3D_CPU
	Fluid->u0[IX(x, y, z, Fluid->w, Fluid->h, Fluid->d)]+=u;
	Fluid->v0[IX(x, y, z, Fluid->w, Fluid->h, Fluid->d)]+=v;
	Fluid->w0[IX(x, y, z, Fluid->w, Fluid->h, Fluid->d)]+=w;
	Fluid->d0[IX(x, y, z, Fluid->w, Fluid->h, Fluid->d)]+=d;
#else
	size_t size[1]={ 1 };

	clSetKernelArg(Fluid->AddDensityVelocity, 0, sizeof(cl_mem), (void *)&Fluid->u0);
	clSetKernelArg(Fluid->AddDensityVelocity, 1, sizeof(cl_mem), (void *)&Fluid->v0);
	clSetKernelArg(Fluid->AddDensityVelocity, 2, sizeof(cl_mem), (void *)&Fluid->w0);
	clSetKernelArg(Fluid->AddDensityVelocity, 3, sizeof(cl_mem), (void *)&Fluid->d0);
	clSetKernelArg(Fluid->AddDensityVelocity, 4, sizeof(cl_int), (void *)&x);
	clSetKernelArg(Fluid->AddDensityVelocity, 5, sizeof(cl_int), (void *)&y);
	clSetKernelArg(Fluid->AddDensityVelocity, 6, sizeof(cl_int), (void *)&z);
	clSetKernelArg(Fluid->AddDensityVelocity, 7, sizeof(cl_float), (void *)&u);
	clSetKernelArg(Fluid->AddDensityVelocity, 8, sizeof(cl_float), (void *)&v);
	clSetKernelArg(Fluid->AddDensityVelocity, 9, sizeof(cl_float), (void *)&w);
	clSetKernelArg(Fluid->AddDensityVelocity,10, sizeof(cl_float), (void *)&d);
	clSetKernelArg(Fluid->AddDensityVelocity,11, sizeof(cl_int), (void *)&Fluid->w);
	clSetKernelArg(Fluid->AddDensityVelocity,12, sizeof(cl_int), (void *)&Fluid->h);
	clSetKernelArg(Fluid->AddDensityVelocity,13, sizeof(cl_int), (void *)&Fluid->d);

	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->AddDensityVelocity, 1, NULL, size, NULL, 0, NULL, NULL);
#endif
}

void Fluid3D_SetDensityVelocity(Fluid3D_t *Fluid, int x, int y, int z, float u, float v, float w, float d)
{
#ifdef FLUID3D_CPU
	Fluid->u0[IX(x, y, z, Fluid->w, Fluid->h, Fluid->d)]=u;
	Fluid->v0[IX(x, y, z, Fluid->w, Fluid->h, Fluid->d)]=v;
	Fluid->w0[IX(x, y, z, Fluid->w, Fluid->h, Fluid->d)]=w;
	Fluid->d0[IX(x, y, z, Fluid->w, Fluid->h, Fluid->d)]=d;
#else
	size_t size[1]={ 1 };

	clSetKernelArg(Fluid->SetDensityVelocity, 0, sizeof(cl_mem), (void *)&Fluid->u0);
	clSetKernelArg(Fluid->SetDensityVelocity, 1, sizeof(cl_mem), (void *)&Fluid->v0);
	clSetKernelArg(Fluid->SetDensityVelocity, 2, sizeof(cl_mem), (void *)&Fluid->w0);
	clSetKernelArg(Fluid->SetDensityVelocity, 3, sizeof(cl_mem), (void *)&Fluid->d0);
	clSetKernelArg(Fluid->SetDensityVelocity, 4, sizeof(cl_int), (void *)&x);
	clSetKernelArg(Fluid->SetDensityVelocity, 5, sizeof(cl_int), (void *)&y);
	clSetKernelArg(Fluid->SetDensityVelocity, 6, sizeof(cl_int), (void *)&z);
	clSetKernelArg(Fluid->SetDensityVelocity, 7, sizeof(cl_float), (void *)&u);
	clSetKernelArg(Fluid->SetDensityVelocity, 8, sizeof(cl_float), (void *)&v);
	clSetKernelArg(Fluid->SetDensityVelocity, 9, sizeof(cl_float), (void *)&w);
	clSetKernelArg(Fluid->SetDensityVelocity,10, sizeof(cl_float), (void *)&d);
	clSetKernelArg(Fluid->SetDensityVelocity,11, sizeof(cl_int), (void *)&Fluid->w);
	clSetKernelArg(Fluid->SetDensityVelocity,12, sizeof(cl_int), (void *)&Fluid->h);
	clSetKernelArg(Fluid->SetDensityVelocity,13, sizeof(cl_int), (void *)&Fluid->d);

	clEnqueueNDRangeKernel(Fluid->Context.CommandQueue, Fluid->SetDensityVelocity, 1, NULL, size, NULL, 0, NULL, NULL);
#endif
}
