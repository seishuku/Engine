#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../utils/list.h"
#include "font.h"
#include "../gl_objects.h"

// Various shader/texture/VBO object IDs
GLuint FontVAO=0, FontVBO=0;

// List for storing current string vectors
List_t FontVectors;

// Initialization flag
bool FontInit=true;

struct
{
	uint32_t Advance;
	uint32_t numPath;
	vec2 *Path;
} Gylphs[256];

uint32_t GylphSize=0;

bool LoadFontGylphs(const char *Filename)
{
	FILE *Stream=NULL;
	const uint32_t GYLP=('G'|('Y'<<8)|('L'<<16)|('P'<<24));

	Stream=fopen(Filename, "rb");

	if(!Stream)
		return false;

	uint32_t Magic=0;

	fread(&Magic, sizeof(uint32_t), 1, Stream);

	if(Magic!=GYLP)
		return false;

	// Vertical size
	fread(&GylphSize, sizeof(uint32_t), 1, Stream);
	GylphSize-=2;

	// Read in all chars
	for(uint32_t i=0;i<255;i++)
	{
		// Advancement spacing for char
		fread(&Gylphs[i].Advance, sizeof(uint32_t), 1, Stream);
		// Number of bezier curves in this char
		fread(&Gylphs[i].numPath, sizeof(uint32_t), 1, Stream);

		// Allocate memory and read in the paths
		Gylphs[i].Path=malloc(sizeof(vec2)*Gylphs[i].numPath);

		if(!Gylphs[i].Path)
			return false;

		fread(Gylphs[i].Path, sizeof(vec2), Gylphs[i].numPath, Stream);
	}

	fclose(Stream);

	return true;
}

