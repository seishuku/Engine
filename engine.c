#include <windows.h>
#include <hidusage.h>
#include <process.h>
#include <intrin.h>

#pragma intrinsic(__rdtsc)

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "math.h"
#include "opengl.h"
#include "gl_objects.h"
#include "image.h"
#include "font.h"
#include "camera.h"
#include "3ds.h"
#include "3ds_gl.h"
#include "md5.h"
#include "md5_gl.h"
#include "skybox_gl.h"

#ifdef WIN32
#define DBGPRINTF(...) { char buf[512]; snprintf(buf, sizeof(buf), __VA_ARGS__); OutputDebugString(buf); }
#else
#define DBGPRINTF(...) { fprintf(stderr, __VA_ARGS__); }
#endif

#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(x) ((char *)NULL+(x))
#endif

#ifndef FREE
#define FREE(p) { if(p) { free(p); p=NULL; } }
#endif

GLContext_t Context;

char szAppName[]="OpenGL";

int Width=1024, Height=1024;

int Done=0, Key[256];

unsigned __int64 Frequency, StartTime, EndTime, EndFrameTime;
float avgfps=0.0f, fps=0.0f, fTimeStep, fFrameTime, fTime=0.0f;
int Frames=0;

unsigned int Objects[NUM_OBJECTS];

Model3DS_t Level;

Model_t Hellknight;
Model_t Fatty;
Model_t Pinky;

Camera_t Camera;

float Projection[16], ModelView[16], MVP[16];

float Light0_Pos[4]={ 0.0f, 0.0f, 200.0f, 1.0f/256.0f };
float Light0_Kd[4]={ 1.0f, 1.0f, 1.0f, 1.0f };

