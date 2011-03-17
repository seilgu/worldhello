#ifndef APPLE

#include "glext.h"

PFNGLGENBUFFERSARBPROC glGenBuffersARB;                     // VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC glBindBufferARB;                     // VBO Bind Procedure
PFNGLBUFFERDATAARBPROC glBufferDataARB;                     // VBO Data Loading Procedure
PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;               // VBO Sub Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;               // VBO Deletion Procedure
PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB; // return various parameters of VBO
PFNGLMAPBUFFERARBPROC glMapBufferARB;                       // map VBO procedure
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;                   // unmap VBO procedure
#else
#include <OpenGL/glext.h>
#endif

#include <math.h>
#include "common.h"
#include "texture.h"
#include "Render.h"

#define WIDTH	1200
#define HEIGHT	800
#define PI2			6.2831853f

float fovY = 45.0f;
float zNear = 1.0f;

int _width	= WIDTH;
int _height = HEIGHT;

#ifndef APPLE
HDC			hDC = 0;
HWND		hWnd = 0;
HINSTANCE	hInst = 0;
HGLRC		hRC = 0, hRC2 = 0;
HINSTANCE	hInstance = 0;
BOOL		active = TRUE;

BOOL		keys[256];

LARGE_INTEGER lastTick, currTick;
#endif
double tickFreq;
int captureMouse = 1;

GLuint				box;
GLuint				base;
GLuint				texture[1];							// Crate.bmp texture

#ifndef APPLE
GLYPHMETRICSFLOAT	gmf[256];
int DrawGLScene();
#else
void DrawGLScene();
#endif

bool supportVBO;

GLvoid ReSizeGLScene(GLsizei, GLsizei);
BOOL InitGL();

#ifndef APPLE
BOOL CALLBACK wp(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


void setVSync(int interval=1)
{
	typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;
	const char *extensions = (char *)glGetString( GL_EXTENSIONS );

	if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 ) {
		MessageBox(0, "NO SWAP CONTROL", "haha", 0);
		return; // Error: WGL_EXT_swap_control extension not supported on your computer.\n");
	}
	else {
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

		if( wglSwapIntervalEXT )
			wglSwapIntervalEXT(interval);
	}
}

GLvoid BuildFont(GLvoid)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID

	base = glGenLists(256);								// Storage For 256 Characters

	font = CreateFont(	-12,							// Height Of Font
		0,								// Width Of Font
		0,								// Angle Of Escapement
		0,								// Orientation Angle
		FW_BOLD,						// Font Weight
		FALSE,							// Italic
		FALSE,							// Underline
		FALSE,							// Strikeout
		ANSI_CHARSET,					// Character Set Identifier
		OUT_TT_PRECIS,					// Output Precision
		CLIP_DEFAULT_PRECIS,			// Clipping Precision
		ANTIALIASED_QUALITY,			// Output Quality
		FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
		"Verdana");				// Font Name

	SelectObject(hDC, font);							// Selects The Font We Created

	wglUseFontOutlines(	hDC,							// Select The Current DC
		0,								// Starting Character
		255,							// Number Of Display Lists To Build
		base,							// Starting Display Lists
		0.0f,							// Deviation From The True Outlines
		0.0f,							// Font Thickness In The Z Direction
		WGL_FONT_POLYGONS,				// Use Polygons, Not Lines
		gmf);							// Address Of Buffer To Recieve Data
}

GLvoid KillFont(GLvoid)									// Delete The Font
{
	glDeleteLists(base, 256);								// Delete All 256 Characters
}

GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	float		length=0;								// Used To Find The Length Of The Text
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf_s(text, sizeof(text), fmt, ap);
	va_end(ap);											// Results Are Stored In Text

	for (unsigned int loops=0;loops<(strlen(text));loops++)	// Loop To Find Text Length
	{
		length+=gmf[text[loops]].gmfCellIncX;			// Increase Length By Each Characters Width
	}

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base);									// Sets The Base Character to 0
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}

bool CheckVBOSupport() {
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
	{
		return true;
	} else {
		MessageBox(0, "VBO not supported", "haha", 0);
		return false;
    }
}
#endif

