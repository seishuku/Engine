#include "opengl.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#define DBGPRINTF(...) { char buf[512]; snprintf(buf, sizeof(buf), __VA_ARGS__); OutputDebugString(buf); }
#else
#define DBGPRINTF(...) { fprintf(stderr, __VA_ARGS__); }
#endif

#ifndef FREE
#define FREE(p) { if(p) { free(p); p=NULL; } }
#endif

#ifdef __linux__
extern Display *dpy;
#endif

#ifdef WIN32
GLboolean WGL_ARB_create_context_Flag=0;
HGLRC (*wglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext, const int *attribList)=NULL;

GLboolean WGL_ARB_create_context_profile_Flag=0;

GLboolean WGL_ARB_create_context_robustness_Flag=0;

GLboolean WGL_ARB_extensions_string_Flag=0;
const char *(*wglGetExtensionsStringARB)(HDC hdc)=NULL;

GLboolean WGL_ARB_framebuffer_sRGB_Flag=0;

GLboolean WGL_ARB_make_current_read_Flag=0;

GLboolean WGL_ARB_multisample_Flag=0;

BOOL (*wglMakeContextCurrentARB)(HDC hDrawDC, HDC hReadDC, HGLRC hglrc)=NULL;
HDC (*wglGetCurrentReadDCARB)(void)=NULL;

GLboolean WGL_ARB_pixel_format_Flag=0;
BOOL (*wglGetPixelFormatAttribivARB)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues)=NULL;
BOOL (*wglGetPixelFormatAttribfvARB)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues)=NULL;
BOOL (*wglChoosePixelFormatARB)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats)=NULL;
#else
GLXContext (*glXCreateContextAttribsARB)(Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list)=NULL;

GLboolean GLX_ARB_create_context_Flag=0;

GLboolean GLX_ARB_create_context_profile_Flag=0;

GLboolean GLX_ARB_create_context_robustness_Flag=0;

GLboolean GLX_ARB_framebuffer_sRGB_Flag=0;

GLboolean GLX_ARB_multisample_Flag=0;
#endif

#ifdef WIN32 // Windows only has up to OpenGL 1.1, Linux is 1.4

// OpenGL 1.2
GLvoid (*glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)=NULL;
GLvoid (*glTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)=NULL;
GLvoid (*glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)=NULL;
GLvoid (*glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)=NULL;

// OpenGL 1.3
GLvoid (*glActiveTexture)(GLenum texture)=NULL;
GLvoid (*glSampleCoverage)(GLfloat value, GLboolean invert)=NULL;
GLvoid (*glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)=NULL;
GLvoid (*glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)=NULL;
GLvoid (*glCompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)=NULL;
GLvoid (*glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)=NULL;
GLvoid (*glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)=NULL;
GLvoid (*glCompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)=NULL;
GLvoid (*glGetCompressedTexImage)(GLenum target, GLint level, GLvoid *img)=NULL;

// OpenGL 1.4
GLvoid (*glBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)=NULL;
GLvoid (*glMultiDrawArrays)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount)=NULL;
GLvoid (*glMultiDrawElements)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid *const*indices, GLsizei drawcount)=NULL;
GLvoid (*glPointParameterf)(GLenum pname, GLfloat param)=NULL;
GLvoid (*glPointParameterfv)(GLenum pname, const GLfloat *params)=NULL;
GLvoid (*glPointParameteri)(GLenum pname, GLint param)=NULL;
GLvoid (*glPointParameteriv)(GLenum pname, const GLint *params)=NULL;
GLvoid (*glBlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)=NULL;
GLvoid (*glBlendEquation)(GLenum mode)=NULL;
#endif

// OpenGL 1.5
GLvoid (*glGenQueries)(GLsizei n, GLuint *ids)=NULL;
GLvoid (*glDeleteQueries)(GLsizei n, const GLuint *ids)=NULL;
GLboolean (*glIsQuery)(GLuint id)=NULL;
GLvoid (*glBeginQuery)(GLenum target, GLuint id)=NULL;
GLvoid (*glEndQuery)(GLenum target)=NULL;
GLvoid (*glGetQueryiv)(GLenum target, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetQueryObjectiv)(GLuint id, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params)=NULL;
GLvoid (*glBindBuffer)(GLenum target, GLuint buffer)=NULL;
GLvoid (*glDeleteBuffers)(GLsizei n, const GLuint *buffers)=NULL;
GLvoid (*glGenBuffers)(GLsizei n, GLuint *buffers)=NULL;
GLboolean (*glIsBuffer)(GLuint buffer)=NULL;
GLvoid (*glBufferData)(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)=NULL;
GLvoid (*glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)=NULL;
GLvoid (*glGetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)=NULL;
GLvoid *(*glMapBuffer)(GLenum target, GLenum access)=NULL;
GLboolean (*glUnmapBuffer)(GLenum target)=NULL;
GLvoid (*glGetBufferParameteriv)(GLenum target, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetBufferPointerv)(GLenum target, GLenum pname, GLvoid **params)=NULL;

// OpenGL 2.0
GLvoid (*glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha)=NULL;
GLvoid (*glDrawBuffers)(GLsizei n, const GLenum *bufs)=NULL;
GLvoid (*glStencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)=NULL;
GLvoid (*glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask)=NULL;
GLvoid (*glStencilMaskSeparate)(GLenum face, GLuint mask)=NULL;
GLvoid (*glAttachShader)(GLuint program, GLuint shader)=NULL;
GLvoid (*glBindAttribLocation)(GLuint program, GLuint index, const GLchar *name)=NULL;
GLvoid (*glCompileShader)(GLuint shader)=NULL;
GLuint (*glCreateProgram)(GLvoid);
GLuint (*glCreateShader)(GLenum type)=NULL;
GLvoid (*glDeleteProgram)(GLuint program)=NULL;
GLvoid (*glDeleteShader)(GLuint shader)=NULL;
GLvoid (*glDetachShader)(GLuint program, GLuint shader)=NULL;
GLvoid (*glDisableVertexAttribArray)(GLuint index)=NULL;
GLvoid (*glEnableVertexAttribArray)(GLuint index)=NULL;
GLvoid (*glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)=NULL;
GLvoid (*glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)=NULL;
GLvoid (*glGetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)=NULL;
GLint (*glGetAttribLocation)(GLuint program, const GLchar *name)=NULL;
GLvoid (*glGetProgramiv)(GLuint program, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)=NULL;
GLvoid (*glGetShaderiv)(GLuint shader, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)=NULL;
GLvoid (*glGetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)=NULL;
GLint (*glGetUniformLocation)(GLuint program, const GLchar *name)=NULL;
GLvoid (*glGetUniformfv)(GLuint program, GLint location, GLfloat *params)=NULL;
GLvoid (*glGetUniformiv)(GLuint program, GLint location, GLint *params)=NULL;
GLvoid (*glGetVertexAttribdv)(GLuint index, GLenum pname, GLdouble *params)=NULL;
GLvoid (*glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params)=NULL;
GLvoid (*glGetVertexAttribiv)(GLuint index, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid **pointer)=NULL;
GLboolean (*glIsProgram)(GLuint program)=NULL;
GLboolean (*glIsShader)(GLuint shader)=NULL;
GLvoid (*glLinkProgram)(GLuint program)=NULL;
GLvoid (*glShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length)=NULL;
GLvoid (*glUseProgram)(GLuint program)=NULL;
GLvoid (*glUniform1f)(GLint location, GLfloat v0)=NULL;
GLvoid (*glUniform2f)(GLint location, GLfloat v0, GLfloat v1)=NULL;
GLvoid (*glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)=NULL;
GLvoid (*glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)=NULL;
GLvoid (*glUniform1i)(GLint location, GLint v0)=NULL;
GLvoid (*glUniform2i)(GLint location, GLint v0, GLint v1)=NULL;
GLvoid (*glUniform3i)(GLint location, GLint v0, GLint v1, GLint v2)=NULL;
GLvoid (*glUniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)=NULL;
GLvoid (*glUniform1fv)(GLint location, GLsizei count, const GLfloat *value)=NULL;
GLvoid (*glUniform2fv)(GLint location, GLsizei count, const GLfloat *value)=NULL;
GLvoid (*glUniform3fv)(GLint location, GLsizei count, const GLfloat *value)=NULL;
GLvoid (*glUniform4fv)(GLint location, GLsizei count, const GLfloat *value)=NULL;
GLvoid (*glUniform1iv)(GLint location, GLsizei count, const GLint *value)=NULL;
GLvoid (*glUniform2iv)(GLint location, GLsizei count, const GLint *value)=NULL;
GLvoid (*glUniform3iv)(GLint location, GLsizei count, const GLint *value)=NULL;
GLvoid (*glUniform4iv)(GLint location, GLsizei count, const GLint *value)=NULL;
GLvoid (*glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)=NULL;
GLvoid (*glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)=NULL;
GLvoid (*glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)=NULL;
GLvoid (*glValidateProgram)(GLuint program)=NULL;
GLvoid (*glVertexAttrib1d)(GLuint index, GLdouble x)=NULL;
GLvoid (*glVertexAttrib1dv)(GLuint index, const GLdouble *v)=NULL;
GLvoid (*glVertexAttrib1f)(GLuint index, GLfloat x)=NULL;
GLvoid (*glVertexAttrib1fv)(GLuint index, const GLfloat *v)=NULL;
GLvoid (*glVertexAttrib1s)(GLuint index, GLshort x)=NULL;
GLvoid (*glVertexAttrib1sv)(GLuint index, const GLshort *v)=NULL;
GLvoid (*glVertexAttrib2d)(GLuint index, GLdouble x, GLdouble y)=NULL;
GLvoid (*glVertexAttrib2dv)(GLuint index, const GLdouble *v)=NULL;
GLvoid (*glVertexAttrib2f)(GLuint index, GLfloat x, GLfloat y)=NULL;
GLvoid (*glVertexAttrib2fv)(GLuint index, const GLfloat *v)=NULL;
GLvoid (*glVertexAttrib2s)(GLuint index, GLshort x, GLshort y)=NULL;
GLvoid (*glVertexAttrib2sv)(GLuint index, const GLshort *v)=NULL;
GLvoid (*glVertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z)=NULL;
GLvoid (*glVertexAttrib3dv)(GLuint index, const GLdouble *v)=NULL;
GLvoid (*glVertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z)=NULL;
GLvoid (*glVertexAttrib3fv)(GLuint index, const GLfloat *v)=NULL;
GLvoid (*glVertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z)=NULL;
GLvoid (*glVertexAttrib3sv)(GLuint index, const GLshort *v)=NULL;
GLvoid (*glVertexAttrib4Nbv)(GLuint index, const GLbyte *v)=NULL;
GLvoid (*glVertexAttrib4Niv)(GLuint index, const GLint *v)=NULL;
GLvoid (*glVertexAttrib4Nsv)(GLuint index, const GLshort *v)=NULL;
GLvoid (*glVertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)=NULL;
GLvoid (*glVertexAttrib4Nubv)(GLuint index, const GLubyte *v)=NULL;
GLvoid (*glVertexAttrib4Nuiv)(GLuint index, const GLuint *v)=NULL;
GLvoid (*glVertexAttrib4Nusv)(GLuint index, const GLushort *v)=NULL;
GLvoid (*glVertexAttrib4bv)(GLuint index, const GLbyte *v)=NULL;
GLvoid (*glVertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)=NULL;
GLvoid (*glVertexAttrib4dv)(GLuint index, const GLdouble *v)=NULL;
GLvoid (*glVertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)=NULL;
GLvoid (*glVertexAttrib4fv)(GLuint index, const GLfloat *v)=NULL;
GLvoid (*glVertexAttrib4iv)(GLuint index, const GLint *v)=NULL;
GLvoid (*glVertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)=NULL;
GLvoid (*glVertexAttrib4sv)(GLuint index, const GLshort *v)=NULL;
GLvoid (*glVertexAttrib4ubv)(GLuint index, const GLubyte *v)=NULL;
GLvoid (*glVertexAttrib4uiv)(GLuint index, const GLuint *v)=NULL;
GLvoid (*glVertexAttrib4usv)(GLuint index, const GLushort *v)=NULL;
GLvoid (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)=NULL;

// OpenGL 2.1
GLvoid (*glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)=NULL;
GLvoid (*glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)=NULL;
GLvoid (*glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)=NULL;
GLvoid (*glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)=NULL;
GLvoid (*glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)=NULL;
GLvoid (*glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)=NULL;

