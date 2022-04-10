#include <windows.h>
#include <process.h>
#include <intrin.h>
#include <crtdbg.h>

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

#define CAMERA_RECORDING 0

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

int Width=1280, Height=720;

int Done=0, Key[256];

unsigned __int64 Frequency, StartTime, EndTime, EndFrameTime;
float avgfps=0.0f, fps=0.0f, fTimeStep=0.0f, fFrameTime=0.0f, fTime=0.0f;
int Frames=0;

unsigned int Objects[NUM_OBJECTS];

Model3DS_t Level;

Model_t Hellknight;
Model_t Fatty;
Model_t Pinky;

Camera_t Camera;
CameraPath_t CameraPath;

int Auto=0;

float Projection[16], ModelView[16], ModelViewInv[16];

float Light0_Pos[4]={ 0.0f, 50.0f, 200.0f, 1.0f/512.0f };
float Light0_Kd[4]={ 1.0f, 1.0f, 1.0f, 1.0f };

float Light1_Pos[4]={ -800.0f, 80.0f, 800.0f, 1.0f/1024.0f };
float Light1_Kd[4]={ 0.75f, 0.75f, 1.0f, 1.0f };

float Light2_Pos[4]={ 800.0f, 80.0f, 800.0f, 1.0f/1024.0f };
float Light2_Kd[4]={ 0.75f, 1.0f, 1.0f, 1.0f };

float Light3_Pos[4]={ -800.0f, 80.0f, -800.0f, 1.0f/1024.0f };
float Light3_Kd[4]={ 0.75f, 1.0f, 0.75f, 1.0f };

float Light4_Pos[4]={ 800.0f, 80.0f, -800.0f, 1.0f/1024.0f };
float Light4_Kd[4]={ 1.0f, 0.75f, 0.75f, 1.0f };

int DynWidth=1024, DynHeight=1024;

typedef struct
{
	const char *Vertex;
	const char *Fragment;
	const char *Geometry;
	const char *Compute;
} ProgNames_t;

void Render(void);
int Init(void);
GLuint CreateShaderProgram(ProgNames_t Names);

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

GLuint VAO, VBO;

#define NUM_SAMPLES 200
float lines[3*NUM_SAMPLES+3];

void UpdateLineChart(const float val)
{
	float temp[3*NUM_SAMPLES+1];
	float xPos=-2.5f;
	float yPos=-0.9f;
	float xScale=2.0f;
	float yScale=0.0625f;

	// Initial seed into the array
	lines[3*0+0]=(1.0f+xPos)/xScale;
	lines[3*0+1]=(val*yScale)+yPos;
	lines[3*0+2]=-1.0f;

	// Propagate out into a temp array
	for(int i=0;i<NUM_SAMPLES;i++)
	{
		temp[3*(i)+0]=(1.0f-((float)i/NUM_SAMPLES*0.5f)+xPos)/xScale;
		temp[3*(i)+1]=lines[3*(i)+1];
		temp[3*(i)+2]=lines[3*(i)+2];
	}

	// Shift the whole array over one, as to create the "continuous" effect
	for(int i=0;i<NUM_SAMPLES-1;i++)
	{
		lines[3*(i+1)+0]=temp[3*(i)+0];
		lines[3*(i+1)+1]=temp[3*(i)+1];
		lines[3*(i+1)+2]=temp[3*(i)+2];
	}

	// Update the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*NUM_SAMPLES+3, lines);
}

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
			// Total frame render in miliseconds
			fFrameTime=(float)(EndFrameTime-StartTime)/(Frequency/1000);
			// Running time
			fTime+=fTimeStep;

			// Accumulate frames per second
			avgfps+=1.0f/fTimeStep;

			// Average over 100 frames
			if(Frames++>100)
			{
				fps=avgfps/Frames;
				avgfps=0.0f;
				Frames=0;
			}

			UpdateLineChart(fFrameTime);
		}
	}

	CameraDeletePath(&CameraPath);

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
					Camera.Yaw-=(float)delta.x/500.0f;
					Camera.Pitch+=(float)delta.y/500.0f;
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

				case VK_SPACE:
#if CAMERA_RECORDING
				{	// Recording camera paths
					FILE *stream=NULL;
					stream=fopen("path.txt", "a");
					if(!stream)
						break;
					fprintf(stream, "%0.3f %0.3f %0.3f %0.3f %0.3f %0.3f\n", Camera.Position[0], Camera.Position[1], Camera.Position[2], Camera.View[0], Camera.View[1], Camera.View[2]);
					fclose(stream);
				}
