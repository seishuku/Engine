#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "opencl.h"

#ifdef WIN32
#include <Windows.h>
#define DBGPRINTF(...) { char buf[512]; snprintf(buf, sizeof(buf), __VA_ARGS__); OutputDebugString(buf); }
#else
#define DBGPRINTF(...) { fprintf(stderr, __VA_ARGS__); }
#endif

#define CL_ERR_TO_STR(err) case err: return #err

char const *clGetErrorString(cl_int const err)
{
	switch(err)
	{
		CL_ERR_TO_STR(CL_SUCCESS);
		CL_ERR_TO_STR(CL_DEVICE_NOT_FOUND);
		CL_ERR_TO_STR(CL_DEVICE_NOT_AVAILABLE);
		CL_ERR_TO_STR(CL_COMPILER_NOT_AVAILABLE);
		CL_ERR_TO_STR(CL_MEM_OBJECT_ALLOCATION_FAILURE);
		CL_ERR_TO_STR(CL_OUT_OF_RESOURCES);
		CL_ERR_TO_STR(CL_OUT_OF_HOST_MEMORY);
		CL_ERR_TO_STR(CL_PROFILING_INFO_NOT_AVAILABLE);
		CL_ERR_TO_STR(CL_MEM_COPY_OVERLAP);
		CL_ERR_TO_STR(CL_IMAGE_FORMAT_MISMATCH);
		CL_ERR_TO_STR(CL_IMAGE_FORMAT_NOT_SUPPORTED);
		CL_ERR_TO_STR(CL_BUILD_PROGRAM_FAILURE);
		CL_ERR_TO_STR(CL_MAP_FAILURE);
		CL_ERR_TO_STR(CL_MISALIGNED_SUB_BUFFER_OFFSET);
		CL_ERR_TO_STR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
		CL_ERR_TO_STR(CL_COMPILE_PROGRAM_FAILURE);
		CL_ERR_TO_STR(CL_LINKER_NOT_AVAILABLE);
		CL_ERR_TO_STR(CL_LINK_PROGRAM_FAILURE);
		CL_ERR_TO_STR(CL_DEVICE_PARTITION_FAILED);
		CL_ERR_TO_STR(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
		CL_ERR_TO_STR(CL_INVALID_VALUE);
		CL_ERR_TO_STR(CL_INVALID_DEVICE_TYPE);
		CL_ERR_TO_STR(CL_INVALID_PLATFORM);
		CL_ERR_TO_STR(CL_INVALID_DEVICE);
		CL_ERR_TO_STR(CL_INVALID_CONTEXT);
		CL_ERR_TO_STR(CL_INVALID_QUEUE_PROPERTIES);
		CL_ERR_TO_STR(CL_INVALID_COMMAND_QUEUE);
		CL_ERR_TO_STR(CL_INVALID_HOST_PTR);
		CL_ERR_TO_STR(CL_INVALID_MEM_OBJECT);
		CL_ERR_TO_STR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
		CL_ERR_TO_STR(CL_INVALID_IMAGE_SIZE);
		CL_ERR_TO_STR(CL_INVALID_SAMPLER);
		CL_ERR_TO_STR(CL_INVALID_BINARY);
		CL_ERR_TO_STR(CL_INVALID_BUILD_OPTIONS);
		CL_ERR_TO_STR(CL_INVALID_PROGRAM);
		CL_ERR_TO_STR(CL_INVALID_PROGRAM_EXECUTABLE);
		CL_ERR_TO_STR(CL_INVALID_KERNEL_NAME);
		CL_ERR_TO_STR(CL_INVALID_KERNEL_DEFINITION);
		CL_ERR_TO_STR(CL_INVALID_KERNEL);
		CL_ERR_TO_STR(CL_INVALID_ARG_INDEX);
		CL_ERR_TO_STR(CL_INVALID_ARG_VALUE);
		CL_ERR_TO_STR(CL_INVALID_ARG_SIZE);
		CL_ERR_TO_STR(CL_INVALID_KERNEL_ARGS);
		CL_ERR_TO_STR(CL_INVALID_WORK_DIMENSION);
		CL_ERR_TO_STR(CL_INVALID_WORK_GROUP_SIZE);
		CL_ERR_TO_STR(CL_INVALID_WORK_ITEM_SIZE);
		CL_ERR_TO_STR(CL_INVALID_GLOBAL_OFFSET);
		CL_ERR_TO_STR(CL_INVALID_EVENT_WAIT_LIST);
		CL_ERR_TO_STR(CL_INVALID_EVENT);
		CL_ERR_TO_STR(CL_INVALID_OPERATION);
		CL_ERR_TO_STR(CL_INVALID_GL_OBJECT);
		CL_ERR_TO_STR(CL_INVALID_BUFFER_SIZE);
		CL_ERR_TO_STR(CL_INVALID_MIP_LEVEL);
		CL_ERR_TO_STR(CL_INVALID_GLOBAL_WORK_SIZE);
		CL_ERR_TO_STR(CL_INVALID_PROPERTY);
		CL_ERR_TO_STR(CL_INVALID_IMAGE_DESCRIPTOR);
		CL_ERR_TO_STR(CL_INVALID_COMPILER_OPTIONS);
		CL_ERR_TO_STR(CL_INVALID_LINKER_OPTIONS);
		CL_ERR_TO_STR(CL_INVALID_DEVICE_PARTITION_COUNT);
		CL_ERR_TO_STR(CL_INVALID_PIPE_SIZE);
		CL_ERR_TO_STR(CL_INVALID_DEVICE_QUEUE);

		default:
			return "UNKNOWN ERROR CODE";
	}
}

cl_program OpenCL_LoadProgam(OpenCLContext_t *Context, const char *Filename)
{
	FILE *Stream=NULL;
	cl_int Error=0;

	if((Stream=fopen(Filename, "rb"))==NULL)
		return 0;

	fseek(Stream, 0, SEEK_END);
	size_t Length=ftell(Stream);
	fseek(Stream, 0, SEEK_SET);

	uint8_t *Buffer=(uint8_t *)malloc(Length+1);

	if(Buffer==NULL)
		return 0;

	fread(Buffer, 1, Length, Stream);
	Buffer[Length]='\0';

	fclose(Stream);

	cl_program Program=clCreateProgramWithSource(Context->Context, 1, &Buffer, &Length, &Error);

	DBGPRINTF("%s - %s\n", Filename, clGetErrorString(Error));
	
	clBuildProgram(Program, 1, Context->Devices, NULL, NULL, NULL);

	clGetProgramBuildInfo(Program, Context->Devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &Length);

	cl_char *build_log=(cl_char *)malloc((Length+1));

	// Second call to get the log
	clGetProgramBuildInfo(Program, Context->Devices[0], CL_PROGRAM_BUILD_LOG, Length, build_log, NULL);
	
	build_log[Length]='\0';

	DBGPRINTF("%s - %s\n", Filename, build_log);

	free(build_log);

	free(Buffer);

	return Program;
}

int OpenCL_Init(OpenCLContext_t *Context)
{
	cl_uint numPlatforms=0;
	cl_platform_id Platform=NULL;

	if(clGetPlatformIDs(0, NULL, &numPlatforms)!=CL_SUCCESS)
	{
		DBGPRINTF("Error: Getting platforms!\n");
		return 0;
	}

	if(numPlatforms>0)
	{
		cl_platform_id *Platforms=(cl_platform_id *)malloc(sizeof(cl_platform_id)*numPlatforms);

		clGetPlatformIDs(numPlatforms, Platforms, NULL);

		Platform=Platforms[0];

		free(Platforms);
	}

	if(clGetDeviceIDs(Platform, CL_DEVICE_TYPE_GPU, 0, NULL, &Context->NumDevices)==CL_DEVICE_NOT_FOUND)
		return 0;

	if(Context->NumDevices==0)
	{
		DBGPRINTF("No GPU device available.\n\tUsing CPU as default device.");

		if(clGetDeviceIDs(Platform, CL_DEVICE_TYPE_CPU, 0, NULL, &Context->NumDevices)==CL_DEVICE_NOT_FOUND)
			return 0;

		Context->Devices=(cl_device_id *)malloc(Context->NumDevices*sizeof(cl_device_id));
		clGetDeviceIDs(Platform, CL_DEVICE_TYPE_CPU, Context->NumDevices, Context->Devices, NULL);
	}
	else
	{
		Context->Devices=(cl_device_id *)malloc(Context->NumDevices*sizeof(cl_device_id));
		clGetDeviceIDs(Platform, CL_DEVICE_TYPE_GPU, Context->NumDevices, Context->Devices, NULL);
	}

	Context->Context=clCreateContext(NULL, 1, Context->Devices, NULL, NULL, NULL);
	Context->CommandQueue=clCreateCommandQueueWithProperties(Context->Context, Context->Devices[0], 0, NULL);

	return 1;
}

void OpenCL_Destroy(OpenCLContext_t *Context)
{
	clReleaseCommandQueue(Context->CommandQueue);
	clReleaseContext(Context->Context);
}