void Font_Print(float x, float y, char *string, ...)
{
	// pointer and buffer for formatted text
	char *ptr, text[4096];
	// variable arguments list
	va_list	ap;
	// some misc variables
	int32_t sx=(int32_t)x, numchar;
	// current text color
	float r=1.0f, g=1.0f, b=1.0f;

	// Check if the string is even valid first
	if(string==NULL)
		return;

	// Format string, including variable arguments
	va_start(ap, string);
	vsprintf(text, string, ap);
	va_end(ap);

	// Find how many characters were need to deal with
	numchar=(int32_t)strlen(text);

	// One time init, loads font paths, set up buffers and list
	if(FontInit)
	{
		if(!LoadFontGylphs("./assets/font.gylph"))
			return;

		// Create the vertex array
		glCreateVertexArrays(1, &FontVAO);

		// Set vertex layout

		// Control point vertices
		glVertexArrayAttribFormat(FontVAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(FontVAO, 0, 0);
		glEnableVertexArrayAttrib(FontVAO, 0);

		// Color
		glVertexArrayAttribFormat(FontVAO, 1, 4, GL_FLOAT, GL_FALSE, sizeof(vec4));
		glVertexArrayAttribBinding(FontVAO, 1, 0);
		glEnableVertexArrayAttrib(FontVAO, 1);

		// Create the vertex buffer and assign it to the vertex array
		glCreateBuffers(1, &FontVBO);
		glVertexArrayVertexBuffer(FontVAO, 0, FontVBO, 0, sizeof(vec4)*2);

		// Initalize a list for building a list of control points to feed to the Bezier shader
		List_Init(&FontVectors, sizeof(vec4)*2, 4, NULL);

		// Done with init
		FontInit=false;
	}

	// Loop through the text string until EOL
	for(ptr=text;*ptr!='\0';ptr++)
	{
		// Decrement 'y' for any CR's
		if(*ptr=='\n')
		{
			x=(float)sx;
			y-=GylphSize;
			continue;
		}

		// Just advance spaces instead of rendering empty character
		if(*ptr==' ')
		{
			x+=Gylphs[*ptr].Advance;
			continue;
		}

		// ANSI color escape codes
		// I'm sure there's a better way to do this!
		// But it works, so whatever.
		if(*ptr=='\x1B')
		{
			ptr++;
			     if(*(ptr+0)=='['&&*(ptr+1)=='3'&&*(ptr+2)=='0'&&*(ptr+3)=='m')	{	r=0.0f;	g=0.0f;	b=0.0f;	}	// BLACK
			else if(*(ptr+0)=='['&&*(ptr+1)=='3'&&*(ptr+2)=='1'&&*(ptr+3)=='m')	{	r=0.5f;	g=0.0f;	b=0.0f;	}	// DARK RED
			else if(*(ptr+0)=='['&&*(ptr+1)=='3'&&*(ptr+2)=='2'&&*(ptr+3)=='m')	{	r=0.0f;	g=0.5f;	b=0.0f;	}	// DARK GREEN
			else if(*(ptr+0)=='['&&*(ptr+1)=='3'&&*(ptr+2)=='3'&&*(ptr+3)=='m')	{	r=0.5f;	g=0.5f;	b=0.0f;	}	// DARK YELLOW
			else if(*(ptr+0)=='['&&*(ptr+1)=='3'&&*(ptr+2)=='4'&&*(ptr+3)=='m')	{	r=0.0f;	g=0.0f;	b=0.5f;	}	// DARK BLUE
			else if(*(ptr+0)=='['&&*(ptr+1)=='3'&&*(ptr+2)=='5'&&*(ptr+3)=='m')	{	r=0.5f;	g=0.0f;	b=0.5f;	}	// DARK MAGENTA
			else if(*(ptr+0)=='['&&*(ptr+1)=='3'&&*(ptr+2)=='6'&&*(ptr+3)=='m')	{	r=0.0f;	g=0.5f;	b=0.5f;	}	// DARK CYAN
			else if(*(ptr+0)=='['&&*(ptr+1)=='3'&&*(ptr+2)=='7'&&*(ptr+3)=='m')	{	r=0.5f;	g=0.5f;	b=0.5f;	}	// GREY
			else if(*(ptr+0)=='['&&*(ptr+1)=='9'&&*(ptr+2)=='0'&&*(ptr+3)=='m')	{	r=0.5f;	g=0.5f;	b=0.5f;	}	// GREY
			else if(*(ptr+0)=='['&&*(ptr+1)=='9'&&*(ptr+2)=='1'&&*(ptr+3)=='m')	{	r=1.0f;	g=0.0f;	b=0.0f;	}	// RED
			else if(*(ptr+0)=='['&&*(ptr+1)=='9'&&*(ptr+2)=='2'&&*(ptr+3)=='m')	{	r=0.0f;	g=1.0f;	b=0.0f;	}	// GREEN
			else if(*(ptr+0)=='['&&*(ptr+1)=='9'&&*(ptr+2)=='3'&&*(ptr+3)=='m')	{	r=1.0f;	g=1.0f;	b=0.0f;	}	// YELLOW
			else if(*(ptr+0)=='['&&*(ptr+1)=='9'&&*(ptr+2)=='4'&&*(ptr+3)=='m')	{	r=0.0f;	g=0.0f;	b=1.0f;	}	// BLUE
			else if(*(ptr+0)=='['&&*(ptr+1)=='9'&&*(ptr+2)=='5'&&*(ptr+3)=='m')	{	r=1.0f;	g=0.0f;	b=1.0f;	}	// MAGENTA
			else if(*(ptr+0)=='['&&*(ptr+1)=='9'&&*(ptr+2)=='6'&&*(ptr+3)=='m')	{	r=0.0f;	g=1.0f;	b=1.0f;	}	// CYAN
			else if(*(ptr+0)=='['&&*(ptr+1)=='9'&&*(ptr+2)=='7'&&*(ptr+3)=='m')	{	r=1.0f;	g=1.0f;	b=1.0f;	}	// WHITE
			ptr+=4;
		}

		// Push the current character gylph path with offset and color on to the list
		for(uint32_t i=0;i<Gylphs[*ptr].numPath;i++)
		{
			float vert[]={ Gylphs[*ptr].Path[i][0]+x, Gylphs[*ptr].Path[i][1]+y, -1.0f, 1.0f, r, g, b, 1.0f};
			List_Add(&FontVectors, vert);
		}

		// Advance one character
		x+=Gylphs[*ptr].Advance;
	}

	// Done building the list, upload the data to the GPU.
	glNamedBufferData(FontVBO, sizeof(vec4)*2*List_GetCount(&FontVectors), List_GetPointer(&FontVectors, 0), GL_DYNAMIC_DRAW);

	// Set program and uniforms
	glUseProgram(Objects[GLSL_BEZIER_SHADER]);

	matrix Projection;
	MatrixIdentity(Projection);
	MatrixOrtho(0.0f, 1024.0f*0.9f, 0.0f, 576.0f*0.9f, 0.001f, 100.0f, Projection);
	glUniformMatrix4fv(Objects[GLSL_BEZIER_PROJ], 1, GL_FALSE, Projection);

	matrix ModelView;
	MatrixIdentity(ModelView);
	glUniformMatrix4fv(Objects[GLSL_BEZIER_MV], 1, GL_FALSE, ModelView);

	matrix local;
	MatrixIdentity(local);
	glUniformMatrix4fv(Objects[GLSL_BEZIER_LOCAL], 1, GL_FALSE, local);

	glUniform1ui(Objects[GLSL_BEZIER_NUMSEGMENTS], 10);

	// Draw characters!
	glBindVertexArray(FontVAO);
	glDrawArrays(GL_LINES_ADJACENCY, 0, (GLsizei)List_GetCount(&FontVectors));

	// Clear the list for next time
	List_Clear(&FontVectors);
}

void Font_Destroy(void)
{
	// Loop through gylphs to free memory
	for(uint32_t i=0;i<255;i++)
		free(Gylphs[i].Path);

	// Destroy the control point list
	List_Destroy(&FontVectors);

	// Delete OpenGL objects
	glDeleteBuffers(1, &FontVBO);
	glDeleteVertexArrays(1, &FontVAO);
}
