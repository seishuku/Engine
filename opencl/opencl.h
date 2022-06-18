#ifndef __OPENCL_H__
#define __OPENCL_H__

#define CL_TARGET_OPENCL_VERSION 220

#include <cl/cl.h>

typedef struct
{
	cl_uint NumDevices;
	cl_device_id *Devices;
	cl_context Context;
	cl_command_queue CommandQueue;
} OpenCLContext_t;

char const *clGetErrorString(cl_int const err);
cl_program OpenCL_LoadProgam(OpenCLContext_t *Context, const char *Filename);
int OpenCL_Init(OpenCLContext_t *Context);
void OpenCL_Destroy(OpenCLContext_t *Context);

#endif