void Render(void);
int Init(void);
GLuint CreateComputeProgram(char *Filename);
GLuint CreateShaderProgram(char *VertexFilename, char *FragmentFilename);

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void APIENTRY error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_data)
{
	const char *_severity, *_source, *_type;

	if(severity==GL_DEBUG_SEVERITY_HIGH)
		_severity="High";
	else if(severity==GL_DEBUG_SEVERITY_MEDIUM)
		_severity="Medium";
	else if(severity==GL_DEBUG_SEVERITY_LOW)
		_severity="Low";
	else
		return; // Don't care about these, it's mostly performance hints
//		_severity="Unknown";

	if(source==GL_DEBUG_SOURCE_API)
		_source="API";
	else if(source==GL_DEBUG_SOURCE_WINDOW_SYSTEM)
		_source="Window System";
	else if(source==GL_DEBUG_SOURCE_SHADER_COMPILER)
		_source="Shader Compiler";
	else if(source==GL_DEBUG_SOURCE_THIRD_PARTY)
		_source="Third Party";
	else if(source==GL_DEBUG_SOURCE_APPLICATION)
		_source="Application";
	else if(source==GL_DEBUG_SOURCE_OTHER)
		_source="Other";
	else
		_source="Unknown Source";

	if(type==GL_DEBUG_TYPE_ERROR)
		_type="Error";
	else if(type==GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		_type="Depricated Behavior";
	else if(type==GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		_type="Undefined Behavior";
	else if(type==GL_DEBUG_TYPE_PORTABILITY)
		_type="Portability";
	else if(type==GL_DEBUG_TYPE_PERFORMANCE)
		_type="Performance";
	else if(type==GL_DEBUG_TYPE_OTHER)
		_type="Other";
	else
		_type="Unknown";

	DBGPRINTF("OpenGL debug:\n\tID: 0x%0.4X\n\tType: %s\n\tSeverity: %s\n\tSource: %s\n\tMessage: %s\n\n", id, _type, _severity, _source, message);
}

unsigned __int64 rdtsc(void)
{
	return __rdtsc();
}

unsigned __int64 GetFrequency(void)
{
	unsigned __int64 TimeStart, TimeStop, TimeFreq;
	unsigned __int64 StartTicks, StopTicks;
	volatile unsigned __int64 i;

	QueryPerformanceFrequency((LARGE_INTEGER *)&TimeFreq);

	QueryPerformanceCounter((LARGE_INTEGER *)&TimeStart);
	StartTicks=rdtsc();

	for(i=0;i<1000000;i++);

	StopTicks=rdtsc();
	QueryPerformanceCounter((LARGE_INTEGER *)&TimeStop);

	return (StopTicks-StartTicks)*TimeFreq/(TimeStop-TimeStart);
}

#include <crtdbg.h>   //for malloc and free

BYTE _raw_message[64]={ 0, };
POINT _client_center;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	WNDCLASS wc;
	MSG msg={ 0, };
	RECT Rect;

#ifdef _DEBUG
	_CrtMemState sOld;
	_CrtMemState sNew;
	_CrtMemState sDiff;

	_CrtMemCheckpoint(&sOld); //take a snapshot
#endif

	wc.style=CS_VREDRAW|CS_HREDRAW|CS_OWNDC;
	wc.lpfnWndProc=WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground=GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=szAppName;

	RegisterClass(&wc);

	SetRect(&Rect, 0, 0, Width, Height);
	AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, FALSE);

	Context.hWnd=CreateWindow(szAppName, szAppName, WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, Rect.right-Rect.left, Rect.bottom-Rect.top, NULL, NULL, hInstance, NULL);

	ShowWindow(Context.hWnd, SW_SHOW);
	SetForegroundWindow(Context.hWnd);

	DBGPRINTF("Creating OpenGL context...\n");
	if(!CreateContext(&Context, 32, 24, 0, 0, OGL_CORE46|OGL_DOUBLEBUFFER|OGL_SAMPLES4X))
	{
		DBGPRINTF("\t...failed.\n");
		DestroyWindow(Context.hWnd);

		return -1;
	}

	DBGPRINTF("\nOpenGL vendor: %s\n", glGetString(GL_VENDOR));
	DBGPRINTF("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	DBGPRINTF("OpenGL Version: %s\n", glGetString(GL_VERSION));

	DBGPRINTF("Initalizing OpenGL resources...\n");
	if(!Init())
	{
		DBGPRINTF("\t...failed.\n");
		DestroyContext(&Context);
		DestroyWindow(Context.hWnd);

		return -1;
	}

	Frequency=GetFrequency();
	DBGPRINTF("\nCPU freqency: %0.2fGHz\n", (float)Frequency/1000000000);

	glDebugMessageCallback(error_callback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_DEBUG_OUTPUT);

	GetWindowRect(Context.hWnd, &Rect);

	_client_center.x=Rect.right/2-Rect.left/2;
	_client_center.y=Rect.bottom/2-Rect.top/2;

	SetCursorPos(_client_center.x, _client_center.y);

	DBGPRINTF("\nStarting main loop.\n");
	while(!Done)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message==WM_QUIT)
				Done=1;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			StartTime=rdtsc();
			Render();
			EndFrameTime=rdtsc();
			SwapBuffers(Context.hDC);
			EndTime=rdtsc();

			// Total screen time in seconds
			fTimeStep=(float)(EndTime-StartTime)/Frequency;
			// Running time
			fTime+=fTimeStep;

			// Accumulate frames per second
			avgfps+=1.0f/fTimeStep;

			// Average over 100 frames
			if(Frames++>100)
			{
				// Total frame render in miliseconds
				fFrameTime=(float)(EndFrameTime-StartTime)/(Frequency/1000);

				fps=avgfps/Frames;
				avgfps=0.0f;
				Frames=0;
			}
		}
	}

	Free3DS(&Level);

	DestroyMD5Model(&Hellknight);
	DestroyMD5Model(&Fatty);
	DestroyMD5Model(&Pinky);

	DestroyContext(&Context);
	DestroyWindow(Context.hWnd);