GLuint blockDisplayList;
void CompileDisplayLists() {
	blockDisplayList = glGenLists(256);

	// 256 types of blocks, not textures
	for (int i=0; i<256; i++) {
		glNewList(blockDisplayList + i, GL_COMPILE);
		
		GLuint side_texture;

		for (int w=0; w<6; w++) {
			switch (i) {
			case Block::CRATE: // Draw a crate
				side_texture = TextureMgr::CRATE;
				break;
			case Block::GRASS: // Draw a grass
				switch (w) {
				case Render::PZ: side_texture = TextureMgr::GRASS_TOP; break;
				case Render::NZ: side_texture = TextureMgr::GRASS_BUTTOM; break;
				default: side_texture = TextureMgr::GRASS_SIDE; break;
				}
				break;
			case Block::SOIL:
				side_texture = TextureMgr::SOIL;
				break;
			default: break;
			}

			float2 coord;
			float csize = 1/16.0f;
			coord = get_texture_coord(side_texture);

			glBegin(GL_QUADS);
			switch (w) {
			case Render::PZ: // top
				glTexCoord2f(coord.x, coord.y);				glVertex3f(0.0f, 0.0f, 1.0f);
				glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(1.0f, 0.0f, 1.0f);
				glTexCoord2f(coord.x+csize, coord.y+csize); glVertex3f(1.0f, 1.0f, 1.0f);
				glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(0.0f, 1.0f, 1.0f);
				break;
			case Render::NZ: // buttom
				glTexCoord2f(coord.x, coord.y);				glVertex3f(1.0f, 0.0f, 0.0f);
				glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(0.0f, 0.0f, 0.0f);
				glTexCoord2f(coord.x+csize, coord.y+csize); glVertex3f(0.0f, 1.0f, 0.0f);
				glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(1.0f, 1.0f, 0.0f);
				break;
			case Render::PY: // +y
				glTexCoord2f(coord.x, coord.y);				glVertex3f(1.0f, 1.0f, 0.0f);
				glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(0.0f, 1.0f, 0.0f);
				glTexCoord2f(coord.x+csize, coord.y+csize);	glVertex3f(0.0f, 1.0f, 1.0f);
				glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(1.0f, 1.0f, 1.0f);
				break;
			case Render::NY: // -y
				glTexCoord2f(coord.x, coord.y);				glVertex3f(0.0f, 0.0f, 0.0f);
				glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(1.0f, 0.0f, 0.0f);
				glTexCoord2f(coord.x+csize, coord.y+csize);	glVertex3f(1.0f, 0.0f, 1.0f);
				glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(0.0f, 0.0f, 1.0f);
				break;
			case Render::PX: // +x
				glTexCoord2f(coord.x, coord.y);				glVertex3f(1.0f, 0.0f, 0.0f);
				glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(1.0f, 1.0f, 0.0f);
				glTexCoord2f(coord.x+csize, coord.y+csize);	glVertex3f(1.0f, 1.0f, 1.0f);
				glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(1.0f, 0.0f, 1.0f);
				break;
			case Render::NX: // -x
				glTexCoord2f(coord.x, coord.y);				glVertex3f(0.0f, 0.0f, 0.0f);
				glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(0.0f, 0.0f, 1.0f);
				glTexCoord2f(coord.x+csize, coord.y+csize); glVertex3f(0.0f, 1.0f, 1.0f);
				glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(0.0f, 1.0f, 0.0f);
				break;
			}
			glEnd();
		}
		glEndList();
	}
}

void DeleteDisplayLists() {
	glDeleteLists(blockDisplayList, 256);
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
		height=1;										// Making Height Equal One

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();

	//glOrtho(0, width, 0, height, -1, 1);
	//gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 10.0f, 1000.0f);
	gluPerspective(fovY, (GLfloat)width/(GLfloat)height, zNear, 1000.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}


BOOL InitGL()
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	/*GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { 1.0, 1.0, 4.0, 0.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);*/

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glEnable( GL_MULTISAMPLE_ARB );
    //glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE_ARB );
	//glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_POINT_SMOOTH);
	glEnable(GL_DEPTH_TEST); // required for smooth lines!
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
#ifndef APPLE
	BuildFont();

	setVSync(1);
#endif

	return TRUE;
}
#ifndef APPLE
BOOL CreateGLWindow()
{
	GLuint	PixelFormat;
	WNDCLASS	wnd;
	HINSTANCE	hInst = GetModuleHandle(0);
	RECT	wndRect;

	wndRect.left = 0;
	wndRect.top = 0;
	wndRect.right = WIDTH;
	wndRect.bottom = HEIGHT;

	wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wnd.lpfnWndProc = WndProc;
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.hInstance = hInst;
	wnd.hIcon = LoadIcon(0, IDI_WINLOGO);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hbrBackground = 0;
	wnd.lpszMenuName = 0;
	wnd.lpszClassName = "OpenGL";

	if (!RegisterClass(&wnd))
	{
		MessageBox(0, "Fail to register", "ERROR", MB_OK);
		return FALSE;
	}

	AdjustWindowRectEx(&wndRect, WS_OVERLAPPEDWINDOW, 0, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);	// Important!

	if (!(hWnd = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, 
		"OpenGL", 
		"Title", 
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
		0, 0, 
		wndRect.right - wndRect.left, wndRect.bottom - wndRect.top,					// Important!
		0, 
		0, 
		hInst, 
		0)))
	{
		MessageBox(0, "Fail to Create", "ERROR", MB_OK);
		return FALSE;
	}

	static PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR), 
		1, 
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_TYPE_RGBA, 
		32, 
		0, 0, 0, 0, 0, 0, 
		0, 
		0, 
		0, 
		0, 0, 0, 0, 
		16, 
		0, 
		0, 
		PFD_MAIN_PLANE, 
		0, 
		0, 0, 0
	};

	if (!(hDC = GetDC(hWnd)))
	{
		MessageBox(0, "Fail to GetDC", "ERROR", MB_OK);
		return FALSE;
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
	{
		MessageBox(0, "Fail to get pixel format", "ERROR", MB_OK);
		return FALSE;
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))
	{
		MessageBox(0, "Fail to set pixel format", "ERROR", MB_OK);
		return FALSE;
	}

	if (!(hRC = wglCreateContext(hDC)))
	{
		MessageBox(0, "Fail to create context", "ERROR", MB_OK);
		return FALSE;
	}

	if (!(hRC2 = wglCreateContext(hDC)))
	{
		MessageBox(0, "Fail to create context", "ERROR", MB_OK);
		return FALSE;
	}

	wglShareLists(hRC, hRC2);

	if (!wglMakeCurrent(hDC, hRC))
	{
		MessageBox(0, "Fail to make current", "ERROR", MB_OK);
		return FALSE;
	}

	return TRUE;
}
#endif