#else
					Auto^=1;
#endif
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

void UpdateShadow(GLuint texture, GLuint buffer, float *pos)
{
	float proj[16], mv[6][16], local[16];
	glBindFramebuffer(GL_FRAMEBUFFER, buffer);

	glViewport(0, 0, DynWidth, DynHeight);
	MatrixIdentity(proj);
	InfPerspective(90.0f, (float)DynWidth/DynHeight, 0.01f, 0, proj);

	glClear(GL_DEPTH_BUFFER_BIT);

	MatrixIdentity(mv[0]);
	LookAt(pos, (float[]) { pos[0]+1.0f, pos[1]+0.0f, pos[2]+0.0f }, (float[]) { 0.0f, -1.0f, 0.0f }, mv[0]);

	MatrixIdentity(mv[1]);
	LookAt(pos, (float[]) { pos[0]-1.0f, pos[1]+0.0f, pos[2]+0.0f }, (float[]) { 0.0f, -1.0f, 0.0f }, mv[1]);

	MatrixIdentity(mv[2]);
	LookAt(pos, (float[]) { pos[0]+0.0f, pos[1]+1.0f, pos[2]+0.0f }, (float[]) { 0.0f, 0.0f, 1.0f }, mv[2]);

	MatrixIdentity(mv[3]);
	LookAt(pos, (float[]) { pos[0]+0.0f, pos[1]-1.0f, pos[2]+0.0f }, (float[]) { 0.0f, 0.0f, -1.0f }, mv[3]);

	MatrixIdentity(mv[4]);
	LookAt(pos, (float[]) { pos[0]+0.0f, pos[1]+0.0f, pos[2]+1.0f }, (float[]) { 0.0f, -1.0f, 0.0f }, mv[4]);

	MatrixIdentity(mv[5]);
	LookAt(pos, (float[]) { pos[0]+0.0f, pos[1]+0.0f, pos[2]-1.0f }, (float[]) { 0.0f, -1.0f, 0.0f }, mv[5]);

	// Select the shader program
	glUseProgram(Objects[GLSL_DISTANCE_SHADER]);

	glUniformMatrix4fv(Objects[GLSL_DISTANCE_PROJ], 1, GL_FALSE, proj);

	glUniformMatrix4fv(Objects[GLSL_DISTANCE_MV], 6, GL_FALSE, (float *)mv);

	glUniform4fv(Objects[GLSL_DISTANCE_LIGHTPOS], 1, pos);

	// Render models
	MatrixIdentity(local);
	MatrixTranslate(0.0f, -100.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	glUniformMatrix4fv(Objects[GLSL_DISTANCE_LOCAL], 1, GL_FALSE, local);
	DrawModelMD5(&Hellknight.Model);

	MatrixIdentity(local);
	MatrixTranslate(-100.0f, -100.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	glUniformMatrix4fv(Objects[GLSL_DISTANCE_LOCAL], 1, GL_FALSE, local);
	DrawModelMD5(&Fatty.Model);

	MatrixIdentity(local);
	MatrixTranslate(100.0f, -100.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	glUniformMatrix4fv(Objects[GLSL_DISTANCE_LOCAL], 1, GL_FALSE, local);
	DrawModelMD5(&Pinky.Model);

	MatrixIdentity(local);
	glUniformMatrix4fv(Objects[GLSL_DISTANCE_LOCAL], 1, GL_FALSE, local);
	DrawModel3DS(&Level);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render(void)
{
	float local[16];
	float min[3]={ 0.0f, };
	float max[3]={ 0.0f, };

	for(int i=0;i<Level.NumMesh;i++)
		CameraCheckCollision(&Camera, Level.Mesh[i].Vertex, Level.Mesh[i].Face, Level.Mesh[i].NumFace);

	// Sphere -> BBox intersection testing
	//MatrixIdentity(local);
	//MatrixTranslate(0.0f, -100.0f, 0.0f, local);
	//MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	//MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	//Matrix4x4MultVec3(Hellknight.Anim.bboxes[Hellknight.frame].min, local, min);
	//Matrix4x4MultVec3(Hellknight.Anim.bboxes[Hellknight.frame].max, local, max);
	//int collide=SphereBBOXIntersection(Camera.Position, Camera.Radius, min, max);
	////

	UpdateAnimation(&Hellknight, fTimeStep);
	UpdateAnimation(&Fatty, fTimeStep);
	UpdateAnimation(&Pinky, fTimeStep);

	Light0_Pos[0]=sinf(fTime)*75.0f;
	Light0_Pos[2]=cosf(fTime)*75.0f;

	UpdateShadow(Objects[TEXTURE_DISTANCE0], Objects[BUFFER_DISTANCE0], Light0_Pos);
		
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Set viewport and calculate a projection matrix (perspective, with infinite z-far plane)
	glViewport(0, 0, Width, Height);
	MatrixIdentity(Projection);
	InfPerspective(90.0f, (float)Width/Height, 0.01f, 0, Projection);

	// Set up model view matrix (translate and rotation)
	MatrixIdentity(ModelView);

	if(Auto)
		CameraInterpolatePath(&CameraPath, &Camera, fTimeStep, ModelView);
	else
		CameraUpdate(&Camera, fTimeStep, ModelView);

	// Select the shader program
	glUseProgram(Objects[GLSL_LIGHT_SHADER]);

	// Light parameters
	glUniform4fv(Objects[GLSL_LIGHT_LIGHT0_POS], 1, Light0_Pos);
	glUniform4fv(Objects[GLSL_LIGHT_LIGHT0_KD], 1, Light0_Kd);

	glUniform4fv(Objects[GLSL_LIGHT_LIGHT1_POS], 1, Light1_Pos);
	glUniform4fv(Objects[GLSL_LIGHT_LIGHT1_KD], 1, Light1_Kd);

	glUniform4fv(Objects[GLSL_LIGHT_LIGHT2_POS], 1, Light2_Pos);
	glUniform4fv(Objects[GLSL_LIGHT_LIGHT2_KD], 1, Light2_Kd);

	glUniform4fv(Objects[GLSL_LIGHT_LIGHT3_POS], 1, Light3_Pos);
	glUniform4fv(Objects[GLSL_LIGHT_LIGHT3_KD], 1, Light3_Kd);

	glUniform4fv(Objects[GLSL_LIGHT_LIGHT4_POS], 1, Light4_Pos);
	glUniform4fv(Objects[GLSL_LIGHT_LIGHT4_KD], 1, Light4_Kd);

	// Projection matrix
	glUniformMatrix4fv(Objects[GLSL_LIGHT_PROJ], 1, GL_FALSE, Projection);

	// Global ModelView
	glUniformMatrix4fv(Objects[GLSL_LIGHT_MV], 1, GL_FALSE, ModelView);

	// Bind correct textures per model
	glBindTextureUnit(3, Objects[TEXTURE_DISTANCE0]);

	// Render model
	glBindTextureUnit(0, Objects[TEXTURE_HELLKNIGHT_BASE]);
	glBindTextureUnit(1, Objects[TEXTURE_HELLKNIGHT_SPECULAR]);
	glBindTextureUnit(2, Objects[TEXTURE_HELLKNIGHT_NORMAL]);
	MatrixIdentity(local);
	MatrixTranslate(0.0f, -100.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
	DrawModelMD5(&Hellknight.Model);

	glBindTextureUnit(0, Objects[TEXTURE_FATTY_BASE]);
	glBindTextureUnit(1, Objects[TEXTURE_FATTY_SPECULAR]);
	glBindTextureUnit(2, Objects[TEXTURE_FATTY_NORMAL]);
	MatrixIdentity(local);
	MatrixTranslate(-100.0f, -100.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
	glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
	DrawModelMD5(&Fatty.Model);

	MatrixIdentity(local);
	MatrixTranslate(100.0f, -100.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
	glBindTextureUnit(0, Objects[TEXTURE_PINKY_BASE]);
	glBindTextureUnit(1, Objects[TEXTURE_PINKY_SPECULAR]);
	glBindTextureUnit(2, Objects[TEXTURE_PINKY_NORMAL]);
	glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
	DrawModelMD5(&Pinky.Model);

	MatrixIdentity(local);
	glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
	DrawModel3DS(&Level);

	glActiveTexture(GL_TEXTURE0);

	glUseProgram(Objects[GLSL_GENERIC_SHADER]);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_STRIP, 0, NUM_SAMPLES);

	glBindVertexArray(0);

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
		Font_Print(0.0f, 16.0f, "FPS: %0.1f\nFrame time: %0.4fms", fps, fFrameTime);
		//if(collide)
		//	Font_Print(0.0f, (float)Height-16.0f, "Ran into hellknight");
		glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

int LoadMaterials3DS(Model3DS_t *Model)
{
	for(int i=0;i<Model->NumMaterial;i++)
	{
		char buf[256], nameNoExt[256], fileExt[256], *ptr=NULL;

		for(char *p=Model->Material[i].Texture;*p;p++)
			*p=*p>0x40&&*p<0x5b?*p|0x60:*p;

		strncpy(nameNoExt, Model->Material[i].Texture, 256);
		ptr=strstr(nameNoExt, ".");
		strncpy(fileExt, ptr, 256);

		if(!ptr)
			continue;

		ptr[0]='\0';

		snprintf(buf, 256, "./assets/%s", Model->Material[i].Texture);
		Model->Material[i].TexBaseID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_TRILINEAR);

		snprintf(buf, 256, "./assets/%s_b%s", nameNoExt, fileExt);
		Model->Material[i].TexNormalID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_NORMALMAP|IMAGE_TRILINEAR);

		snprintf(buf, 256, "./assets/%s_s%s", nameNoExt, fileExt);
		Model->Material[i].TexSpecularID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_TRILINEAR);
	}
}

int Init(void)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	memset(lines, 0, sizeof(float)*3*NUM_SAMPLES+3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*NUM_SAMPLES+3, lines, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

#if CAMERA_RECORDING
	CameraInit(&Camera, (float[]) { 0.0f, 0.0f, 0.0f }, (float[]) { 0.0f, 0.0f, 1.0f }, (float[3]) { 0.0f, 1.0f, 0.0f });
#else
	if(!CameraLoadPath("path.txt", &CameraPath))
		return 0;

	// Set up camera structs
	CameraInit(&Camera, CameraPath.Position, CameraPath.View, (float[3]) { 0.0f, 1.0f, 0.0f });
#endif

	// Load texture images
	Objects[TEXTURE_HELLKNIGHT_BASE]=Image_Upload("./assets/hellknight.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR);
	Objects[TEXTURE_HELLKNIGHT_SPECULAR]=Image_Upload("./assets/hellknight_s.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR);
	Objects[TEXTURE_HELLKNIGHT_NORMAL]=Image_Upload("./assets/hellknight_n.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR|IMAGE_NORMALIZE);

	Objects[TEXTURE_FATTY_BASE]=Image_Upload("./assets/fatty.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR);
	Objects[TEXTURE_FATTY_SPECULAR]=Image_Upload("./assets/fatty_s.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR);
	Objects[TEXTURE_FATTY_NORMAL]=Image_Upload("./assets/fatty_n.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR|IMAGE_NORMALIZE);

	Objects[TEXTURE_PINKY_BASE]=Image_Upload("./assets/pinky.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR);
	Objects[TEXTURE_PINKY_SPECULAR]=Image_Upload("./assets/pinky_s.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR);
	Objects[TEXTURE_PINKY_NORMAL]=Image_Upload("./assets/pinky_n.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR|IMAGE_NORMALIZE);

	// Build a VAO/VBO for the skybox
	BuildSkyboxVBO();

	// Load the "level" 3D Studio model
	if(Load3DS(&Level, "./assets/room.3ds"))
	{
		BuildVBO3DS(&Level);
		LoadMaterials3DS(&Level);
	}
	else
		return 0;

	// Compile/link MD5 skinning compute program
	Objects[GLSL_MD5_GENVERTS_COMPUTE]=CreateShaderProgram((ProgNames_t) { NULL, NULL, NULL, "./shaders/md5_genverts_c.glsl" });

	// Load MD5 model meshes
	if(!LoadMD5Model("./assets/hellknight", &Hellknight))
		return 0;

	if(!LoadMD5Model("./assets/fatty", &Fatty))
		return 0;

	if(!LoadMD5Model("./assets/pinky", &Pinky))
		return 0;

	// Load shaders

	// Generic debugging shader
	Objects[GLSL_GENERIC_SHADER]=CreateShaderProgram((ProgNames_t) { "./shaders/generic_v.glsl", "./shaders/generic_f.glsl", NULL, NULL });

	// General lighting shader
	Objects[GLSL_LIGHT_SHADER]=CreateShaderProgram((ProgNames_t) { "./shaders/light_v.glsl", "./shaders/light_f.glsl", NULL, NULL });
	glUseProgram(Objects[GLSL_LIGHT_SHADER]);
	Objects[GLSL_LIGHT_PROJ]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "proj");
	Objects[GLSL_LIGHT_MVINV]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "mvinv");
	Objects[GLSL_LIGHT_MV]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "mv");
	Objects[GLSL_LIGHT_LOCAL]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "local");
	Objects[GLSL_LIGHT_LIGHT0_POS]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light0_Pos");
	Objects[GLSL_LIGHT_LIGHT0_KD]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light0_Kd");
	Objects[GLSL_LIGHT_LIGHT1_POS]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light1_Pos");
	Objects[GLSL_LIGHT_LIGHT1_KD]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light1_Kd");
	Objects[GLSL_LIGHT_LIGHT2_POS]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light2_Pos");
	Objects[GLSL_LIGHT_LIGHT2_KD]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light2_Kd");
	Objects[GLSL_LIGHT_LIGHT3_POS]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light3_Pos");
	Objects[GLSL_LIGHT_LIGHT3_KD]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light3_Kd");
	Objects[GLSL_LIGHT_LIGHT4_POS]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light4_Pos");
	Objects[GLSL_LIGHT_LIGHT4_KD]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Light4_Kd");

	// Build program for generating depth cube map
	Objects[GLSL_DISTANCE_SHADER]=CreateShaderProgram((ProgNames_t) { "./shaders/distance_v.glsl", "./shaders/distance_f.glsl", "./shaders/distance_g.glsl", NULL });
	glUseProgram(Objects[GLSL_DISTANCE_SHADER]);
	Objects[GLSL_DISTANCE_PROJ]=glGetUniformLocation(Objects[GLSL_DISTANCE_SHADER], "proj");
	Objects[GLSL_DISTANCE_MV]=glGetUniformLocation(Objects[GLSL_DISTANCE_SHADER], "mv");
	Objects[GLSL_DISTANCE_LOCAL]=glGetUniformLocation(Objects[GLSL_DISTANCE_SHADER], "local");
	Objects[GLSL_DISTANCE_LIGHTPOS]=glGetUniformLocation(Objects[GLSL_DISTANCE_SHADER], "Light_Pos");

	// Genereate texture and frame buffer for the depth cube map
	glGenTextures(1, &Objects[TEXTURE_DISTANCE0]);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Objects[TEXTURE_DISTANCE0]);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	for(int i=0;i<6;i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_DEPTH_COMPONENT32, DynWidth, DynHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glGenFramebuffers(1, &Objects[BUFFER_DISTANCE0]);
	glBindFramebuffer(GL_FRAMEBUFFER, Objects[BUFFER_DISTANCE0]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Objects[TEXTURE_DISTANCE0], 0);

	// Disable drawing, we're only interested in depth information
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Set OpenGL states
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return 1;
}

int LoadShader(GLuint Shader, const char *Filename)
{
	FILE *stream=NULL;

	if((stream=fopen(Filename, "rb"))==NULL)
		return 0;

	fseek(stream, 0, SEEK_END);
	size_t length=ftell(stream);
	fseek(stream, 0, SEEK_SET);

	char *buffer=(char *)malloc(length+1);

	if(buffer==NULL)
		return 0;

	fread(buffer, 1, length, stream);
	buffer[length]='\0';

	glShaderSource(Shader, 1, (const char **)&buffer, NULL);

	fclose(stream);
	FREE(buffer);

	return 1;
}

void CompileAndAttachShader(GLuint Program, const char *Filename, GLuint Target)
{
	GLint _Status=0, LogLength=0;
	char *Log=NULL;

	GLuint Shader=glCreateShader(Target);

	if(LoadShader(Shader, Filename))
	{
		glCompileShader(Shader);
		glGetShaderiv(Shader, GL_COMPILE_STATUS, &_Status);

		if(!_Status)
		{
			glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogLength);
			Log=(char *)malloc(LogLength);

			if(Log)
			{
				glGetShaderInfoLog(Shader, LogLength, NULL, Log);
				DBGPRINTF("%s - %s\n", Filename, Log);
				FREE(Log);
			}
		}
		else
			glAttachShader(Program, Shader);
	}

	glDeleteShader(Shader);
}

GLuint CreateShaderProgram(ProgNames_t Names)
{
	GLint _Status=0, LogLength=0;
	GLchar *Log=NULL;

	GLuint Program=glCreateProgram();

	if(Names.Vertex)
		CompileAndAttachShader(Program, Names.Vertex, GL_VERTEX_SHADER);

	if(Names.Fragment)
		CompileAndAttachShader(Program, Names.Fragment, GL_FRAGMENT_SHADER);

	if(Names.Geometry)
		CompileAndAttachShader(Program, Names.Geometry, GL_GEOMETRY_SHADER);

	if(Names.Compute)
		CompileAndAttachShader(Program, Names.Compute, GL_COMPUTE_SHADER);

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