#ifdef _DEBUG
	_CrtMemCheckpoint(&sNew); //take a snapshot 
	if(_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
	{
		OutputDebugString("-----------_CrtMemDumpStatistics ---------");
		_CrtMemDumpStatistics(&sDiff);
		OutputDebugString("-----------_CrtMemDumpAllObjectsSince ---------");
		_CrtMemDumpAllObjectsSince(&sOld);
		OutputDebugString("-----------_CrtDumpMemoryLeaks ---------");
		_CrtDumpMemoryLeaks();
	}
#endif

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static POINT old;
	POINT pos, delta;

	switch(uMsg)
	{
		case WM_CREATE:
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
			break;

		case WM_SIZE:
			Width=LOWORD(lParam);
			Height=HIWORD(lParam);
			break;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetCapture(hWnd);
			ShowCursor(FALSE);

			GetCursorPos(&pos);
			old.x=pos.x;
			old.y=pos.y;
			break;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			ShowCursor(TRUE);
			ReleaseCapture();
			break;

		case WM_MOUSEMOVE:
			GetCursorPos(&pos);

			if(!wParam)
			{
				old.x=pos.x;
				old.y=pos.y;
				break;
			}

			delta.x=pos.x-old.x;
			delta.y=old.y-pos.y;

			if(!delta.x&&!delta.y)
				break;

			SetCursorPos(old.x, old.y);

			switch(wParam)
			{
				case MK_LBUTTON:
					Camera.YawVelocity-=(float)delta.x/2000.0f;
					Camera.PitchVelocity+=(float)delta.y/2000.0f;
					break;

				case MK_MBUTTON:
					break;

				case MK_RBUTTON:
					break;
			}
			break;

		case WM_KEYDOWN:
			Key[wParam]=1;

			switch(wParam)
			{
				case 'W':
					Camera.key_w=1;
					break;

				case 'S':
					Camera.key_s=1;
					break;

				case 'A':
					Camera.key_a=1;
					break;

				case 'D':
					Camera.key_d=1;
					break;

				case 'V':
					Camera.key_v=1;
					break;

				case 'C':
					Camera.key_c=1;
					break;

				case 'Q':
					Camera.key_q=1;
					break;

				case 'E':
					Camera.key_e=1;
					break;

				case VK_UP:
					Camera.key_up=1;
					break;

				case VK_DOWN:
					Camera.key_down=1;
					break;

				case VK_LEFT:
					Camera.key_left=1;
					break;

				case VK_RIGHT:
					Camera.key_right=1;
					break;

				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

				default:
					break;
			}
			break;

		case WM_KEYUP:
			Key[wParam]=0;

			switch(wParam)
			{
				case 'W':
					Camera.key_w=0;
					break;

				case 'S':
					Camera.key_s=0;
					break;

				case 'A':
					Camera.key_a=0;
					break;

				case 'D':
					Camera.key_d=0;
					break;

				case 'V':
					Camera.key_v=0;
					break;

				case 'C':
					Camera.key_c=0;
					break;

				case 'Q':
					Camera.key_q=0;
					break;

				case 'E':
					Camera.key_e=0;
					break;

				case VK_UP:
					Camera.key_up=0;
					break;

				case VK_DOWN:
					Camera.key_down=0;
					break;

				case VK_LEFT:
					Camera.key_left=0;
					break;

				case VK_RIGHT:
					Camera.key_right=0;
					break;

				default:
					break;
			}
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Render(void)
{
	UpdateAnimation(&Hellknight, fTimeStep);
	UpdateAnimation(&Fatty, fTimeStep);
	UpdateAnimation(&Pinky, fTimeStep);

	Light0_Pos[0]=sinf(fTime)*150.0f;
	Light0_Pos[2]=cosf(fTime)*150.0f;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Set viewport and calculate a projection matrix (perspective, with infinite z-far plane)
	glViewport(0, 0, Width, Height);
	MatrixIdentity(Projection);
	InfPerspective(90.0f, (float)Width/Height, 0.01f, 0, Projection);

	// Set up model view matrix (translate and rotation)
	MatrixIdentity(ModelView);
	CameraUpdate(&Camera, fTimeStep, ModelView);

	MatrixMult(ModelView, Projection, MVP);

	//glUseProgram(Objects[GLSL_SKYBOX_SHADER]);
	//glUniformMatrix4fv(Objects[GLSL_SKYBOX_MVP], 1, GL_FALSE, MVP);

	//glBindTextureUnit(0, Objects[TEXTURE_HDR_REFLECT]);

	//DrawSkybox();

	// Select the shader program
	glUseProgram(Objects[GLSL_LIGHT_SHADER]);

	// Light parameters
	glUniform4fv(Objects[GLSL_LIGHT_LIGHT0_POS], 1, Light0_Pos);
	glUniform4fv(Objects[GLSL_LIGHT_LIGHT0_KD], 1, Light0_Kd);

	// Inverse model view matrix for "eye" vector
	MatrixInverse(ModelView, ModelView);
	glUniformMatrix4fv(Objects[GLSL_LIGHT_MVINV], 1, GL_FALSE, ModelView);

	// Model view projection matrix for vertex to clipspace transform (vertex shader)
	glUniformMatrix4fv(Objects[GLSL_LIGHT_MVP], 1, GL_FALSE, MVP);

	// Bind correct textures per model
	glBindTextureUnit(3, Objects[TEXTURE_HDR_REFLECT]);

	// Render model
	glBindTextureUnit(0, Objects[TEXTURE_HELLKNIGHT_BASE]);
	glBindTextureUnit(1, Objects[TEXTURE_HELLKNIGHT_SPECULAR]);
	glBindTextureUnit(2, Objects[TEXTURE_HELLKNIGHT_NORMAL]);
	DrawModelMD5(&Hellknight.Model);

	glBindTextureUnit(0, Objects[TEXTURE_FATTY_BASE]);
	glBindTextureUnit(1, Objects[TEXTURE_FATTY_SPECULAR]);
	glBindTextureUnit(2, Objects[TEXTURE_FATTY_NORMAL]);
	DrawModelMD5(&Fatty.Model);

	glBindTextureUnit(0, Objects[TEXTURE_PINKY_BASE]);
	glBindTextureUnit(1, Objects[TEXTURE_PINKY_SPECULAR]);
	glBindTextureUnit(2, Objects[TEXTURE_PINKY_NORMAL]);
	DrawModelMD5(&Pinky.Model);

	glBindTextureUnit(0, Objects[TEXTURE_TILE_BASE]);
	glBindTextureUnit(1, Objects[TEXTURE_TILE_SPECULAR]);
	glBindTextureUnit(2, Objects[TEXTURE_TILE_NORMAL]);
	DrawModel3DS(&Level);

	glActiveTexture(GL_TEXTURE0);

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
		Font_Print(0.0f, 16.0f, "FPS: %0.1f\nFrame time: %0.4fms", fps, fFrameTime);
		Font_Print(0.0f, (float)Height-16.0f, "%f", Camera.PitchVelocity);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

int Init(void)
{
	CameraInit(&Camera,
		(float[3]) { 0.0f, 0.0f, 100.0f },	// Position
		(float[3]) { 0.0f, 0.0f, -1.0f },	// Heading
		(float[3]) { 0.0f, 1.0f, 0.0f }		// Up
	);

	// Load texture images
	Objects[TEXTURE_HELLKNIGHT_BASE]=Image_Upload("hellknight.tga", IMAGE_MIPMAP);
	Objects[TEXTURE_HELLKNIGHT_SPECULAR]=Image_Upload("hellknight_s.tga", IMAGE_MIPMAP);
	Objects[TEXTURE_HELLKNIGHT_NORMAL]=Image_Upload("hellknight_n.tga", IMAGE_MIPMAP|IMAGE_NORMALIZE);

	Objects[TEXTURE_FATTY_BASE]=Image_Upload("fatty.tga", IMAGE_MIPMAP);
	Objects[TEXTURE_FATTY_SPECULAR]=Image_Upload("fatty_s.tga", IMAGE_MIPMAP);
	Objects[TEXTURE_FATTY_NORMAL]=Image_Upload("fatty_n.tga", IMAGE_MIPMAP|IMAGE_NORMALIZE);

	Objects[TEXTURE_PINKY_BASE]=Image_Upload("pinky.tga", IMAGE_MIPMAP);
	Objects[TEXTURE_PINKY_SPECULAR]=Image_Upload("pinky_s.tga", IMAGE_MIPMAP);
	Objects[TEXTURE_PINKY_NORMAL]=Image_Upload("pinky_n.tga", IMAGE_MIPMAP|IMAGE_NORMALIZE);

	Objects[TEXTURE_TILE_BASE]=Image_Upload("tile.tga", IMAGE_MIPMAP);
	Objects[TEXTURE_TILE_SPECULAR]=Image_Upload("tile_s.tga", IMAGE_MIPMAP);
	Objects[TEXTURE_TILE_NORMAL]=Image_Upload("tile_b.tga", IMAGE_MIPMAP|IMAGE_NORMALMAP);

	Objects[TEXTURE_HDR_REFLECT]=Image_Upload("rnl.tga", IMAGE_RGBE|IMAGE_MIPMAP);

	BuildSkyboxVBO();

	if(Load3DS(&Level, "level.3ds"))
		BuildVBO3DS(&Level);
	else
		return 0;

	// Compile/link MD5 skinning compute program
	Objects[GLSL_MD5_GENVERTS_COMPUTE]=CreateComputeProgram("md5_genverts_c.glsl");

	// Load MD5 model meshes
	if(!LoadMD5Model("hellknight", &Hellknight))
		return 0;

	if(!LoadMD5Model("fatty", &Fatty))
		return 0;

	if(!LoadMD5Model("pinky", &Pinky))
		return 0;

	// Load shaders
	Objects[GLSL_GENERIC_SHADER]=CreateShaderProgram("generic_v.glsl", "generic_f.glsl");

	Objects[GLSL_SKYBOX_SHADER]=CreateShaderProgram("skybox_v.glsl", "skybox_f.glsl");
	glUseProgram(Objects[GLSL_SKYBOX_SHADER]);
	Objects[GLSL_SKYBOX_MVP]=glGetUniformLocation(Objects[GLSL_SKYBOX_SHADER], "mvp");
	Objects[GLSL_SKYBOX_TEXTURE]=glGetUniformLocation(Objects[GLSL_SKYBOX_SHADER], "Texture");

	Objects[GLSL_LIGHT_SHADER]=CreateShaderProgram("light_v.glsl", "light_f.glsl");
	glUseProgram(Objects[GLSL_LIGHT_SHADER]);
	Objects[GLSL_LIGHT_MVINV]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "mvinv");
	Objects[GLSL_LIGHT_MVP]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "mvp");
	Objects[GLSL_LIGHT_LIGHT0_POS]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light0_Pos");
	Objects[GLSL_LIGHT_LIGHT0_KD]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light0_Kd");

	// Set OpenGL states
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return 1;
}

int LoadShader(GLuint Shader, char *Filename)
{
	FILE *stream=NULL;
	char *buffer=NULL;
	size_t length;

	if((stream=fopen(Filename, "rb"))==NULL)
		return 0;

	fseek(stream, 0, SEEK_END);
	length=ftell(stream);
	fseek(stream, 0, SEEK_SET);

	buffer=(char *)malloc(length+1);

	if(buffer==NULL)
		return 0;

	fread(buffer, 1, length, stream);
	buffer[length]='\0';

	glShaderSource(Shader, 1, (const char **)&buffer, NULL);

	fclose(stream);
	FREE(buffer);

	return 1;
}

GLuint CreateComputeProgram(char *Filename)
{
	GLuint Program, Compute;
	GLint _Status, LogLength;
	char *Log=NULL;

	Program=glCreateProgram();

	if(Filename)
	{
		Compute=glCreateShader(GL_COMPUTE_SHADER);

		if(LoadShader(Compute, Filename))
		{
			glCompileShader(Compute);
			glGetShaderiv(Compute, GL_COMPILE_STATUS, &_Status);

			if(!_Status)
			{
				glGetShaderiv(Compute, GL_INFO_LOG_LENGTH, &LogLength);
				Log=(char *)malloc(LogLength);

				if(Log)
				{
					glGetShaderInfoLog(Compute, LogLength, NULL, Log);
					DBGPRINTF("%s - %s\n", Filename, Log);
					FREE(Log);
				}
			}
			else
				glAttachShader(Program, Compute);
		}

		glDeleteShader(Compute);
	}

	glLinkProgram(Program);
	glGetProgramiv(Program, GL_LINK_STATUS, &_Status);

	if(!_Status)
	{
		glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &LogLength);
		Log=(char *)malloc(LogLength);

		if(Log)
		{
			glGetProgramInfoLog(Program, LogLength, NULL, Log);
			DBGPRINTF("Link - %s\n", Log);
			FREE(Log);
		}
	}

	return Program;
}