// OpenGL 3.0
GLvoid (*glColorMaski)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)=NULL;
GLvoid (*glGetBooleani_v)(GLenum target, GLuint index, GLboolean *data)=NULL;
GLvoid (*glGetIntegeri_v)(GLenum target, GLuint index, GLint *data)=NULL;
GLvoid (*glEnablei)(GLenum target, GLuint index)=NULL;
GLvoid (*glDisablei)(GLenum target, GLuint index)=NULL;
GLboolean (*glIsEnabledi)(GLenum target, GLuint index)=NULL;
GLvoid (*glBeginTransformFeedback)(GLenum primitiveMode)=NULL;
GLvoid (*glEndTransformFeedback)(GLvoid)=NULL;
GLvoid (*glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)=NULL;
GLvoid (*glBindBufferBase)(GLenum target, GLuint index, GLuint buffer)=NULL;
GLvoid (*glTransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)=NULL;
GLvoid (*glGetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)=NULL;
GLvoid (*glClampColor)(GLenum target, GLenum clamp)=NULL;
GLvoid (*glBeginConditionalRender)(GLuint id, GLenum mode)=NULL;
GLvoid (*glEndConditionalRender)(GLvoid)=NULL;
GLvoid (*glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)=NULL;
GLvoid (*glGetVertexAttribIiv)(GLuint index, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint *params)=NULL;
GLvoid (*glVertexAttribI1i)(GLuint index, GLint x)=NULL;
GLvoid (*glVertexAttribI2i)(GLuint index, GLint x, GLint y)=NULL;
GLvoid (*glVertexAttribI3i)(GLuint index, GLint x, GLint y, GLint z)=NULL;
GLvoid (*glVertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w)=NULL;
GLvoid (*glVertexAttribI1ui)(GLuint index, GLuint x)=NULL;
GLvoid (*glVertexAttribI2ui)(GLuint index, GLuint x, GLuint y)=NULL;
GLvoid (*glVertexAttribI3ui)(GLuint index, GLuint x, GLuint y, GLuint z)=NULL;
GLvoid (*glVertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)=NULL;
GLvoid (*glVertexAttribI1iv)(GLuint index, const GLint *v)=NULL;
GLvoid (*glVertexAttribI2iv)(GLuint index, const GLint *v)=NULL;
GLvoid (*glVertexAttribI3iv)(GLuint index, const GLint *v)=NULL;
GLvoid (*glVertexAttribI4iv)(GLuint index, const GLint *v)=NULL;
GLvoid (*glVertexAttribI1uiv)(GLuint index, const GLuint *v)=NULL;
GLvoid (*glVertexAttribI2uiv)(GLuint index, const GLuint *v)=NULL;
GLvoid (*glVertexAttribI3uiv)(GLuint index, const GLuint *v)=NULL;
GLvoid (*glVertexAttribI4uiv)(GLuint index, const GLuint *v)=NULL;
GLvoid (*glVertexAttribI4bv)(GLuint index, const GLbyte *v)=NULL;
GLvoid (*glVertexAttribI4sv)(GLuint index, const GLshort *v)=NULL;
GLvoid (*glVertexAttribI4ubv)(GLuint index, const GLubyte *v)=NULL;
GLvoid (*glVertexAttribI4usv)(GLuint index, const GLushort *v)=NULL;
GLvoid (*glGetUniformuiv)(GLuint program, GLint location, GLuint *params)=NULL;
GLvoid (*glBindFragDataLocation)(GLuint program, GLuint color, const GLchar *name)=NULL;
GLint (*glGetFragDataLocation)(GLuint program, const GLchar *name)=NULL;
GLvoid (*glUniform1ui)(GLint location, GLuint v0)=NULL;
GLvoid (*glUniform2ui)(GLint location, GLuint v0, GLuint v1)=NULL;
GLvoid (*glUniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2)=NULL;
GLvoid (*glUniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)=NULL;
GLvoid (*glUniform1uiv)(GLint location, GLsizei count, const GLuint *value)=NULL;
GLvoid (*glUniform2uiv)(GLint location, GLsizei count, const GLuint *value)=NULL;
GLvoid (*glUniform3uiv)(GLint location, GLsizei count, const GLuint *value)=NULL;
GLvoid (*glUniform4uiv)(GLint location, GLsizei count, const GLuint *value)=NULL;
GLvoid (*glTexParameterIiv)(GLenum target, GLenum pname, const GLint *params)=NULL;
GLvoid (*glTexParameterIuiv)(GLenum target, GLenum pname, const GLuint *params)=NULL;
GLvoid (*glGetTexParameterIiv)(GLenum target, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetTexParameterIuiv)(GLenum target, GLenum pname, GLuint *params)=NULL;
GLvoid (*glClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint *value)=NULL;
GLvoid (*glClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint *value)=NULL;
GLvoid (*glClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat *value)=NULL;
GLvoid (*glClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)=NULL;
const GLubyte *(*glGetStringi)(GLenum name, GLuint index)=NULL;
GLboolean (*glIsRenderbuffer)(GLuint renderbuffer)=NULL;
GLvoid (*glBindRenderbuffer)(GLenum target, GLuint renderbuffer)=NULL;
GLvoid (*glDeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers)=NULL;
GLvoid (*glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers)=NULL;
GLvoid (*glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)=NULL;
GLvoid (*glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint *params)=NULL;
GLboolean (*glIsFramebuffer)(GLuint framebuffer)=NULL;
GLvoid (*glBindFramebuffer)(GLenum target, GLuint framebuffer)=NULL;
GLvoid (*glDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers)=NULL;
GLvoid (*glGenFramebuffers)(GLsizei n, GLuint *framebuffers)=NULL;
GLenum (*glCheckFramebufferStatus)(GLenum target)=NULL;
GLvoid (*glFramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)=NULL;
GLvoid (*glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)=NULL;
GLvoid (*glFramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)=NULL;
GLvoid (*glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)=NULL;
GLvoid (*glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint *params)=NULL;
GLvoid (*glGenerateMipmap)(GLenum target)=NULL;
GLvoid (*glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)=NULL;
GLvoid (*glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)=NULL;
GLvoid (*glFramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)=NULL;
GLvoid *(*glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)=NULL;
GLvoid (*glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length)=NULL;
GLvoid (*glBindVertexArray)(GLuint array)=NULL;
GLvoid (*glDeleteVertexArrays)(GLsizei n, const GLuint *arrays)=NULL;
GLvoid (*glGenVertexArrays)(GLsizei n, GLuint *arrays)=NULL;
GLboolean (*glIsVertexArray)(GLuint array)=NULL;

// OpenGL 3.1
GLvoid (*glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)=NULL;
GLvoid (*glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount)=NULL;
GLvoid (*glTexBuffer)(GLenum target, GLenum internalformat, GLuint buffer)=NULL;
GLvoid (*glPrimitiveRestartIndex)(GLuint index)=NULL;
GLvoid (*glCopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)=NULL;
GLvoid (*glGetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices)=NULL;
GLvoid (*glGetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)=NULL;
GLuint (*glGetUniformBlockIndex)(GLuint program, const GLchar *uniformBlockName)=NULL;
GLvoid (*glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)=NULL;
GLvoid (*glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)=NULL;

// OpenGL 3.2
GLvoid (*glDrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex)=NULL;
GLvoid (*glDrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex)=NULL;
GLvoid (*glDrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount, GLint basevertex)=NULL;
GLvoid (*glMultiDrawElementsBaseVertex)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid *const*indices, GLsizei drawcount, const GLint *basevertex)=NULL;
GLvoid (*glProvokingVertex)(GLenum mode)=NULL;
GLsync (*glFenceSync)(GLenum condition, GLbitfield flags)=NULL;
GLboolean (*glIsSync)(GLsync sync)=NULL;
GLvoid (*glDeleteSync)(GLsync sync)=NULL;
GLenum (*glClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)=NULL;
GLvoid (*glWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)=NULL;
GLvoid (*glGetInteger64v)(GLenum pname, GLint64 *data)=NULL;
GLvoid (*glGetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)=NULL;
GLvoid (*glGetInteger64i_v)(GLenum target, GLuint index, GLint64 *data)=NULL;
GLvoid (*glGetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 *params)=NULL;
GLvoid (*glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level)=NULL;
GLvoid (*glTexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)=NULL;
GLvoid (*glTexImage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)=NULL;
GLvoid (*glGetMultisamplefv)(GLenum pname, GLuint index, GLfloat *val)=NULL;
GLvoid (*glSampleMaski)(GLuint maskNumber, GLbitfield mask)=NULL;

// OpenGL 3.3
GLvoid (*glBindFragDataLocationIndexed)(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)=NULL;
GLint (*glGetFragDataIndex)(GLuint program, const GLchar *name)=NULL;
GLvoid (*glGenSamplers)(GLsizei count, GLuint *samplers)=NULL;
GLvoid (*glDeleteSamplers)(GLsizei count, const GLuint *samplers)=NULL;
GLboolean (*glIsSampler)(GLuint sampler)=NULL;
GLvoid (*glBindSampler)(GLuint unit, GLuint sampler)=NULL;
GLvoid (*glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param)=NULL;
GLvoid (*glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint *param)=NULL;
GLvoid (*glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param)=NULL;
GLvoid (*glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat *param)=NULL;
GLvoid (*glSamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint *param)=NULL;
GLvoid (*glSamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint *param)=NULL;
GLvoid (*glGetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint *params)=NULL;
GLvoid (*glGetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat *params)=NULL;
GLvoid (*glGetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint *params)=NULL;
GLvoid (*glQueryCounter)(GLuint id, GLenum target)=NULL;
GLvoid (*glGetQueryObjecti64v)(GLuint id, GLenum pname, GLint64 *params)=NULL;
GLvoid (*glGetQueryObjectui64v)(GLuint id, GLenum pname, GLuint64 *params)=NULL;
GLvoid (*glVertexAttribDivisor)(GLuint index, GLuint divisor)=NULL;
GLvoid (*glVertexAttribP1ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)=NULL;
GLvoid (*glVertexAttribP1uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)=NULL;
GLvoid (*glVertexAttribP2ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)=NULL;
GLvoid (*glVertexAttribP2uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)=NULL;
GLvoid (*glVertexAttribP3ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)=NULL;
GLvoid (*glVertexAttribP3uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)=NULL;
GLvoid (*glVertexAttribP4ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)=NULL;
GLvoid (*glVertexAttribP4uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)=NULL;

