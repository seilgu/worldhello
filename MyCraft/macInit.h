
#include <math.h>
#include <OpenGL/glext.h>

#define WIDTH	1200
#define HEIGHT	800
#define PI2			6.2831853f

int _width	= WIDTH;
int _height = HEIGHT;

double tickFreq;
int captureMouse = 1;

void DrawGLScene();
GLvoid ReSizeGLScene(GLsizei, GLsizei);
GLboolean InitGL();

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
		height=1;										// Making Height Equal One

	glViewport(0,0,width,height);						// Reset The Current Viewport
	_width = width;
	_height = height;

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();

	//glOrtho(0, width, 0, height, -1, 1);
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 10000.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

void CheckVBOSupport() {
/*
	glGenBuffersARB = 0;                     // VBO Name Generation Procedure
	glBindBufferARB = 0;                     // VBO Bind Procedure
	glBufferDataARB = 0;                     // VBO Data Loading Procedure
	glBufferSubDataARB = 0;               // VBO Sub Data Loading Procedure
	glDeleteBuffersARB = 0;               // VBO Deletion Procedure
	glGetBufferParameterivARB = 0; // return various parameters of VBO
	glMapBufferARB = 0;                       // map VBO procedure
	glUnmapBufferARB = 0;                   // unmap VBO procedure


	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
	glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
	glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
	glMapBufferARB = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
	glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");

	if(glGenBuffersARB && glBindBufferARB && glBufferDataARB && glBufferSubDataARB &&
			glMapBufferARB && glUnmapBufferARB && glDeleteBuffersARB && glGetBufferParameterivARB)
	{} else {
				MessageBox(0, "VBO not supported", "haha", 0);
			}
			*/
}

GLboolean InitGL()
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_DEPTH_TEST); // required for smooth lines!
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	return true;
}