GLuint CreateShaderProgram(char *VertexFilename, char *FragmentFilename)
{
	GLuint Program, Vertex, Fragment;
	GLint _Status, LogLength;
	char *Log=NULL;

	Program=glCreateProgram();

	if(VertexFilename)
	{
		Vertex=glCreateShader(GL_VERTEX_SHADER);

		if(LoadShader(Vertex, VertexFilename))
		{
			glCompileShader(Vertex);
			glGetShaderiv(Vertex, GL_COMPILE_STATUS, &_Status);

			if(!_Status)
			{
				glGetShaderiv(Vertex, GL_INFO_LOG_LENGTH, &LogLength);
				Log=(char *)malloc(LogLength);

				if(Log)
				{
					glGetShaderInfoLog(Vertex, LogLength, NULL, Log);
					DBGPRINTF("%s - %s\n", VertexFilename, Log);
					FREE(Log);
				}
			}
			else
				glAttachShader(Program, Vertex);
		}

		glDeleteShader(Vertex);
	}

	if(FragmentFilename)
	{
		Fragment=glCreateShader(GL_FRAGMENT_SHADER);

		if(LoadShader(Fragment, FragmentFilename))
		{
			glCompileShader(Fragment);
			glGetShaderiv(Fragment, GL_COMPILE_STATUS, &_Status);

			if(!_Status)
			{
				glGetShaderiv(Fragment, GL_INFO_LOG_LENGTH, &LogLength);
				Log=(char *)malloc(LogLength);

				if(Log)
				{
					glGetShaderInfoLog(Fragment, LogLength, NULL, Log);
					DBGPRINTF("%s - %s\n", FragmentFilename, Log);
					FREE(Log);
				}
			}
			else
				glAttachShader(Program, Fragment);
		}

		glDeleteShader(Fragment);
	}

	glLinkProgram(Program);
	glGetProgramiv(Program, GL_LINK_STATUS, &_Status);

	if(!_Status)
	{
		glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &LogLength);
		Log=(char *)malloc(LogLength);

		if(Log)
		{
			glGetProgramInfoLog(Program, LogLength, NULL, Log);
			DBGPRINTF("Link - %s\n", Log);
			FREE(Log);
		}
	}

	return Program;
}