// OpenGL 4.0
GLvoid (*glMinSampleShading)(GLfloat value);
GLvoid (*glBlendEquationi)(GLuint buf, GLenum mode);
GLvoid (*glBlendEquationSeparatei)(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
GLvoid (*glBlendFunci)(GLuint buf, GLenum src, GLenum dst);
GLvoid (*glBlendFuncSeparatei)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
GLvoid (*glDrawArraysIndirect)(GLenum mode, const GLvoid *indirect);
GLvoid (*glDrawElementsIndirect)(GLenum mode, GLenum type, const GLvoid *indirect);
GLvoid (*glUniform1d)(GLint location, GLdouble x);
GLvoid (*glUniform2d)(GLint location, GLdouble x, GLdouble y);
GLvoid (*glUniform3d)(GLint location, GLdouble x, GLdouble y, GLdouble z);
GLvoid (*glUniform4d)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLvoid (*glUniform1dv)(GLint location, GLsizei count, const GLdouble *value);
GLvoid (*glUniform2dv)(GLint location, GLsizei count, const GLdouble *value);
GLvoid (*glUniform3dv)(GLint location, GLsizei count, const GLdouble *value);
GLvoid (*glUniform4dv)(GLint location, GLsizei count, const GLdouble *value);
GLvoid (*glUniformMatrix2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glUniformMatrix3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glUniformMatrix4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glUniformMatrix2x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glUniformMatrix2x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glUniformMatrix3x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glUniformMatrix3x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glUniformMatrix4x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glUniformMatrix4x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glGetUniformdv)(GLuint program, GLint location, GLdouble *params);
GLint (*glGetSubroutineUniformLocation)(GLuint program, GLenum shadertype, const GLchar *name);
GLuint (*glGetSubroutineIndex)(GLuint program, GLenum shadertype, const GLchar *name);
GLvoid (*glGetActiveSubroutineUniformiv)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
GLvoid (*glGetActiveSubroutineUniformName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
GLvoid (*glGetActiveSubroutineName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
GLvoid (*glUniformSubroutinesuiv)(GLenum shadertype, GLsizei count, const GLuint *indices);
GLvoid (*glGetUniformSubroutineuiv)(GLenum shadertype, GLint location, GLuint *params);
GLvoid (*glGetProgramStageiv)(GLuint program, GLenum shadertype, GLenum pname, GLint *values);
GLvoid (*glPatchParameteri)(GLenum pname, GLint value);
GLvoid (*glPatchParameterfv)(GLenum pname, const GLfloat *values);
GLvoid (*glBindTransformFeedback)(GLenum target, GLuint id);
GLvoid (*glDeleteTransformFeedbacks)(GLsizei n, const GLuint *ids);
GLvoid (*glGenTransformFeedbacks)(GLsizei n, GLuint *ids);
GLboolean (*glIsTransformFeedback)(GLuint id);
GLvoid (*glPauseTransformFeedback)(GLvoid);
GLvoid (*glResumeTransformFeedback)(GLvoid);
GLvoid (*glDrawTransformFeedback)(GLenum mode, GLuint id);
GLvoid (*glDrawTransformFeedbackStream)(GLenum mode, GLuint id, GLuint stream);
GLvoid (*glBeginQueryIndexed)(GLenum target, GLuint index, GLuint id);
GLvoid (*glEndQueryIndexed)(GLenum target, GLuint index);
GLvoid (*glGetQueryIndexediv)(GLenum target, GLuint index, GLenum pname, GLint *params);

// OpenGL 4.1
GLvoid (*glReleaseShaderCompiler)(GLvoid);
GLvoid (*glShaderBinary)(GLsizei count, const GLuint *shaders, GLenum binaryFormat, const GLvoid *binary, GLsizei length);
GLvoid (*glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
GLvoid (*glDepthRangef)(GLfloat n, GLfloat f);
GLvoid (*glClearDepthf)(GLfloat d);
GLvoid (*glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
GLvoid (*glProgramBinary)(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length);
GLvoid (*glProgramParameteri)(GLuint program, GLenum pname, GLint value);
GLvoid (*glUseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program);
GLvoid (*glActiveShaderProgram)(GLuint pipeline, GLuint program);
GLuint (*glCreateShaderProgramv)(GLenum type, GLsizei count, const GLchar *const *strings);
GLvoid (*glBindProgramPipeline)(GLuint pipeline);
GLvoid (*glDeleteProgramPipelines)(GLsizei n, const GLuint *pipelines);
GLvoid (*glGenProgramPipelines)(GLsizei n, GLuint *pipelines);
GLboolean (*glIsProgramPipeline)(GLuint pipeline);
GLvoid (*glGetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint *params);
GLvoid (*glProgramUniform1i)(GLuint program, GLint location, GLint v0);
GLvoid (*glProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
GLvoid (*glProgramUniform1f)(GLuint program, GLint location, GLfloat v0);
GLvoid (*glProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
GLvoid (*glProgramUniform1d)(GLuint program, GLint location, GLdouble v0);
GLvoid (*glProgramUniform1dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
GLvoid (*glProgramUniform1ui)(GLuint program, GLint location, GLuint v0);
GLvoid (*glProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
GLvoid (*glProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1);
GLvoid (*glProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
GLvoid (*glProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
GLvoid (*glProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
GLvoid (*glProgramUniform2d)(GLuint program, GLint location, GLdouble v0, GLdouble v1);
GLvoid (*glProgramUniform2dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
GLvoid (*glProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1);
GLvoid (*glProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
GLvoid (*glProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
GLvoid (*glProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
GLvoid (*glProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GLvoid (*glProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
GLvoid (*glProgramUniform3d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
GLvoid (*glProgramUniform3dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
GLvoid (*glProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
GLvoid (*glProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
GLvoid (*glProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GLvoid (*glProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
GLvoid (*glProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GLvoid (*glProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
GLvoid (*glProgramUniform4d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
GLvoid (*glProgramUniform4dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
GLvoid (*glProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GLvoid (*glProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
GLvoid (*glProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid (*glProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid (*glProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid (*glProgramUniformMatrix2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glProgramUniformMatrix3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glProgramUniformMatrix4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid (*glProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid (*glProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid (*glProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid (*glProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid (*glProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid (*glProgramUniformMatrix2x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glProgramUniformMatrix3x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glProgramUniformMatrix2x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glProgramUniformMatrix4x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glProgramUniformMatrix3x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glProgramUniformMatrix4x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
GLvoid (*glValidateProgramPipeline)(GLuint pipeline);
GLvoid (*glGetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLvoid (*glVertexAttribL1d)(GLuint index, GLdouble x);
GLvoid (*glVertexAttribL2d)(GLuint index, GLdouble x, GLdouble y);
GLvoid (*glVertexAttribL3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
GLvoid (*glVertexAttribL4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLvoid (*glVertexAttribL1dv)(GLuint index, const GLdouble *v);
GLvoid (*glVertexAttribL2dv)(GLuint index, const GLdouble *v);
GLvoid (*glVertexAttribL3dv)(GLuint index, const GLdouble *v);
GLvoid (*glVertexAttribL4dv)(GLuint index, const GLdouble *v);
GLvoid (*glVertexAttribLPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLvoid (*glGetVertexAttribLdv)(GLuint index, GLenum pname, GLdouble *params);
GLvoid (*glViewportArrayv)(GLuint first, GLsizei count, const GLfloat *v);
GLvoid (*glViewportIndexedf)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
GLvoid (*glViewportIndexedfv)(GLuint index, const GLfloat *v);
GLvoid (*glScissorArrayv)(GLuint first, GLsizei count, const GLint *v);
GLvoid (*glScissorIndexed)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
GLvoid (*glScissorIndexedv)(GLuint index, const GLint *v);
GLvoid (*glDepthRangeArrayv)(GLuint first, GLsizei count, const GLdouble *v);
GLvoid (*glDepthRangeIndexed)(GLuint index, GLdouble n, GLdouble f);
GLvoid (*glGetFloati_v)(GLenum target, GLuint index, GLfloat *data);
GLvoid (*glGetDoublei_v)(GLenum target, GLuint index, GLdouble *data);

// OpenGL 4.2
GLvoid (*glDrawArraysInstancedBaseInstance)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
GLvoid (*glDrawElementsInstancedBaseInstance)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount, GLuint baseinstance);
GLvoid (*glDrawElementsInstancedBaseVertexBaseInstance)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
GLvoid (*glGetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params);
GLvoid (*glGetActiveAtomicCounterBufferiv)(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
GLvoid (*glBindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
GLvoid (*glMemoryBarrier)(GLbitfield barriers);
GLvoid (*glTexStorage1D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
GLvoid (*glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GLvoid (*glTexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
GLvoid (*glDrawTransformFeedbackInstanced)(GLenum mode, GLuint id, GLsizei instancecount);
GLvoid (*glDrawTransformFeedbackStreamInstanced)(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
typedef GLvoid(APIENTRY *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);

// OpenGL 4.3
GLvoid (*glClearBufferData)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const GLvoid *data);
GLvoid (*glClearBufferSubData)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid *data);
GLvoid (*glDispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
GLvoid (*glDispatchComputeIndirect)(GLintptr indirect);
GLvoid (*glCopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
GLvoid (*glFramebufferParameteri)(GLenum target, GLenum pname, GLint param);
GLvoid (*glGetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params);
GLvoid (*glGetInternalformati64v)(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64 *params);
GLvoid (*glInvalidateTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
GLvoid (*glInvalidateTexImage)(GLuint texture, GLint level);
GLvoid (*glInvalidateBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr length);
GLvoid (*glInvalidateBufferData)(GLuint buffer);
GLvoid (*glInvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
GLvoid (*glInvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
GLvoid (*glMultiDrawArraysIndirect)(GLenum mode, const GLvoid *indirect, GLsizei drawcount, GLsizei stride);
GLvoid (*glMultiDrawElementsIndirect)(GLenum mode, GLenum type, const GLvoid *indirect, GLsizei drawcount, GLsizei stride);
GLvoid (*glGetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint *params);
GLuint (*glGetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name);
GLvoid (*glGetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
GLvoid (*glGetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params);
GLint (*glGetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name);
GLint (*glGetProgramResourceLocationIndex)(GLuint program, GLenum programInterface, const GLchar *name);
GLvoid (*glShaderStorageBlockBinding)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
GLvoid (*glTexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
GLvoid (*glTexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
GLvoid (*glTexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
GLvoid (*glTextureView)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
GLvoid (*glBindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
GLvoid (*glVertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
GLvoid (*glVertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
GLvoid (*glVertexAttribLFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
GLvoid (*glVertexAttribBinding)(GLuint attribindex, GLuint bindingindex);
GLvoid (*glVertexBindingDivisor)(GLuint bindingindex, GLuint divisor);
GLvoid (*glDebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
GLvoid (*glDebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
GLvoid (*glDebugMessageCallback)(GLDEBUGPROC callback, const GLvoid *userParam);
GLuint (*glGetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
GLvoid (*glPushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar *message);
GLvoid (*glPopDebugGroup)(GLvoid);
GLvoid (*glObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
GLvoid (*glGetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
GLvoid (*glObjectPtrLabel)(const GLvoid *ptr, GLsizei length, const GLchar *label);
GLvoid (*glGetObjectPtrLabel)(const GLvoid *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);

// OpenGL 4.4
GLvoid (*glBufferStorage)(GLenum target, GLsizeiptr size, const GLvoid *data, GLbitfield flags);
GLvoid (*glClearTexImage)(GLuint texture, GLint level, GLenum format, GLenum type, const GLvoid *data);
GLvoid (*glClearTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *data);
GLvoid (*glBindBuffersBase)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
GLvoid (*glBindBuffersRange)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes);
GLvoid (*glBindTextures)(GLuint first, GLsizei count, const GLuint *textures);
GLvoid (*glBindSamplers)(GLuint first, GLsizei count, const GLuint *samplers);
GLvoid (*glBindImageTextures)(GLuint first, GLsizei count, const GLuint *textures);
GLvoid (*glBindVertexBuffers)(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);

// OpenGL 4.5
GLvoid (*glClipControl)(GLenum origin, GLenum depth);
GLvoid (*glCreateTransformFeedbacks)(GLsizei n, GLuint *ids);
GLvoid (*glTransformFeedbackBufferBase)(GLuint xfb, GLuint index, GLuint buffer);
GLvoid (*glTransformFeedbackBufferRange)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
GLvoid (*glGetTransformFeedbackiv)(GLuint xfb, GLenum pname, GLint *param);
GLvoid (*glGetTransformFeedbacki_v)(GLuint xfb, GLenum pname, GLuint index, GLint *param);
GLvoid (*glGetTransformFeedbacki64_v)(GLuint xfb, GLenum pname, GLuint index, GLint64 *param);
GLvoid (*glCreateBuffers)(GLsizei n, GLuint *buffers);
GLvoid (*glNamedBufferStorage)(GLuint buffer, GLsizeiptr size, const GLvoid *data, GLbitfield flags);
GLvoid (*glNamedBufferData)(GLuint buffer, GLsizeiptr size, const GLvoid *data, GLenum usage);
GLvoid (*glNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid *data);
GLvoid (*glCopyNamedBufferSubData)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
GLvoid (*glClearNamedBufferData)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const GLvoid *data);
GLvoid (*glClearNamedBufferSubData)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid *data);
GLvoid *(*glMapNamedBuffer)(GLuint buffer, GLenum access);
GLvoid *(*glMapNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
GLboolean (*glUnmapNamedBuffer)(GLuint buffer);
GLvoid (*glFlushMappedNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length);
GLvoid (*glGetNamedBufferParameteriv)(GLuint buffer, GLenum pname, GLint *params);
GLvoid (*glGetNamedBufferParameteri64v)(GLuint buffer, GLenum pname, GLint64 *params);
GLvoid (*glGetNamedBufferPointerv)(GLuint buffer, GLenum pname, GLvoid **params);
GLvoid (*glGetNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid *data);
GLvoid (*glCreateFramebuffers)(GLsizei n, GLuint *framebuffers);
GLvoid (*glNamedFramebufferRenderbuffer)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GLvoid (*glNamedFramebufferParameteri)(GLuint framebuffer, GLenum pname, GLint param);
GLvoid (*glNamedFramebufferTexture)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
GLvoid (*glNamedFramebufferTextureLayer)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
GLvoid (*glNamedFramebufferDrawBuffer)(GLuint framebuffer, GLenum buf);
GLvoid (*glNamedFramebufferDrawBuffers)(GLuint framebuffer, GLsizei n, const GLenum *bufs);
GLvoid (*glNamedFramebufferReadBuffer)(GLuint framebuffer, GLenum src);
GLvoid (*glInvalidateNamedFramebufferData)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments);
GLvoid (*glInvalidateNamedFramebufferSubData)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
GLvoid (*glClearNamedFramebufferiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
GLvoid (*glClearNamedFramebufferuiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
GLvoid (*glClearNamedFramebufferfv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
GLvoid (*glClearNamedFramebufferfi)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
GLvoid (*glBlitNamedFramebuffer)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
GLenum (*glCheckNamedFramebufferStatus)(GLuint framebuffer, GLenum target);
GLvoid (*glGetNamedFramebufferParameteriv)(GLuint framebuffer, GLenum pname, GLint *param);
GLvoid (*glGetNamedFramebufferAttachmentParameteriv)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
GLvoid (*glCreateRenderbuffers)(GLsizei n, GLuint *renderbuffers);
GLvoid (*glNamedRenderbufferStorage)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
GLvoid (*glNamedRenderbufferStorageMultisample)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GLvoid (*glGetNamedRenderbufferParameteriv)(GLuint renderbuffer, GLenum pname, GLint *params);
GLvoid (*glCreateTextures)(GLenum target, GLsizei n, GLuint *textures);
GLvoid (*glTextureBuffer)(GLuint texture, GLenum internalformat, GLuint buffer);
GLvoid (*glTextureBufferRange)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
GLvoid (*glTextureStorage1D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
GLvoid (*glTextureStorage2D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GLvoid (*glTextureStorage3D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
GLvoid (*glTextureStorage2DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
GLvoid (*glTextureStorage3DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
GLvoid (*glTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
GLvoid (*glTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GLvoid (*glTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
GLvoid (*glCompressedTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
GLvoid (*glCompressedTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
GLvoid (*glCompressedTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
GLvoid (*glCopyTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GLvoid (*glCopyTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GLvoid (*glCopyTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GLvoid (*glTextureParameterf)(GLuint texture, GLenum pname, GLfloat param);
GLvoid (*glTextureParameterfv)(GLuint texture, GLenum pname, const GLfloat *param);
GLvoid (*glTextureParameteri)(GLuint texture, GLenum pname, GLint param);
GLvoid (*glTextureParameterIiv)(GLuint texture, GLenum pname, const GLint *params);
GLvoid (*glTextureParameterIuiv)(GLuint texture, GLenum pname, const GLuint *params);
GLvoid (*glTextureParameteriv)(GLuint texture, GLenum pname, const GLint *param);
GLvoid (*glGenerateTextureMipmap)(GLuint texture);
GLvoid (*glBindTextureUnit)(GLuint unit, GLuint texture);
GLvoid (*glGetTextureImage)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid *pixels);
GLvoid (*glGetCompressedTextureImage)(GLuint texture, GLint level, GLsizei bufSize, GLvoid *pixels);
GLvoid (*glGetTextureLevelParameterfv)(GLuint texture, GLint level, GLenum pname, GLfloat *params);
GLvoid (*glGetTextureLevelParameteriv)(GLuint texture, GLint level, GLenum pname, GLint *params);
GLvoid (*glGetTextureParameterfv)(GLuint texture, GLenum pname, GLfloat *params);
GLvoid (*glGetTextureParameterIiv)(GLuint texture, GLenum pname, GLint *params);
GLvoid (*glGetTextureParameterIuiv)(GLuint texture, GLenum pname, GLuint *params);
GLvoid (*glGetTextureParameteriv)(GLuint texture, GLenum pname, GLint *params);
GLvoid (*glCreateVertexArrays)(GLsizei n, GLuint *arrays);
GLvoid (*glDisableVertexArrayAttrib)(GLuint vaobj, GLuint index);
GLvoid (*glEnableVertexArrayAttrib)(GLuint vaobj, GLuint index);
GLvoid (*glVertexArrayElementBuffer)(GLuint vaobj, GLuint buffer);
GLvoid (*glVertexArrayVertexBuffer)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
GLvoid (*glVertexArrayVertexBuffers)(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
GLvoid (*glVertexArrayAttribBinding)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
GLvoid (*glVertexArrayAttribFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
GLvoid (*glVertexArrayAttribIFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
GLvoid (*glVertexArrayAttribLFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
GLvoid (*glVertexArrayBindingDivisor)(GLuint vaobj, GLuint bindingindex, GLuint divisor);
GLvoid (*glGetVertexArrayiv)(GLuint vaobj, GLenum pname, GLint *param);
GLvoid (*glGetVertexArrayIndexediv)(GLuint vaobj, GLuint index, GLenum pname, GLint *param);
GLvoid (*glGetVertexArrayIndexed64iv)(GLuint vaobj, GLuint index, GLenum pname, GLint64 *param);
GLvoid (*glCreateSamplers)(GLsizei n, GLuint *samplers);
GLvoid (*glCreateProgramPipelines)(GLsizei n, GLuint *pipelines);
GLvoid (*glCreateQueries)(GLenum target, GLsizei n, GLuint *ids);
GLvoid (*glGetQueryBufferObjecti64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
GLvoid (*glGetQueryBufferObjectiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
GLvoid (*glGetQueryBufferObjectui64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
GLvoid (*glGetQueryBufferObjectuiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
GLvoid (*glMemoryBarrierByRegion)(GLbitfield barriers);
GLvoid (*glGetTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, GLvoid *pixels);
GLvoid (*glGetCompressedTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, GLvoid *pixels);
GLenum (*glGetGraphicsResetStatus)(GLvoid);
GLvoid (*glGetnCompressedTexImage)(GLenum target, GLint lod, GLsizei bufSize, GLvoid *pixels);
GLvoid (*glGetnTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid *pixels);
GLvoid (*glGetnUniformdv)(GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
GLvoid (*glGetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
GLvoid (*glGetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint *params);
GLvoid (*glGetnUniformuiv)(GLuint program, GLint location, GLsizei bufSize, GLuint *params);
GLvoid (*glReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data);
GLvoid (*glTextureBarrier)(GLvoid);

// OpenGL 4.6
GLvoid (*glSpecializeShader)(GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue);
GLvoid (*glMultiDrawArraysIndirectCount)(GLenum mode, const GLvoid *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
GLvoid (*glMultiDrawElementsIndirectCount)(GLenum mode, GLenum type, const GLvoid *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
GLvoid (*glPolygonOffsetClamp)(GLfloat factor, GLfloat units, GLfloat clamp);

int GLExtensionsInitialized_Flag=0;

#ifdef WIN32
int WGLExtensionsInitialized_Flag=0;
#else
int GLXExtensionsInitialized_Flag=0;
#endif

void *glGetProcAddress(char *Name, int *Return)
{
	void *FuncPtr=NULL;

#ifdef WIN32
	FuncPtr=(void *)wglGetProcAddress(Name);
#else
	FuncPtr=(void *)glXGetProcAddressARB(Name);
#endif

	if(FuncPtr!=NULL)
		*Return&=1;
	else
		*Return&=0;

	return FuncPtr;
}

int FindSubString(char *string, char *substring)
{
	if(string)
	{
		char *str=string;
		size_t len=strlen(substring);

		while((str=strstr(str, substring))!=NULL)
		{
			str+=len;

			if(*str==' '||*str=='\0')
				return 1;
		}
	}

	return 0;
}

void GLSetupExtensions(void)
{
	int Return=1;
	char *GLExtensionString=NULL;

	// GL Extensions
	GLExtensionString=(char *)glGetString(GL_EXTENSIONS);
	
// Don't get procdure address for functions already in the library
/*	glCullFace=glGetProcAddress("glCullFace", &Return);
	glFrontFace=glGetProcAddress("glFrontFace", &Return);
	glHint=glGetProcAddress("glHint", &Return);
	glLineWidth=glGetProcAddress("glLineWidth", &Return);
	glPointSize=glGetProcAddress("glPointSize", &Return);
	glPolygonMode=glGetProcAddress("glPolygonMode", &Return);
	glScissor=glGetProcAddress("glScissor", &Return);
	glTexParameterf=glGetProcAddress("glTexParameterf", &Return);
	glTexParameterfv=glGetProcAddress("glTexParameterfv", &Return);
	glTexParameteri=glGetProcAddress("glTexParameteri", &Return);
	glTexParameteriv=glGetProcAddress("glTexParameteriv", &Return);
	glTexImage1D=glGetProcAddress("glTexImage1D", &Return);
	glTexImage2D=glGetProcAddress("glTexImage2D", &Return);
	glDrawBuffer=glGetProcAddress("glDrawBuffer", &Return);
	glClear=glGetProcAddress("glClear", &Return);
	glClearColor=glGetProcAddress("glClearColor", &Return);
	glClearStencil=glGetProcAddress("glClearStencil", &Return);
	glClearDepth=glGetProcAddress("glClearDepth", &Return);
	glStencilMask=glGetProcAddress("glStencilMask", &Return);
	glColorMask=glGetProcAddress("glColorMask", &Return);
	glDepthMask=glGetProcAddress("glDepthMask", &Return);
	glDisable=glGetProcAddress("glDisable", &Return);
	glEnable=glGetProcAddress("glEnable", &Return);
	glFinish=glGetProcAddress("glFinish", &Return);
	glFlush=glGetProcAddress("glFlush", &Return);
	glBlendFunc=glGetProcAddress("glBlendFunc", &Return);
	glLogicOp=glGetProcAddress("glLogicOp", &Return);
	glStencilFunc=glGetProcAddress("glStencilFunc", &Return);
	glStencilOp=glGetProcAddress("glStencilOp", &Return);
	glDepthFunc=glGetProcAddress("glDepthFunc", &Return);
	glPixelStoref=glGetProcAddress("glPixelStoref", &Return);
	glPixelStorei=glGetProcAddress("glPixelStorei", &Return);
	glReadBuffer=glGetProcAddress("glReadBuffer", &Return);
	glReadPixels=glGetProcAddress("glReadPixels", &Return);
	glGetBooleanv=glGetProcAddress("glGetBooleanv", &Return);
	glGetDoublev=glGetProcAddress("glGetDoublev", &Return);
	glGetError=glGetProcAddress("glGetError", &Return);
	glGetFloatv=glGetProcAddress("glGetFloatv", &Return);
	glGetIntegerv=glGetProcAddress("glGetIntegerv", &Return);
	glGetString=glGetProcAddress("glGetString", &Return);
	glGetTexImage=glGetProcAddress("glGetTexImage", &Return);
	glGetTexParameterfv=glGetProcAddress("glGetTexParameterfv", &Return);
	glGetTexParameteriv=glGetProcAddress("glGetTexParameteriv", &Return);
	glGetTexLevelParameterfv=glGetProcAddress("glGetTexLevelParameterfv", &Return);
	glGetTexLevelParameteriv=glGetProcAddress("glGetTexLevelParameteriv", &Return);
	glIsEnabled=glGetProcAddress("glIsEnabled", &Return);
	glDepthRange=glGetProcAddress("glDepthRange", &Return);
	glViewport=glGetProcAddress("glViewport", &Return);
	glDrawArrays=glGetProcAddress("glDrawArrays", &Return);
	glDrawElements=glGetProcAddress("glDrawElements", &Return);
	glGetPointerv=glGetProcAddress("glGetPointerv", &Return);
	glPolygonOffset=glGetProcAddress("glPolygonOffset", &Return);
	glCopyTexImage1D=glGetProcAddress("glCopyTexImage1D", &Return);
	glCopyTexImage2D=glGetProcAddress("glCopyTexImage2D", &Return);
	glCopyTexSubImage1D=glGetProcAddress("glCopyTexSubImage1D", &Return);
	glCopyTexSubImage2D=glGetProcAddress("glCopyTexSubImage2D", &Return);
	glTexSubImage1D=glGetProcAddress("glTexSubImage1D", &Return);
	glTexSubImage2D=glGetProcAddress("glTexSubImage2D", &Return);
	glBindTexture=glGetProcAddress("glBindTexture", &Return);
	glDeleteTextures=glGetProcAddress("glDeleteTextures", &Return);
	glGenTextures=glGetProcAddress("glGenTextures", &Return);
	glIsTexture=glGetProcAddress("glIsTexture", &Return);*/
#ifdef WIN32 // Linux has up to 2.1, Windows does not.
	glDrawRangeElements=glGetProcAddress("glDrawRangeElements", &Return);
	glTexImage3D=glGetProcAddress("glTexImage3D", &Return);
	glTexSubImage3D=glGetProcAddress("glTexSubImage3D", &Return);
	glCopyTexSubImage3D=glGetProcAddress("glCopyTexSubImage3D", &Return);
	glActiveTexture=glGetProcAddress("glActiveTexture", &Return);
	glSampleCoverage=glGetProcAddress("glSampleCoverage", &Return);
	glCompressedTexImage3D=glGetProcAddress("glCompressedTexImage3D", &Return);
	glCompressedTexImage2D=glGetProcAddress("glCompressedTexImage2D", &Return);
	glCompressedTexImage1D=glGetProcAddress("glCompressedTexImage1D", &Return);
	glCompressedTexSubImage3D=glGetProcAddress("glCompressedTexSubImage3D", &Return);
	glCompressedTexSubImage2D=glGetProcAddress("glCompressedTexSubImage2D", &Return);
	glCompressedTexSubImage1D=glGetProcAddress("glCompressedTexSubImage1D", &Return);
	glGetCompressedTexImage=glGetProcAddress("glGetCompressedTexImage", &Return);
	glBlendFuncSeparate=glGetProcAddress("glBlendFuncSeparate", &Return);
	glMultiDrawArrays=glGetProcAddress("glMultiDrawArrays", &Return);
	glMultiDrawElements=glGetProcAddress("glMultiDrawElements", &Return);
	glPointParameterf=glGetProcAddress("glPointParameterf", &Return);
	glPointParameterfv=glGetProcAddress("glPointParameterfv", &Return);
	glPointParameteri=glGetProcAddress("glPointParameteri", &Return);
	glPointParameteriv=glGetProcAddress("glPointParameteriv", &Return);
	glBlendColor=glGetProcAddress("glBlendColor", &Return);
	glBlendEquation=glGetProcAddress("glBlendEquation", &Return);
#endif
	glGenQueries=glGetProcAddress("glGenQueries", &Return);
	glDeleteQueries=glGetProcAddress("glDeleteQueries", &Return);
	glIsQuery=glGetProcAddress("glIsQuery", &Return);
	glBeginQuery=glGetProcAddress("glIsQuery", &Return);
	glEndQuery=glGetProcAddress("glEndQuery", &Return);
	glGetQueryiv=glGetProcAddress("glGetQueryiv", &Return);
	glGetQueryObjectiv=glGetProcAddress("glGetQueryObjectiv", &Return);
	glGetQueryObjectuiv=glGetProcAddress("glGetQueryObjectuiv", &Return);
	glBindBuffer=glGetProcAddress("glBindBuffer", &Return);
	glDeleteBuffers=glGetProcAddress("glDeleteBuffers", &Return);
	glGenBuffers=glGetProcAddress("glGenBuffers", &Return);
	glIsBuffer=glGetProcAddress("glIsBuffer", &Return);
	glBufferData=glGetProcAddress("glBufferData", &Return);
	glBufferSubData=glGetProcAddress("glBufferSubData", &Return);
	glGetBufferSubData=glGetProcAddress("glGetBufferSubData", &Return);
	glMapBuffer=glGetProcAddress("glMapBuffer", &Return);
	glUnmapBuffer=glGetProcAddress("glUnmapBuffer", &Return);
	glGetBufferParameteriv=glGetProcAddress("glGetBufferParameteriv", &Return);
	glGetBufferPointerv=glGetProcAddress("glGetBufferPointerv", &Return);
	glBlendEquationSeparate=glGetProcAddress("glBlendEquationSeparate", &Return);
	glDrawBuffers=glGetProcAddress("glDrawBuffers", &Return);
	glStencilOpSeparate=glGetProcAddress("glStencilOpSeparate", &Return);
	glStencilFuncSeparate=glGetProcAddress("glStencilFuncSeparate", &Return);
	glStencilMaskSeparate=glGetProcAddress("glStencilMaskSeparate", &Return);
	glAttachShader=glGetProcAddress("glAttachShader", &Return);
	glBindAttribLocation=glGetProcAddress("glBindAttribLocation", &Return);
	glCompileShader=glGetProcAddress("glCompileShader", &Return);
	glCreateProgram=glGetProcAddress("glCreateProgram", &Return);
	glCreateShader=glGetProcAddress("glCreateShader", &Return);
	glDeleteProgram=glGetProcAddress("glDeleteProgram", &Return);
	glDeleteShader=glGetProcAddress("glDeleteShader", &Return);
	glDetachShader=glGetProcAddress("glDetachShader", &Return);
	glDisableVertexAttribArray=glGetProcAddress("glDisableVertexAttribArray", &Return);
	glEnableVertexAttribArray=glGetProcAddress("glEnableVertexAttribArray", &Return);
	glGetActiveAttrib=glGetProcAddress("glGetActiveAttrib", &Return);
	glGetActiveUniform=glGetProcAddress("glGetActiveUniform", &Return);
	glGetAttachedShaders=glGetProcAddress("glGetAttachedShaders", &Return);
	glGetAttribLocation=glGetProcAddress("glGetAttribLocation", &Return);
	glGetProgramiv=glGetProcAddress("glGetProgramiv", &Return);
	glGetProgramInfoLog=glGetProcAddress("glGetProgramInfoLog", &Return);
	glGetShaderiv=glGetProcAddress("glGetShaderiv", &Return);
	glGetShaderInfoLog=glGetProcAddress("glGetShaderInfoLog", &Return);
	glGetShaderSource=glGetProcAddress("glGetShaderSource", &Return);
	glGetUniformLocation=glGetProcAddress("glGetUniformLocation", &Return);
	glGetUniformfv=glGetProcAddress("glGetUniformfv", &Return);
	glGetUniformiv=glGetProcAddress("glGetUniformiv", &Return);
	glGetVertexAttribdv=glGetProcAddress("glGetVertexAttribdv", &Return);
	glGetVertexAttribfv=glGetProcAddress("glGetVertexAttribfv", &Return);
	glGetVertexAttribiv=glGetProcAddress("glGetVertexAttribiv", &Return);
	glGetVertexAttribPointerv=glGetProcAddress("glGetVertexAttribPointerv", &Return);
	glIsProgram=glGetProcAddress("glIsProgram", &Return);
	glIsShader=glGetProcAddress("glIsShader", &Return);
	glLinkProgram=glGetProcAddress("glLinkProgram", &Return);
	glShaderSource=glGetProcAddress("glShaderSource", &Return);
	glUseProgram=glGetProcAddress("glUseProgram", &Return);
	glUniform1f=glGetProcAddress("glUniform1f", &Return);
	glUniform2f=glGetProcAddress("glUniform2f", &Return);
	glUniform3f=glGetProcAddress("glUniform3f", &Return);
	glUniform4f=glGetProcAddress("glUniform4f", &Return);
	glUniform1i=glGetProcAddress("glUniform1i", &Return);
	glUniform2i=glGetProcAddress("glUniform2i", &Return);
	glUniform3i=glGetProcAddress("glUniform3i", &Return);
	glUniform4i=glGetProcAddress("glUniform4i", &Return);
	glUniform1fv=glGetProcAddress("glUniform1fv", &Return);
	glUniform2fv=glGetProcAddress("glUniform2fv", &Return);
	glUniform3fv=glGetProcAddress("glUniform3fv", &Return);
	glUniform4fv=glGetProcAddress("glUniform4fv", &Return);
	glUniform1iv=glGetProcAddress("glUniform1iv", &Return);
	glUniform2iv=glGetProcAddress("glUniform2iv", &Return);
	glUniform3iv=glGetProcAddress("glUniform3iv", &Return);
	glUniform4iv=glGetProcAddress("glUniform4iv", &Return);
	glUniformMatrix2fv=glGetProcAddress("glUniformMatrix2fv", &Return);
	glUniformMatrix3fv=glGetProcAddress("glUniformMatrix3fv", &Return);
	glUniformMatrix4fv=glGetProcAddress("glUniformMatrix4fv", &Return);
	glValidateProgram=glGetProcAddress("glValidateProgram", &Return);
	glVertexAttrib1d=glGetProcAddress("glVertexAttrib1d", &Return);
	glVertexAttrib1dv=glGetProcAddress("glVertexAttrib1dv", &Return);
	glVertexAttrib1f=glGetProcAddress("glVertexAttrib1f", &Return);
	glVertexAttrib1fv=glGetProcAddress("glVertexAttrib1fv", &Return);
	glVertexAttrib1s=glGetProcAddress("glVertexAttrib1s", &Return);
	glVertexAttrib1sv=glGetProcAddress("glVertexAttrib1sv", &Return);
	glVertexAttrib2d=glGetProcAddress("glVertexAttrib2d", &Return);
	glVertexAttrib2dv=glGetProcAddress("glVertexAttrib2dv", &Return);
	glVertexAttrib2f=glGetProcAddress("glVertexAttrib2f", &Return);
	glVertexAttrib2fv=glGetProcAddress("glVertexAttrib2fv", &Return);
	glVertexAttrib2s=glGetProcAddress("glVertexAttrib2s", &Return);
	glVertexAttrib2sv=glGetProcAddress("glVertexAttrib2sv", &Return);
	glVertexAttrib3d=glGetProcAddress("glVertexAttrib3d", &Return);
	glVertexAttrib3dv=glGetProcAddress("glVertexAttrib3dv", &Return);
	glVertexAttrib3f=glGetProcAddress("glVertexAttrib3f", &Return);
	glVertexAttrib3fv=glGetProcAddress("glVertexAttrib3fv", &Return);
	glVertexAttrib3s=glGetProcAddress("glVertexAttrib3s", &Return);
	glVertexAttrib3sv=glGetProcAddress("glVertexAttrib3sv", &Return);
	glVertexAttrib4Nbv=glGetProcAddress("glVertexAttrib4Nbv", &Return);
	glVertexAttrib4Niv=glGetProcAddress("glVertexAttrib4Niv", &Return);
	glVertexAttrib4Nsv=glGetProcAddress("glVertexAttrib4Nsv", &Return);
	glVertexAttrib4Nub=glGetProcAddress("glVertexAttrib4Nub", &Return);
	glVertexAttrib4Nubv=glGetProcAddress("glVertexAttrib4Nubv", &Return);
	glVertexAttrib4Nuiv=glGetProcAddress("glVertexAttrib4Nuiv", &Return);
	glVertexAttrib4Nusv=glGetProcAddress("glVertexAttrib4Nusv", &Return);
	glVertexAttrib4bv=glGetProcAddress("glVertexAttrib4bv", &Return);
	glVertexAttrib4d=glGetProcAddress("glVertexAttrib4d", &Return);
	glVertexAttrib4dv=glGetProcAddress("glVertexAttrib4dv", &Return);
	glVertexAttrib4f=glGetProcAddress("glVertexAttrib4f", &Return);
	glVertexAttrib4fv=glGetProcAddress("glVertexAttrib4fv", &Return);
	glVertexAttrib4iv=glGetProcAddress("glVertexAttrib4iv", &Return);
	glVertexAttrib4s=glGetProcAddress("glVertexAttrib4s", &Return);
	glVertexAttrib4sv=glGetProcAddress("glVertexAttrib4sv", &Return);
	glVertexAttrib4ubv=glGetProcAddress("glVertexAttrib4ubv", &Return);
	glVertexAttrib4uiv=glGetProcAddress("glVertexAttrib4uiv", &Return);
	glVertexAttrib4usv=glGetProcAddress("glVertexAttrib4usv", &Return);
	glVertexAttribPointer=glGetProcAddress("glVertexAttribPointer", &Return);
	glUniformMatrix2x3fv=glGetProcAddress("glUniformMatrix2x3fv", &Return);
	glUniformMatrix3x2fv=glGetProcAddress("glUniformMatrix3x2fv", &Return);
	glUniformMatrix2x4fv=glGetProcAddress("glUniformMatrix2x4fv", &Return);
	glUniformMatrix4x2fv=glGetProcAddress("glUniformMatrix4x2fv", &Return);
	glUniformMatrix3x4fv=glGetProcAddress("glUniformMatrix3x4fv", &Return);
	glUniformMatrix4x3fv=glGetProcAddress("glUniformMatrix4x3fv", &Return);

	// Something went horribly wrong, or incomplete GL2.1 support
	if(!Return)
		return;

	glColorMaski=glGetProcAddress("glColorMaski", &Return);
	glGetBooleani_v=glGetProcAddress("glGetBooleani_v", &Return);
	glGetIntegeri_v=glGetProcAddress("glGetIntegeri_v", &Return);
	glEnablei=glGetProcAddress("glEnablei", &Return);
	glDisablei=glGetProcAddress("glDisablei", &Return);
	glIsEnabledi=glGetProcAddress("glIsEnabledi", &Return);
	glBeginTransformFeedback=glGetProcAddress("glBeginTransformFeedback", &Return);
	glEndTransformFeedback=glGetProcAddress("glEndTransformFeedback", &Return);
	glBindBufferRange=glGetProcAddress("glBindBufferRange", &Return);
	glBindBufferBase=glGetProcAddress("glBindBufferBase", &Return);
	glTransformFeedbackVaryings=glGetProcAddress("glTransformFeedbackVaryings", &Return);
	glGetTransformFeedbackVarying=glGetProcAddress("glGetTransformFeedbackVarying", &Return);
	glClampColor=glGetProcAddress("glClampColor", &Return);
	glBeginConditionalRender=glGetProcAddress("glBeginConditionalRender", &Return);
	glEndConditionalRender=glGetProcAddress("glEndConditionalRender", &Return);
	glVertexAttribIPointer=glGetProcAddress("glVertexAttribIPointer", &Return);
	glGetVertexAttribIiv=glGetProcAddress("glGetVertexAttribIiv", &Return);
	glGetVertexAttribIuiv=glGetProcAddress("glGetVertexAttribIuiv", &Return);
	glVertexAttribI1i=glGetProcAddress("glVertexAttribI1i", &Return);
	glVertexAttribI2i=glGetProcAddress("glVertexAttribI2i", &Return);
	glVertexAttribI3i=glGetProcAddress("glVertexAttribI3i", &Return);
	glVertexAttribI4i=glGetProcAddress("glVertexAttribI4i", &Return);
	glVertexAttribI1ui=glGetProcAddress("glVertexAttribI1ui", &Return);
	glVertexAttribI2ui=glGetProcAddress("glVertexAttribI2ui", &Return);
	glVertexAttribI3ui=glGetProcAddress("glVertexAttribI3ui", &Return);
	glVertexAttribI4ui=glGetProcAddress("glVertexAttribI4ui", &Return);
	glVertexAttribI1iv=glGetProcAddress("glVertexAttribI1iv", &Return);
	glVertexAttribI2iv=glGetProcAddress("glVertexAttribI2iv", &Return);
	glVertexAttribI3iv=glGetProcAddress("glVertexAttribI3iv", &Return);
	glVertexAttribI4iv=glGetProcAddress("glVertexAttribI4iv", &Return);
	glVertexAttribI1uiv=glGetProcAddress("glVertexAttribI1uiv", &Return);
	glVertexAttribI2uiv=glGetProcAddress("glVertexAttribI2uiv", &Return);
	glVertexAttribI3uiv=glGetProcAddress("glVertexAttribI3uiv", &Return);
	glVertexAttribI4uiv=glGetProcAddress("glVertexAttribI4uiv", &Return);
	glVertexAttribI4bv=glGetProcAddress("glVertexAttribI4bv", &Return);
	glVertexAttribI4sv=glGetProcAddress("glVertexAttribI4sv", &Return);
	glVertexAttribI4ubv=glGetProcAddress("glVertexAttribI4ubv", &Return);
	glVertexAttribI4usv=glGetProcAddress("glVertexAttribI4usv", &Return);
	glGetUniformuiv=glGetProcAddress("glGetUniformuiv", &Return);
	glBindFragDataLocation=glGetProcAddress("glBindFragDataLocation", &Return);
	glGetFragDataLocation=glGetProcAddress("glGetFragDataLocation", &Return);
	glUniform1ui=glGetProcAddress("glUniform1ui", &Return);
	glUniform2ui=glGetProcAddress("glUniform2ui", &Return);
	glUniform3ui=glGetProcAddress("glUniform3ui", &Return);
	glUniform4ui=glGetProcAddress("glUniform4ui", &Return);
	glUniform1uiv=glGetProcAddress("glUniform1uiv", &Return);
	glUniform2uiv=glGetProcAddress("glUniform2uiv", &Return);
	glUniform3uiv=glGetProcAddress("glUniform3uiv", &Return);
	glUniform4uiv=glGetProcAddress("glUniform4uiv", &Return);
	glTexParameterIiv=glGetProcAddress("glTexParameterIiv", &Return);
	glTexParameterIuiv=glGetProcAddress("glTexParameterIuiv", &Return);
	glGetTexParameterIiv=glGetProcAddress("glGetTexParameterIiv", &Return);
	glGetTexParameterIuiv=glGetProcAddress("glGetTexParameterIuiv", &Return);
	glClearBufferiv=glGetProcAddress("glClearBufferiv", &Return);
	glClearBufferuiv=glGetProcAddress("glClearBufferuiv", &Return);
	glClearBufferfv=glGetProcAddress("glClearBufferfv", &Return);
	glClearBufferfi=glGetProcAddress("glClearBufferfi", &Return);
	glGetStringi=glGetProcAddress("glGetStringi", &Return);
	glIsRenderbuffer=glGetProcAddress("glIsRenderbuffer", &Return);
	glBindRenderbuffer=glGetProcAddress("glBindRenderbuffer", &Return);
	glDeleteRenderbuffers=glGetProcAddress("glDeleteRenderbuffers", &Return);
	glGenRenderbuffers=glGetProcAddress("glGenRenderbuffers", &Return);
	glRenderbufferStorage=glGetProcAddress("glRenderbufferStorage", &Return);
	glGetRenderbufferParameteriv=glGetProcAddress("glGetRenderbufferParameteriv", &Return);
	glIsFramebuffer=glGetProcAddress("glIsFramebuffer", &Return);
	glBindFramebuffer=glGetProcAddress("glBindFramebuffer", &Return);
	glDeleteFramebuffers=glGetProcAddress("glDeleteFramebuffers", &Return);
	glGenFramebuffers=glGetProcAddress("glGenFramebuffers", &Return);
	glCheckFramebufferStatus=glGetProcAddress("glCheckFramebufferStatus", &Return);
	glFramebufferTexture1D=glGetProcAddress("glFramebufferTexture1D", &Return);
	glFramebufferTexture2D=glGetProcAddress("glFramebufferTexture2D", &Return);
	glFramebufferTexture3D=glGetProcAddress("glFramebufferTexture3D", &Return);
	glFramebufferRenderbuffer=glGetProcAddress("glFramebufferRenderbuffer", &Return);
	glGetFramebufferAttachmentParameteriv=glGetProcAddress("glGetFramebufferAttachmentParameteriv", &Return);
	glGenerateMipmap=glGetProcAddress("glGenerateMipmap", &Return);
	glBlitFramebuffer=glGetProcAddress("glBlitFramebuffer", &Return);
	glRenderbufferStorageMultisample=glGetProcAddress("glRenderbufferStorageMultisample", &Return);
	glFramebufferTextureLayer=glGetProcAddress("glFramebufferTextureLayer", &Return);
	glMapBufferRange=glGetProcAddress("glMapBufferRange", &Return);
	glFlushMappedBufferRange=glGetProcAddress("glFlushMappedBufferRange", &Return);
	glBindVertexArray=glGetProcAddress("glBindVertexArray", &Return);
	glDeleteVertexArrays=glGetProcAddress("glDeleteVertexArrays", &Return);
	glGenVertexArrays=glGetProcAddress("glGenVertexArrays", &Return);
	glIsVertexArray=glGetProcAddress("glIsVertexArray", &Return);
	glDrawArraysInstanced=glGetProcAddress("glDrawArraysInstanced", &Return);
	glDrawElementsInstanced=glGetProcAddress("glDrawElementsInstanced", &Return);
	glTexBuffer=glGetProcAddress("glTexBuffer", &Return);
	glPrimitiveRestartIndex=glGetProcAddress("glPrimitiveRestartIndex", &Return);
	glCopyBufferSubData=glGetProcAddress("glCopyBufferSubData", &Return);
	glGetUniformIndices=glGetProcAddress("glGetUniformIndices", &Return);
	glGetActiveUniformsiv=glGetProcAddress("glGetActiveUniformsiv", &Return);
	glGetActiveUniformName=glGetProcAddress("glGetActiveUniformName", &Return);
	glGetUniformBlockIndex=glGetProcAddress("glGetUniformBlockIndex", &Return);
	glGetActiveUniformBlockiv=glGetProcAddress("glGetActiveUniformBlockiv", &Return);
	glGetActiveUniformBlockName=glGetProcAddress("glGetActiveUniformBlockName", &Return);
	glUniformBlockBinding=glGetProcAddress("glUniformBlockBinding", &Return);
	glDrawElementsBaseVertex=glGetProcAddress("glDrawElementsBaseVertex", &Return);
	glDrawRangeElementsBaseVertex=glGetProcAddress("glDrawRangeElementsBaseVertex", &Return);
	glDrawElementsInstancedBaseVertex=glGetProcAddress("glDrawElementsInstancedBaseVertex", &Return);
	glMultiDrawElementsBaseVertex=glGetProcAddress("glMultiDrawElementsBaseVertex", &Return);
	glProvokingVertex=glGetProcAddress("glProvokingVertex", &Return);
	glFenceSync=glGetProcAddress("glFenceSync", &Return);
	glIsSync=glGetProcAddress("glIsSync", &Return);
	glDeleteSync=glGetProcAddress("glDeleteSync", &Return);
	glClientWaitSync=glGetProcAddress("glClientWaitSync", &Return);
	glWaitSync=glGetProcAddress("glWaitSync", &Return);
	glGetInteger64v=glGetProcAddress("glGetInteger64v", &Return);
	glGetSynciv=glGetProcAddress("glGetSynciv", &Return);
	glGetInteger64i_v=glGetProcAddress("glGetInteger64i_v", &Return);
	glGetBufferParameteri64v=glGetProcAddress("glGetBufferParameteri64v", &Return);
	glFramebufferTexture=glGetProcAddress("glFramebufferTexture", &Return);
	glTexImage2DMultisample=glGetProcAddress("glTexImage2DMultisample", &Return);
	glTexImage3DMultisample=glGetProcAddress("glTexImage3DMultisample", &Return);
	glGetMultisamplefv=glGetProcAddress("glGetMultisamplefv", &Return);
	glSampleMaski=glGetProcAddress("glSampleMaski", &Return);
	glBindFragDataLocationIndexed=glGetProcAddress("glBindFragDataLocationIndexed", &Return);
	glGetFragDataIndex=glGetProcAddress("glGetFragDataIndex", &Return);
	glGenSamplers=glGetProcAddress("glGenSamplers", &Return);
	glDeleteSamplers=glGetProcAddress("glDeleteSamplers", &Return);
	glIsSampler=glGetProcAddress("glIsSampler", &Return);
	glBindSampler=glGetProcAddress("glBindSampler", &Return);
	glSamplerParameteri=glGetProcAddress("glSamplerParameteri", &Return);
	glSamplerParameteriv=glGetProcAddress("glSamplerParameteriv", &Return);
	glSamplerParameterf=glGetProcAddress("glSamplerParameterf", &Return);
	glSamplerParameterfv=glGetProcAddress("glSamplerParameterfv", &Return);
	glSamplerParameterIiv=glGetProcAddress("glSamplerParameterIiv", &Return);
	glSamplerParameterIuiv=glGetProcAddress("glSamplerParameterIuiv", &Return);
	glGetSamplerParameteriv=glGetProcAddress("glGetSamplerParameteriv", &Return);
	glGetSamplerParameterIiv=glGetProcAddress("glGetSamplerParameterIiv", &Return);
	glGetSamplerParameterfv=glGetProcAddress("glGetSamplerParameterfv", &Return);
	glGetSamplerParameterIuiv=glGetProcAddress("glGetSamplerParameterIuiv", &Return);
	glQueryCounter=glGetProcAddress("glQueryCounter", &Return);
	glGetQueryObjecti64v=glGetProcAddress("glGetQueryObjecti64v", &Return);
	glGetQueryObjectui64v=glGetProcAddress("glGetQueryObjectui64v", &Return);
	glVertexAttribDivisor=glGetProcAddress("glVertexAttribDivisor", &Return);
	glVertexAttribP1ui=glGetProcAddress("glVertexAttribP1ui", &Return);
	glVertexAttribP1uiv=glGetProcAddress("glVertexAttribP1uiv", &Return);
	glVertexAttribP2ui=glGetProcAddress("glVertexAttribP2ui", &Return);
	glVertexAttribP2uiv=glGetProcAddress("glVertexAttribP2uiv", &Return);
	glVertexAttribP3ui=glGetProcAddress("glVertexAttribP3ui", &Return);
	glVertexAttribP3uiv=glGetProcAddress("glVertexAttribP3uiv", &Return);
	glVertexAttribP4ui=glGetProcAddress("glVertexAttribP4ui", &Return);
	glVertexAttribP4uiv=glGetProcAddress("glVertexAttribP4uiv", &Return);

	// Something went horribly wrong, or incomplete GL3.3 support
	if(!Return)
		return;

	// OpenGL 4.0
	glMinSampleShading=glGetProcAddress("glMinSampleShading", &Return);
	glBlendEquationi=glGetProcAddress("glBlendEquationi", &Return);
	glBlendEquationSeparatei=glGetProcAddress("glBlendEquationSeparatei", &Return);
	glBlendFunci=glGetProcAddress("glBlendFunci", &Return);
	glBlendFuncSeparatei=glGetProcAddress("glBlendFuncSeparatei", &Return);
	glDrawArraysIndirect=glGetProcAddress("glDrawArraysIndirect", &Return);
	glDrawElementsIndirect=glGetProcAddress("glDrawElementsIndirect", &Return);
	glUniform1d=glGetProcAddress("glUniform1d", &Return);
	glUniform2d=glGetProcAddress("glUniform2d", &Return);
	glUniform3d=glGetProcAddress("glUniform3d", &Return);
	glUniform4d=glGetProcAddress("glUniform4d", &Return);
	glUniform1dv=glGetProcAddress("glUniform1dv", &Return);
	glUniform2dv=glGetProcAddress("glUniform2dv", &Return);
	glUniform3dv=glGetProcAddress("glUniform3dv", &Return);
	glUniform4dv=glGetProcAddress("glUniform4dv", &Return);
	glUniformMatrix2dv=glGetProcAddress("glUniformMatrix2dv", &Return);
	glUniformMatrix3dv=glGetProcAddress("glUniformMatrix3dv", &Return);
	glUniformMatrix4dv=glGetProcAddress("glUniformMatrix4dv", &Return);
	glUniformMatrix2x3dv=glGetProcAddress("glUniformMatrix2x3dv", &Return);
	glUniformMatrix2x4dv=glGetProcAddress("glUniformMatrix2x4dv", &Return);
	glUniformMatrix3x2dv=glGetProcAddress("glUniformMatrix3x2dv", &Return);
	glUniformMatrix3x4dv=glGetProcAddress("glUniformMatrix3x4dv", &Return);
	glUniformMatrix4x2dv=glGetProcAddress("glUniformMatrix4x2dv", &Return);
	glUniformMatrix4x3dv=glGetProcAddress("glUniformMatrix4x3dv", &Return);
	glGetUniformdv=glGetProcAddress("glGetUniformdv", &Return);
	glGetSubroutineUniformLocation=glGetProcAddress("glGetSubroutineUniformLocation", &Return);
	glGetSubroutineIndex=glGetProcAddress("glGetSubroutineIndex", &Return);
	glGetActiveSubroutineUniformiv=glGetProcAddress("glGetActiveSubroutineUniformiv", &Return);
	glGetActiveSubroutineUniformName=glGetProcAddress("glGetActiveSubroutineUniformName", &Return);
	glGetActiveSubroutineName=glGetProcAddress("glGetActiveSubroutineName", &Return);
	glUniformSubroutinesuiv=glGetProcAddress("glUniformSubroutinesuiv", &Return);
	glGetUniformSubroutineuiv=glGetProcAddress("glGetUniformSubroutineuiv", &Return);
	glGetProgramStageiv=glGetProcAddress("glGetProgramStageiv", &Return);
	glPatchParameteri=glGetProcAddress("glPatchParameteri", &Return);
	glPatchParameterfv=glGetProcAddress("glPatchParameterfv", &Return);
	glBindTransformFeedback=glGetProcAddress("glBindTransformFeedback", &Return);
	glDeleteTransformFeedbacks=glGetProcAddress("glDeleteTransformFeedbacks", &Return);
	glGenTransformFeedbacks=glGetProcAddress("glGenTransformFeedbacks", &Return);
	glIsTransformFeedback=glGetProcAddress("glIsTransformFeedback", &Return);
	glPauseTransformFeedback=glGetProcAddress("glPauseTransformFeedback", &Return);
	glPauseTransformFeedback=glGetProcAddress("glPauseTransformFeedback", &Return);
	glResumeTransformFeedback=glGetProcAddress("glResumeTransformFeedback", &Return);
	glDrawTransformFeedback=glGetProcAddress("glDrawTransformFeedback", &Return);
	glDrawTransformFeedbackStream=glGetProcAddress("glDrawTransformFeedbackStream", &Return);
	glBeginQueryIndexed=glGetProcAddress("glBeginQueryIndexed", &Return);
	glEndQueryIndexed=glGetProcAddress("glEndQueryIndexed", &Return);
	glGetQueryIndexediv=glGetProcAddress("glGetQueryIndexediv", &Return);

	// Something went horribly wrong, or incomplete GL4.0 support
	if(!Return)
		return;

	// OpenGL 4.1
	glReleaseShaderCompiler=glGetProcAddress("glReleaseShaderCompiler", &Return);
	glShaderBinary=glGetProcAddress("glShaderBinary", &Return);
	glGetShaderPrecisionFormat=glGetProcAddress("glGetShaderPrecisionFormat", &Return);
	glDepthRangef=glGetProcAddress("glDepthRangef", &Return);
	glClearDepthf=glGetProcAddress("glClearDepthf", &Return);
	glGetProgramBinary=glGetProcAddress("glGetProgramBinary", &Return);
	glProgramBinary=glGetProcAddress("glProgramBinary", &Return);
	glProgramParameteri=glGetProcAddress("glProgramParameteri", &Return);
	glUseProgramStages=glGetProcAddress("glUseProgramStages", &Return);
	glActiveShaderProgram=glGetProcAddress("glActiveShaderProgram", &Return);
	glCreateShaderProgramv=glGetProcAddress("glCreateShaderProgramv", &Return);
	glBindProgramPipeline=glGetProcAddress("glBindProgramPipeline", &Return);
	glDeleteProgramPipelines=glGetProcAddress("glDeleteProgramPipelines", &Return);
	glGenProgramPipelines=glGetProcAddress("glGenProgramPipelines", &Return);
	glIsProgramPipeline=glGetProcAddress("glIsProgramPipeline", &Return);
	glGetProgramPipelineiv=glGetProcAddress("glGetProgramPipelineiv", &Return);
	glProgramUniform1i=glGetProcAddress("glProgramUniform1i", &Return);
	glProgramUniform1iv=glGetProcAddress("glProgramUniform1iv", &Return);
	glProgramUniform1f=glGetProcAddress("glProgramUniform1f", &Return);
	glProgramUniform1fv=glGetProcAddress("glProgramUniform1fv", &Return);
	glProgramUniform1d=glGetProcAddress("glProgramUniform1d", &Return);
	glProgramUniform1dv=glGetProcAddress("glProgramUniform1dv", &Return);
	glProgramUniform1ui=glGetProcAddress("glProgramUniform1ui", &Return);
	glProgramUniform1uiv=glGetProcAddress("glProgramUniform1uiv", &Return);
	glProgramUniform2i=glGetProcAddress("glProgramUniform2i", &Return);
	glProgramUniform2iv=glGetProcAddress("glProgramUniform2iv", &Return);
	glProgramUniform2f=glGetProcAddress("glProgramUniform2f", &Return);
	glProgramUniform2fv=glGetProcAddress("glProgramUniform2fv", &Return);
	glProgramUniform2d=glGetProcAddress("glProgramUniform2d", &Return);
	glProgramUniform2dv=glGetProcAddress("glProgramUniform2dv", &Return);
	glProgramUniform2ui=glGetProcAddress("glProgramUniform2ui", &Return);
	glProgramUniform2uiv=glGetProcAddress("glProgramUniform2uiv", &Return);
	glProgramUniform3i=glGetProcAddress("glProgramUniform3i", &Return);
	glProgramUniform3iv=glGetProcAddress("glProgramUniform3iv", &Return);
	glProgramUniform3f=glGetProcAddress("glProgramUniform3f", &Return);
	glProgramUniform3fv=glGetProcAddress("glProgramUniform3fv", &Return);
	glProgramUniform3d=glGetProcAddress("glProgramUniform3d", &Return);
	glProgramUniform3dv=glGetProcAddress("glProgramUniform3dv", &Return);
	glProgramUniform3ui=glGetProcAddress("glProgramUniform3ui", &Return);
	glProgramUniform3uiv=glGetProcAddress("glProgramUniform3uiv", &Return);
	glProgramUniform4i=glGetProcAddress("glProgramUniform4i", &Return);
	glProgramUniform4iv=glGetProcAddress("glProgramUniform4iv", &Return);
	glProgramUniform4f=glGetProcAddress("glProgramUniform4f", &Return);
	glProgramUniform4fv=glGetProcAddress("glProgramUniform4fv", &Return);
	glProgramUniform4d=glGetProcAddress("glProgramUniform4d", &Return);
	glProgramUniform4dv=glGetProcAddress("glProgramUniform4dv", &Return);
	glProgramUniform4ui=glGetProcAddress("glProgramUniform4ui", &Return);
	glProgramUniform4uiv=glGetProcAddress("glProgramUniform4uiv", &Return);
	glProgramUniformMatrix2fv=glGetProcAddress("glProgramUniformMatrix2fv", &Return);
	glProgramUniformMatrix3fv=glGetProcAddress("glProgramUniformMatrix3fv", &Return);
	glProgramUniformMatrix4fv=glGetProcAddress("glProgramUniformMatrix4fv", &Return);
	glProgramUniformMatrix2dv=glGetProcAddress("glProgramUniformMatrix2dv", &Return);
	glProgramUniformMatrix3dv=glGetProcAddress("glProgramUniformMatrix3dv", &Return);
	glProgramUniformMatrix4dv=glGetProcAddress("glProgramUniformMatrix4dv", &Return);
	glProgramUniformMatrix2x3fv=glGetProcAddress("glProgramUniformMatrix2x3fv", &Return);
	glProgramUniformMatrix3x2fv=glGetProcAddress("glProgramUniformMatrix3x2fv", &Return);
	glProgramUniformMatrix2x4fv=glGetProcAddress("glProgramUniformMatrix2x4fv", &Return);
	glProgramUniformMatrix4x2fv=glGetProcAddress("glProgramUniformMatrix4x2fv", &Return);
	glProgramUniformMatrix3x4fv=glGetProcAddress("glProgramUniformMatrix3x4fv", &Return);
	glProgramUniformMatrix4x3fv=glGetProcAddress("glProgramUniformMatrix4x3fv", &Return);
	glProgramUniformMatrix2x3dv=glGetProcAddress("glProgramUniformMatrix2x3dv", &Return);
	glProgramUniformMatrix3x2dv=glGetProcAddress("glProgramUniformMatrix3x2dv", &Return);
	glProgramUniformMatrix2x4dv=glGetProcAddress("glProgramUniformMatrix2x4dv", &Return);
	glProgramUniformMatrix4x2dv=glGetProcAddress("glProgramUniformMatrix4x2dv", &Return);
	glProgramUniformMatrix3x4dv=glGetProcAddress("glProgramUniformMatrix3x4dv", &Return);
	glProgramUniformMatrix4x3dv=glGetProcAddress("glProgramUniformMatrix4x3dv", &Return);
	glValidateProgramPipeline=glGetProcAddress("glValidateProgramPipeline", &Return);
	glGetProgramPipelineInfoLog=glGetProcAddress("glGetProgramPipelineInfoLog", &Return);
	glVertexAttribL1d=glGetProcAddress("glVertexAttribL1d", &Return);
	glVertexAttribL2d=glGetProcAddress("glVertexAttribL2d", &Return);
	glVertexAttribL3d=glGetProcAddress("glVertexAttribL3d", &Return);
	glVertexAttribL4d=glGetProcAddress("glVertexAttribL4d", &Return);
	glVertexAttribL1dv=glGetProcAddress("glVertexAttribL1dv", &Return);
	glVertexAttribL2dv=glGetProcAddress("glVertexAttribL2dv", &Return);
	glVertexAttribL3dv=glGetProcAddress("glVertexAttribL3dv", &Return);
	glVertexAttribL4dv=glGetProcAddress("glVertexAttribL4dv", &Return);
	glVertexAttribLPointer=glGetProcAddress("glVertexAttribLPointer", &Return);
	glGetVertexAttribLdv=glGetProcAddress("glGetVertexAttribLdv", &Return);
	glViewportArrayv=glGetProcAddress("glViewportArrayv", &Return);
	glViewportIndexedf=glGetProcAddress("glViewportIndexedf", &Return);
	glViewportIndexedfv=glGetProcAddress("glViewportIndexedfv", &Return);
	glScissorArrayv=glGetProcAddress("glScissorArrayv", &Return);
	glScissorIndexed=glGetProcAddress("glScissorIndexed", &Return);
	glScissorIndexedv=glGetProcAddress("glScissorIndexedv", &Return);
	glDepthRangeArrayv=glGetProcAddress("glDepthRangeArrayv", &Return);
	glDepthRangeIndexed=glGetProcAddress("glDepthRangeIndexed", &Return);
	glGetFloati_v=glGetProcAddress("glGetFloati_v", &Return);
	glGetDoublei_v=glGetProcAddress("glGetDoublei_v", &Return);

	// Something went horribly wrong, or incomplete GL4.1 support
	if(!Return)
		return;

	// OpenGL 4.2
	glDrawArraysInstancedBaseInstance=glGetProcAddress("glDrawArraysInstancedBaseInstance", &Return);
	glDrawElementsInstancedBaseInstance=glGetProcAddress("glDrawElementsInstancedBaseInstance", &Return);
	glDrawElementsInstancedBaseVertexBaseInstance=glGetProcAddress("glDrawElementsInstancedBaseVertexBaseInstance", &Return);
	glGetInternalformativ=glGetProcAddress("glGetInternalformativ", &Return);
	glGetActiveAtomicCounterBufferiv=glGetProcAddress("glGetActiveAtomicCounterBufferiv", &Return);
	glBindImageTexture=glGetProcAddress("glBindImageTexture", &Return);
	glMemoryBarrier=glGetProcAddress("glMemoryBarrier", &Return);
	glTexStorage1D=glGetProcAddress("glTexStorage1D", &Return);
	glTexStorage2D=glGetProcAddress("glTexStorage2D", &Return);
	glTexStorage3D=glGetProcAddress("glTexStorage3D", &Return);
	glDrawTransformFeedbackInstanced=glGetProcAddress("glDrawTransformFeedbackInstanced", &Return);
	glDrawTransformFeedbackStreamInstanced=glGetProcAddress("glDrawTransformFeedbackStreamInstanced", &Return);

	// Something went horribly wrong, or incomplete GL4.2 support
	if(!Return)
		return;

	// OpenGL 4.3
	glClearBufferData=glGetProcAddress("glClearBufferData", &Return);
	glClearBufferSubData=glGetProcAddress("glClearBufferSubData", &Return);
	glDispatchCompute=glGetProcAddress("glDispatchCompute", &Return);
	glDispatchComputeIndirect=glGetProcAddress("glDispatchComputeIndirect", &Return);
	glCopyImageSubData=glGetProcAddress("glCopyImageSubData", &Return);
	glFramebufferParameteri=glGetProcAddress("glFramebufferParameteri", &Return);
	glGetFramebufferParameteriv=glGetProcAddress("glGetFramebufferParameteriv", &Return);
	glGetInternalformati64v=glGetProcAddress("glGetInternalformati64v", &Return);
	glInvalidateTexSubImage=glGetProcAddress("glInvalidateTexSubImage", &Return);
	glInvalidateTexImage=glGetProcAddress("glInvalidateTexImage", &Return);
	glInvalidateBufferSubData=glGetProcAddress("glInvalidateBufferSubData", &Return);
	glInvalidateBufferData=glGetProcAddress("glInvalidateBufferData", &Return);
	glInvalidateFramebuffer=glGetProcAddress("glInvalidateFramebuffer", &Return);
	glInvalidateSubFramebuffer=glGetProcAddress("glInvalidateSubFramebuffer", &Return);
	glMultiDrawArraysIndirect=glGetProcAddress("glMultiDrawArraysIndirect", &Return);
	glMultiDrawElementsIndirect=glGetProcAddress("glMultiDrawElementsIndirect", &Return);
	glGetProgramInterfaceiv=glGetProcAddress("glGetProgramInterfaceiv", &Return);
	glGetProgramResourceIndex=glGetProcAddress("glGetProgramResourceIndex", &Return);
	glGetProgramResourceName=glGetProcAddress("glGetProgramResourceName", &Return);
	glGetProgramResourceiv=glGetProcAddress("glGetProgramResourceiv", &Return);
	glGetProgramResourceLocation=glGetProcAddress("glGetProgramResourceLocation", &Return);
	glGetProgramResourceLocationIndex=glGetProcAddress("glGetProgramResourceLocationIndex", &Return);
	glShaderStorageBlockBinding=glGetProcAddress("glShaderStorageBlockBinding", &Return);
	glTexBufferRange=glGetProcAddress("glTexBufferRange", &Return);
	glTexStorage2DMultisample=glGetProcAddress("glTexStorage2DMultisample", &Return);
	glTexStorage3DMultisample=glGetProcAddress("glTexStorage3DMultisample", &Return);
	glTextureView=glGetProcAddress("glTextureView", &Return);
	glBindVertexBuffer=glGetProcAddress("glBindVertexBuffer", &Return);
	glVertexAttribFormat=glGetProcAddress("glVertexAttribFormat", &Return);
	glVertexAttribIFormat=glGetProcAddress("glVertexAttribIFormat", &Return);
	glVertexAttribLFormat=glGetProcAddress("glVertexAttribLFormat", &Return);
	glVertexAttribBinding=glGetProcAddress("glVertexAttribBinding", &Return);
	glVertexBindingDivisor=glGetProcAddress("glVertexBindingDivisor", &Return);
	glDebugMessageControl=glGetProcAddress("glDebugMessageControl", &Return);
	glDebugMessageInsert=glGetProcAddress("glDebugMessageInsert", &Return);
	glDebugMessageCallback=glGetProcAddress("glDebugMessageCallback", &Return);
	glGetDebugMessageLog=glGetProcAddress("glGetDebugMessageLog", &Return);
	glPushDebugGroup=glGetProcAddress("glPushDebugGroup", &Return);
	glPopDebugGroup=glGetProcAddress("glPopDebugGroup", &Return);
	glObjectLabel=glGetProcAddress("glObjectLabel", &Return);
	glGetObjectLabel=glGetProcAddress("glGetObjectLabel", &Return);
	glObjectPtrLabel=glGetProcAddress("glObjectPtrLabel", &Return);
	glGetObjectPtrLabel=glGetProcAddress("glGetObjectPtrLabel", &Return);

	// Something went horribly wrong, or incomplete GL4.3 support
	if(!Return)
		return;

	// OpenGL 4.4
	glBufferStorage=glGetProcAddress("glBufferStorage", &Return);
	glClearTexImage=glGetProcAddress("glClearTexImage", &Return);
	glClearTexSubImage=glGetProcAddress("glClearTexSubImage", &Return);
	glBindBuffersBase=glGetProcAddress("glBindBuffersBase", &Return);
	glBindBuffersRange=glGetProcAddress("glBindBuffersRange", &Return);
	glBindTextures=glGetProcAddress("glBindTextures", &Return);
	glBindSamplers=glGetProcAddress("glBindSamplers", &Return);
	glBindImageTextures=glGetProcAddress("glBindImageTextures", &Return);
	glBindVertexBuffers=glGetProcAddress("glBindVertexBuffers", &Return);

	// Something went horribly wrong, or incomplete GL4.4 support
	if(!Return)
		return;

	// OpenGL 4.5
	glClipControl=glGetProcAddress("glClipControl", &Return);
	glCreateTransformFeedbacks=glGetProcAddress("glCreateTransformFeedbacks", &Return);
	glTransformFeedbackBufferBase=glGetProcAddress("glTransformFeedbackBufferBase", &Return);
	glTransformFeedbackBufferRange=glGetProcAddress("glTransformFeedbackBufferRange", &Return);
	glGetTransformFeedbackiv=glGetProcAddress("glGetTransformFeedbackiv", &Return);
	glGetTransformFeedbacki_v=glGetProcAddress("glGetTransformFeedbacki_v", &Return);
	glGetTransformFeedbacki64_v=glGetProcAddress("glGetTransformFeedbacki64_v", &Return);
	glCreateBuffers=glGetProcAddress("glCreateBuffers", &Return);
	glNamedBufferStorage=glGetProcAddress("glNamedBufferStorage", &Return);
	glNamedBufferData=glGetProcAddress("glNamedBufferData", &Return);
	glNamedBufferSubData=glGetProcAddress("glNamedBufferSubData", &Return);
	glCopyNamedBufferSubData=glGetProcAddress("glCopyNamedBufferSubData", &Return);
	glClearNamedBufferData=glGetProcAddress("glClearNamedBufferData", &Return);
	glClearNamedBufferSubData=glGetProcAddress("glClearNamedBufferSubData", &Return);
	glMapNamedBuffer=glGetProcAddress("glMapNamedBuffer", &Return);
	glMapNamedBufferRange=glGetProcAddress("glMapNamedBufferRange", &Return);
	glUnmapNamedBuffer=glGetProcAddress("glUnmapNamedBuffer", &Return);
	glFlushMappedNamedBufferRange=glGetProcAddress("glFlushMappedNamedBufferRange", &Return);
	glGetNamedBufferParameteriv=glGetProcAddress("glGetNamedBufferParameteriv", &Return);
	glGetNamedBufferParameteri64v=glGetProcAddress("glGetNamedBufferParameteri64v", &Return);
	glGetNamedBufferPointerv=glGetProcAddress("glGetNamedBufferPointerv", &Return);
	glGetNamedBufferSubData=glGetProcAddress("glGetNamedBufferSubData", &Return);
	glCreateFramebuffers=glGetProcAddress("glCreateFramebuffers", &Return);
	glNamedFramebufferRenderbuffer=glGetProcAddress("glNamedFramebufferRenderbuffer", &Return);
	glNamedFramebufferParameteri=glGetProcAddress("glNamedFramebufferParameteri", &Return);
	glNamedFramebufferTexture=glGetProcAddress("glNamedFramebufferTexture", &Return);
	glNamedFramebufferTextureLayer=glGetProcAddress("glNamedFramebufferTextureLayer", &Return);
	glNamedFramebufferDrawBuffer=glGetProcAddress("glNamedFramebufferDrawBuffer", &Return);
	glNamedFramebufferDrawBuffers=glGetProcAddress("glNamedFramebufferDrawBuffers", &Return);
	glNamedFramebufferReadBuffer=glGetProcAddress("glNamedFramebufferReadBuffer", &Return);
	glInvalidateNamedFramebufferData=glGetProcAddress("glInvalidateNamedFramebufferData", &Return);
	glInvalidateNamedFramebufferSubData=glGetProcAddress("glInvalidateNamedFramebufferSubData", &Return);
	glClearNamedFramebufferiv=glGetProcAddress("glClearNamedFramebufferiv", &Return);
	glClearNamedFramebufferuiv=glGetProcAddress("glClearNamedFramebufferuiv", &Return);
	glClearNamedFramebufferfv=glGetProcAddress("glClearNamedFramebufferfv", &Return);
	glClearNamedFramebufferfi=glGetProcAddress("glClearNamedFramebufferfi", &Return);
	glBlitNamedFramebuffer=glGetProcAddress("glBlitNamedFramebuffer", &Return);
	glCheckNamedFramebufferStatus=glGetProcAddress("glCheckNamedFramebufferStatus", &Return);
	glGetNamedFramebufferParameteriv=glGetProcAddress("glGetNamedFramebufferParameteriv", &Return);
	glGetNamedFramebufferAttachmentParameteriv=glGetProcAddress("glGetNamedFramebufferAttachmentParameteriv", &Return);
	glCreateRenderbuffers=glGetProcAddress("glCreateRenderbuffers", &Return);
	glNamedRenderbufferStorage=glGetProcAddress("glNamedRenderbufferStorage", &Return);
	glNamedRenderbufferStorageMultisample=glGetProcAddress("glNamedRenderbufferStorageMultisample", &Return);
	glGetNamedRenderbufferParameteriv=glGetProcAddress("glGetNamedRenderbufferParameteriv", &Return);
	glCreateTextures=glGetProcAddress("glCreateTextures", &Return);
	glTextureBuffer=glGetProcAddress("glTextureBuffer", &Return);
	glTextureBufferRange=glGetProcAddress("glTextureBufferRange", &Return);
	glTextureStorage1D=glGetProcAddress("glTextureStorage1D", &Return);
	glTextureStorage2D=glGetProcAddress("glTextureStorage2D", &Return);
	glTextureStorage3D=glGetProcAddress("glTextureStorage3D", &Return);
	glTextureStorage2DMultisample=glGetProcAddress("glTextureStorage2DMultisample", &Return);
	glTextureStorage3DMultisample=glGetProcAddress("glTextureStorage3DMultisample", &Return);
	glTextureSubImage1D=glGetProcAddress("glTextureSubImage1D", &Return);
	glTextureSubImage2D=glGetProcAddress("glTextureSubImage2D", &Return);
	glTextureSubImage3D=glGetProcAddress("glTextureSubImage3D", &Return);
	glCompressedTextureSubImage1D=glGetProcAddress("glCompressedTextureSubImage1D", &Return);
	glCompressedTextureSubImage2D=glGetProcAddress("glCompressedTextureSubImage2D", &Return);
	glCompressedTextureSubImage3D=glGetProcAddress("glCompressedTextureSubImage3D", &Return);
	glCopyTextureSubImage1D=glGetProcAddress("glCopyTextureSubImage1D", &Return);
	glCopyTextureSubImage2D=glGetProcAddress("glCopyTextureSubImage2D", &Return);
	glCopyTextureSubImage3D=glGetProcAddress("glCopyTextureSubImage3D", &Return);
	glTextureParameterf=glGetProcAddress("glTextureParameterf", &Return);
	glTextureParameterfv=glGetProcAddress("glTextureParameterfv", &Return);
	glTextureParameteri=glGetProcAddress("glTextureParameteri", &Return);
	glTextureParameterIiv=glGetProcAddress("glTextureParameterIiv", &Return);
	glTextureParameterIuiv=glGetProcAddress("glTextureParameterIuiv", &Return);
	glTextureParameteriv=glGetProcAddress("glTextureParameteriv", &Return);
	glGenerateTextureMipmap=glGetProcAddress("glGenerateTextureMipmap", &Return);
	glBindTextureUnit=glGetProcAddress("glBindTextureUnit", &Return);
	glGetTextureImage=glGetProcAddress("glGetTextureImage", &Return);
	glGetCompressedTextureImage=glGetProcAddress("glGetCompressedTextureImage", &Return);
	glGetTextureLevelParameterfv=glGetProcAddress("glGetTextureLevelParameterfv", &Return);
	glGetTextureLevelParameteriv=glGetProcAddress("glGetTextureLevelParameteriv", &Return);
	glGetTextureParameterfv=glGetProcAddress("glGetTextureParameterfv", &Return);
	glGetTextureParameterIiv=glGetProcAddress("glGetTextureParameterIiv", &Return);
	glGetTextureParameterIuiv=glGetProcAddress("glGetTextureParameterIuiv", &Return);
	glGetTextureParameteriv=glGetProcAddress("glGetTextureParameteriv", &Return);
	glCreateVertexArrays=glGetProcAddress("glCreateVertexArrays", &Return);
	glDisableVertexArrayAttrib=glGetProcAddress("glDisableVertexArrayAttrib", &Return);
	glEnableVertexArrayAttrib=glGetProcAddress("glEnableVertexArrayAttrib", &Return);
	glVertexArrayElementBuffer=glGetProcAddress("glVertexArrayElementBuffer", &Return);
	glVertexArrayVertexBuffer=glGetProcAddress("glVertexArrayVertexBuffer", &Return);
	glVertexArrayVertexBuffers=glGetProcAddress("glVertexArrayVertexBuffers", &Return);
	glVertexArrayAttribBinding=glGetProcAddress("glVertexArrayAttribBinding", &Return);
	glVertexArrayAttribFormat=glGetProcAddress("glVertexArrayAttribFormat", &Return);
	glVertexArrayAttribIFormat=glGetProcAddress("glVertexArrayAttribIFormat", &Return);
	glVertexArrayAttribLFormat=glGetProcAddress("glVertexArrayAttribLFormat", &Return);
	glVertexArrayBindingDivisor=glGetProcAddress("glVertexArrayBindingDivisor", &Return);
	glGetVertexArrayiv=glGetProcAddress("glGetVertexArrayiv", &Return);
	glGetVertexArrayIndexediv=glGetProcAddress("glGetVertexArrayIndexediv", &Return);
	glGetVertexArrayIndexed64iv=glGetProcAddress("glGetVertexArrayIndexed64iv", &Return);
	glCreateSamplers=glGetProcAddress("glCreateSamplers", &Return);
	glCreateProgramPipelines=glGetProcAddress("glCreateProgramPipelines", &Return);
	glCreateQueries=glGetProcAddress("glCreateQueries", &Return);
	glGetQueryBufferObjecti64v=glGetProcAddress("glGetQueryBufferObjecti64v", &Return);
	glGetQueryBufferObjectiv=glGetProcAddress("glGetQueryBufferObjectiv", &Return);
	glGetQueryBufferObjectui64v=glGetProcAddress("glGetQueryBufferObjectui64v", &Return);
	glGetQueryBufferObjectuiv=glGetProcAddress("glGetQueryBufferObjectuiv", &Return);
	glMemoryBarrierByRegion=glGetProcAddress("glMemoryBarrierByRegion", &Return);
	glGetTextureSubImage=glGetProcAddress("glGetTextureSubImage", &Return);
	glGetCompressedTextureSubImage=glGetProcAddress("glGetCompressedTextureSubImage", &Return);
	glGetGraphicsResetStatus=glGetProcAddress("glGetGraphicsResetStatus", &Return);
	glGetnCompressedTexImage=glGetProcAddress("glGetnCompressedTexImage", &Return);
	glGetnTexImage=glGetProcAddress("glGetnTexImage", &Return);
	glGetnUniformdv=glGetProcAddress("glGetnUniformdv", &Return);
	glGetnUniformfv=glGetProcAddress("glGetnUniformfv", &Return);
	glGetnUniformiv=glGetProcAddress("glGetnUniformiv", &Return);
	glGetnUniformuiv=glGetProcAddress("glGetnUniformuiv", &Return);
	glReadnPixels=glGetProcAddress("glReadnPixels", &Return);
	glTextureBarrier=glGetProcAddress("glTextureBarrier", &Return);

	// Something went horribly wrong, or incomplete GL4.5 support
	if(!Return)
		return;

	// OpenGL 4.6
	glSpecializeShader=glGetProcAddress("glSpecializeShader", &Return);
	glMultiDrawArraysIndirectCount=glGetProcAddress("glMultiDrawArraysIndirectCount", &Return);
	glMultiDrawElementsIndirectCount=glGetProcAddress("glMultiDrawElementsIndirectCount", &Return);
	glPolygonOffsetClamp=glGetProcAddress("glPolygonOffsetClamp", &Return);

	// Something went horribly wrong, or incomplete GL4.6 support
	if(!Return)
		return;

	GLExtensionsInitialized_Flag=1;
}

#ifdef WIN32
void WGLSetupExtensions(void)
{
	int Return=1;
	char *WGLExtensionString=NULL;

	wglGetExtensionsStringARB=glGetProcAddress("wglGetExtensionsStringARB", &Return);

	if(Return)
		WGLExtensionString=(char *)wglGetExtensionsStringARB(wglGetCurrentDC());

	if(FindSubString(WGLExtensionString, "WGL_ARB_create_context_profile"))
		WGL_ARB_create_context_profile_Flag=1;

	if(FindSubString(WGLExtensionString, "WGL_ARB_create_context_robustness"))
		WGL_ARB_create_context_robustness_Flag=1;

	if(FindSubString(WGLExtensionString, "WGL_ARB_create_context"))
	{
		Return=1;

		wglCreateContextAttribsARB=glGetProcAddress("wglCreateContextAttribsARB", &Return);

		if(Return)
			WGL_ARB_create_context_Flag=1;
	}

	if(FindSubString(WGLExtensionString, "WGL_ARB_framebuffer_sRGB"))
		WGL_ARB_framebuffer_sRGB_Flag=1;

	if(FindSubString(WGLExtensionString, "WGL_ARB_make_current_read"))
	{
		Return=1;

		wglMakeContextCurrentARB=glGetProcAddress("wglMakeContextCurrentARB", &Return);
		wglGetCurrentReadDCARB=glGetProcAddress("wglGetCurrentReadDCARB", &Return);

		if(Return)
			WGL_ARB_make_current_read_Flag=1;
	}

	if(FindSubString(WGLExtensionString, "WGL_ARB_multisample"))
		WGL_ARB_multisample_Flag=1;

	if(FindSubString(WGLExtensionString, "WGL_ARB_pixel_format"))
	{
		Return=1;

		wglGetPixelFormatAttribivARB=glGetProcAddress("wglGetPixelFormatAttribivARB", &Return);
		wglGetPixelFormatAttribfvARB=glGetProcAddress("wglGetPixelFormatAttribfvARB", &Return);
		wglChoosePixelFormatARB=glGetProcAddress("wglChoosePixelFormatARB", &Return);

		if(Return)
			WGL_ARB_pixel_format_Flag=1;
	}

	WGLExtensionsInitialized_Flag=1;
}
#else
void GLXSetupExtensions(Display *dpy)
{
	int Return=1;
	char *GLXExtensionString=NULL;

	GLXExtensionString=(char *)glXQueryExtensionsString(dpy, DefaultScreen(dpy));

	if(FindSubString(GLXExtensionString, "GLX_ARB_create_context"))
	{
		glXCreateContextAttribsARB=glGetProcAddress("glXCreateContextAttribsARB", &Return);

		if(Return)
			GLX_ARB_create_context_Flag=1;
	}

	if(FindSubString(GLXExtensionString, "GLX_ARB_create_context_profile"))
		GLX_ARB_create_context_profile_Flag=1;

	if(FindSubString(GLXExtensionString, "GLX_ARB_create_context_robustness"))
		GLX_ARB_create_context_robustness_Flag=1;

	if(FindSubString(GLXExtensionString, "GLX_ARB_framebuffer_sRGB"))
		GLX_ARB_framebuffer_sRGB_Flag=1;

	if(FindSubString(GLXExtensionString, "GLX_ARB_multisample"))
		GLX_ARB_multisample_Flag=1;

	GLXExtensionsInitialized_Flag=1;
}
#endif

int CreateContext(GLContext_t *Context, int Color, int Depth, int Stencil, int Accum, unsigned long Flags)
{
#ifdef WIN32
	int PixelFormat;
	unsigned int NumFormats;
	int Attrib[128], i=0;
	int ContextAttribs[32];

	if(!(GLExtensionsInitialized_Flag||WGLExtensionsInitialized_Flag))
	{
		PIXELFORMATDESCRIPTOR pfd;
		HWND hWndTemp=NULL;
		HDC hDCTemp=NULL;
		HGLRC hRCTemp=NULL;

		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion=1;
		pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL;
		pfd.iPixelType=PFD_TYPE_RGBA;
		pfd.cColorBits=32;
		pfd.cDepthBits=24;
		pfd.iLayerType=PFD_MAIN_PLANE;
		
		hWndTemp=CreateWindow("static", NULL, WS_POPUP, 0, 0, 1, 1, NULL, NULL, (HINSTANCE)GetModuleHandle(NULL), NULL);
		hDCTemp=GetDC(hWndTemp);

		if(!(PixelFormat=ChoosePixelFormat(hDCTemp, &pfd)))
		{
			DBGPRINTF("Dummy window ChoosePixelFormat Failed!");
			return 0;
		}

		if(!SetPixelFormat(hDCTemp, PixelFormat, &pfd))
		{
			DBGPRINTF("Dummy window SetPixelFormat Failed!");
			return 0;
		}

		if(!(hRCTemp=wglCreateContext(hDCTemp)))
		{
			DBGPRINTF("Dummy window wglCreateContext Failed!");
			return 0;
		}


		if(!wglMakeCurrent(hDCTemp, hRCTemp))
		{
			DBGPRINTF("Dummy window wglMakeCurrent Failed!");
			return 0;
		}

		if(!GLExtensionsInitialized_Flag)
			GLSetupExtensions();

		if(!WGLExtensionsInitialized_Flag)
			WGLSetupExtensions();

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hRCTemp);
		ReleaseDC(hWndTemp, hDCTemp);
		DestroyWindow(hWndTemp);
	}

	if(!WGL_ARB_pixel_format_Flag)
		return 0;

	Context->hDC=GetDC(Context->hWnd);

	Attrib[i++]=WGL_DRAW_TO_WINDOW_ARB;
	Attrib[i++]=TRUE;
	Attrib[i++]=WGL_SUPPORT_OPENGL_ARB;
	Attrib[i++]=TRUE;

	if(Flags&OGL_SINGLEBUFFER)
	{
		Attrib[i++]=WGL_DOUBLE_BUFFER_ARB;
		Attrib[i++]=FALSE;
	}

	if(Flags&OGL_DOUBLEBUFFER)
	{
		Attrib[i++]=WGL_DOUBLE_BUFFER_ARB;
		Attrib[i++]=TRUE;
	}

	Attrib[i++]=WGL_PIXEL_TYPE_ARB;
	Attrib[i++]=WGL_TYPE_RGBA_ARB;

	Attrib[i++]=WGL_COLOR_BITS_ARB;
	Attrib[i++]=Color;
	Attrib[i++]=WGL_ACCUM_BITS_ARB;
	Attrib[i++]=Accum;
	Attrib[i++]=WGL_DEPTH_BITS_ARB;
	Attrib[i++]=Depth;
	Attrib[i++]=WGL_STENCIL_BITS_ARB;
	Attrib[i++]=Stencil;

	if(Flags&OGL_AUXBUFFERS1)
	{
		Attrib[i++]=WGL_AUX_BUFFERS_ARB;
		Attrib[i++]=1;
	}

	if(Flags&OGL_AUXBUFFERS2)
	{
		Attrib[i++]=WGL_AUX_BUFFERS_ARB;
		Attrib[i++]=2;
	}

	if(Flags&OGL_AUXBUFFERS3)
	{
		Attrib[i++]=WGL_AUX_BUFFERS_ARB;
		Attrib[i++]=3;
	}

	if(Flags&OGL_AUXBUFFERS4)
	{
		Attrib[i++]=WGL_AUX_BUFFERS_ARB;
		Attrib[i++]=4;
	}

	if(WGL_ARB_multisample_Flag)
	{
		if(Flags&OGL_SAMPLES2X)
		{
			Attrib[i++]=WGL_SAMPLE_BUFFERS_ARB;
			Attrib[i++]=TRUE;
			Attrib[i++]=WGL_SAMPLES_ARB;
			Attrib[i++]=2;
		}

		if(Flags&OGL_SAMPLES4X)
		{
			Attrib[i++]=WGL_SAMPLE_BUFFERS_ARB;
			Attrib[i++]=TRUE;
			Attrib[i++]=WGL_SAMPLES_ARB;
			Attrib[i++]=4;
		}

		if(Flags&OGL_SAMPLES6X)
		{
			Attrib[i++]=WGL_SAMPLE_BUFFERS_ARB;
			Attrib[i++]=TRUE;
			Attrib[i++]=WGL_SAMPLES_ARB;
			Attrib[i++]=6;
		}

		if(Flags&OGL_SAMPLES8X)
		{
			Attrib[i++]=WGL_SAMPLE_BUFFERS_ARB;
			Attrib[i++]=TRUE;
			Attrib[i++]=WGL_SAMPLES_ARB;
			Attrib[i++]=8;
		}
	}

	Attrib[i++]=0;

	if(!wglChoosePixelFormatARB(Context->hDC, Attrib, NULL, 1, &PixelFormat, &NumFormats))
	{
		DBGPRINTF("wglChoosePixelFormatARB failed!");
		return 0;
	}

	if(!SetPixelFormat(Context->hDC, PixelFormat, NULL))
	{
		DBGPRINTF("SetPixelFormat Failed!");
		return 0;
	}

	if(WGL_ARB_create_context_Flag)
	{
		i=0;

		if(Flags&OGL_CORE30)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=3;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=0;
		}
		else if(Flags&OGL_CORE31)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=3;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=1;
		}
		else if(Flags&OGL_CORE32)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=3;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=2;
		}
		else if(Flags&OGL_CORE33)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=3;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=3;
		}
		else if(Flags&OGL_CORE40)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=4;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=0;
		}
		else if(Flags&OGL_CORE41)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=4;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=1;
		}
		else if(Flags&OGL_CORE42)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=4;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=2;
		}
		else if(Flags&OGL_CORE43)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=4;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=3;
		}
		else if(Flags&OGL_CORE44)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=4;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=4;
		}
		else if(Flags&OGL_CORE45)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=4;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=5;
		}
		else if(Flags&OGL_CORE46)
		{
			ContextAttribs[i++]=WGL_CONTEXT_MAJOR_VERSION_ARB;	ContextAttribs[i++]=4;
			ContextAttribs[i++]=WGL_CONTEXT_MINOR_VERSION_ARB;	ContextAttribs[i++]=6;
		}

		ContextAttribs[i++]=WGL_CONTEXT_FLAGS_ARB;
		ContextAttribs[i++]=WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
		ContextAttribs[i++]=WGL_CONTEXT_PROFILE_MASK_ARB;
		ContextAttribs[i++]=WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		ContextAttribs[i++]=0;

		if(!(Context->hRC=wglCreateContextAttribsARB(Context->hDC, 0, ContextAttribs)))
		{
			DBGPRINTF("wglCreateContextAttribsARB Failed!");
			return 0;
		}
	}
	else
	{
		DBGPRINTF("WGL_ARB_create_context not supported, attempting old wglCreateContext...");

		if(!(Context->hRC=wglCreateContext(Context->hDC)))
		{
			DBGPRINTF("wglCreateContext Failed!");
			return 0;
		}

		return 0;
	}

	if(!wglMakeCurrent(Context->hDC, Context->hRC))
	{
		DBGPRINTF("wglMakeCurrent Failed!");
		return 0;
	}

	return 1;
#else
	int i, Attrib[128];
	int ContextAttribs[8];
	GLXFBConfig *fbconfig=NULL;

	if(!GLXExtensionsInitialized_Flag)
		GLXSetupExtensions(dpy);

	i=0;
	Attrib[i++]=GLX_X_RENDERABLE,
	Attrib[i++]=True;
	Attrib[i++]=GLX_DRAWABLE_TYPE;
	Attrib[i++]=GLX_WINDOW_BIT;
	Attrib[i++]=GLX_RENDER_TYPE;
	Attrib[i++]=GLX_RGBA_BIT;

	if(Flags&OGL_SINGLEBUFFER)
	{
		Attrib[i++]=GLX_DOUBLEBUFFER;
		Attrib[i++]=False;
	}

	if(Flags&OGL_DOUBLEBUFFER)
	{
		Attrib[i++]=GLX_DOUBLEBUFFER;
		Attrib[i++]=True;
	}

	Attrib[i++]=GLX_BUFFER_SIZE;
	Attrib[i++]=Color;
	Attrib[i++]=GLX_DEPTH_SIZE;
	Attrib[i++]=Depth;
	Attrib[i++]=GLX_STENCIL_SIZE;
	Attrib[i++]=Stencil;

	if(GLX_ARB_multisample_Flag)
	{
		if(Flags&OGL_SAMPLES2X)
		{
			Attrib[i++]=GLX_SAMPLE_BUFFERS_ARB;
			Attrib[i++]=True;
			Attrib[i++]=GLX_SAMPLES_ARB;
			Attrib[i++]=2;
		}

		if(Flags&OGL_SAMPLES4X)
		{
			Attrib[i++]=GLX_SAMPLE_BUFFERS_ARB;
			Attrib[i++]=True;
			Attrib[i++]=GLX_SAMPLES_ARB;
			Attrib[i++]=4;
		}

		if(Flags&OGL_SAMPLES6X)
		{
			Attrib[i++]=GLX_SAMPLE_BUFFERS_ARB;
			Attrib[i++]=True;
			Attrib[i++]=GLX_SAMPLES_ARB;
			Attrib[i++]=6;
		}

		if(Flags&OGL_SAMPLES8X)
		{
			Attrib[i++]=GLX_SAMPLE_BUFFERS_ARB;
			Attrib[i++]=True;
			Attrib[i++]=GLX_SAMPLES_ARB;
			Attrib[i++]=8;
		}
	}

	Attrib[i++]=None;

	if((fbconfig=glXChooseFBConfig(dpy, DefaultScreen(dpy), Attrib, &i))==NULL)
	{
		DBGPRINTF("Error: glXChooseFBConfig failed\n");
		return 0;
	}

	if(Flags&OGL_CORE33)
	{
		if(GLX_ARB_create_context_Flag)
		{
			i=0;
			ContextAttribs[i++]=GLX_CONTEXT_MAJOR_VERSION_ARB;
			ContextAttribs[i++]=3;
			ContextAttribs[i++]=GLX_CONTEXT_MINOR_VERSION_ARB;
			ContextAttribs[i++]=3;
			ContextAttribs[i++]=GLX_CONTEXT_FLAGS_ARB;
			ContextAttribs[i++]=GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
			ContextAttribs[i++]=None;

			if((Context->ctx=glXCreateContextAttribsARB(dpy, fbconfig[0], NULL, True, ContextAttribs))==NULL)
			{
				DBGPRINTF("glXCreateContextAttribsARB failed.\n");
				return 0;
			}
		}
		else
		{
			DBGPRINTF("Error: Requested OpenGL 3.3 CORE, GLX_ARB_create_context not supported!\n");
			return 0;
		}
	}
	else
	{
		if((Context->ctx=glXCreateNewContext(dpy, fbconfig[0], GLX_RGBA, NULL, True))==NULL)
		{
			DBGPRINTF("Error: glXCreateContext failed\n");
			return 0;
		}
	}

	glXGetFBConfigAttrib(dpy, fbconfig[0], GLX_VISUAL_ID, &Context->visualid);

	if(!GLExtensionsInitialized_Flag)
	{
		glXMakeCurrent(dpy, ((_XPrivDisplay)dpy)->screens[0].root, Context->ctx);
		GLSetupExtensions();
		glXMakeCurrent(dpy, None, NULL);
	}

	XFree(fbconfig);

	return 1;
#endif
}

void DestroyContext(GLContext_t *Context)
{
#ifdef WIN32
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(Context->hRC);
	ReleaseDC(Context->hWnd, Context->hDC);
#else
	glXDestroyContext(dpy, Context->ctx);
#endif

	memset(Context, 0, sizeof(GLContext_t));
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
