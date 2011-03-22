#ifndef APPLE

/*#include "glext.h"

PFNGLGENBUFFERSARBPROC glGenBuffersARB;                     // VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC glBindBufferARB;                     // VBO Bind Procedure
PFNGLBUFFERDATAARBPROC glBufferDataARB;                     // VBO Data Loading Procedure
PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;               // VBO Sub Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;               // VBO Deletion Procedure
PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB; // return various parameters of VBO
PFNGLMAPBUFFERARBPROC glMapBufferARB;                       // map VBO procedure
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;                   // unmap VBO procedure*/
#else
#include <OpenGL/glext.h>
#endif

#include <math.h>
#include "common.h"
#include "texture.h"
#include "Render.h"
#include "glew.h"


#include "World.h"
#include "Player.h"
#include "File.h"
#include "Debug.h"
#include "Shader.h"

#ifndef APPLE
#include "wglew.h"
#endif

#include "Maths/Maths.h"

#define WIDTH	1600
#define HEIGHT	1000
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


void setVSync(int interval=1) {
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

GLvoid BuildFont(GLvoid) {								// Build Our Bitmap Font
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

GLvoid KillFont(GLvoid)	{								// Delete The Font
	glDeleteLists(base, 256);								// Delete All 256 Characters
}

GLvoid glPrint(const char *fmt, ...) {					// Custom GL "Print" Routine
	float		length=0;								// Used To Find The Length Of The Text
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf_s(text, sizeof(text), fmt, ap);
	va_end(ap);											// Results Are Stored In Text

	for (unsigned int loops=0;loops<(strlen(text));loops++)	{// Loop To Find Text Length
		length+=gmf[text[loops]].gmfCellIncX;			// Increase Length By Each Characters Width
	}

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base);									// Sets The Base Character to 0
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
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
			case Block::GRASS:
				switch (w) {
				case Render::PZ: side_texture = TextureMgr::GRASS_TOP; break;
				case Render::NZ: side_texture = TextureMgr::GRASS_BUTTOM; break;
				default: side_texture = TextureMgr::GRASS_SIDE; break;
				}
				break;
			case Block::SOIL:
				side_texture = TextureMgr::SOIL;
				break;
			case Block::STONE:
				side_texture = TextureMgr::STONE;
				break;
			case Block::GOLD_MINE:
				side_texture = TextureMgr::GOLD_MINE;
				break;
			case Block::COAL_MINE:
				side_texture = TextureMgr::COAL_MINE;
				break;
			case Block::COAL:
				side_texture = TextureMgr::COAL;
				break;
			case Block::SAND:
				side_texture = TextureMgr::SAND;
				break;
			case Block::GLASS:
				side_texture = TextureMgr::GLASS;
				break;
			case Block::LAVA:
				side_texture = TextureMgr::LAVA;
				break;
			case Block::SNOW:
				switch (w) {
				case Render::PZ: side_texture = TextureMgr::SNOW_TOP; break;
				case Render::NZ: side_texture = TextureMgr::SNOW_BUTTOM; break;
				default: side_texture = TextureMgr::SNOW_SIDE; break;
				}
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

MATRIX4X4 cameraProjectionMatrix;
MATRIX4X4 lightProjectionMatrix;
MATRIX4X4 cameraViewMatrix;
MATRIX4X4 lightViewMatrix;

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
		height=1;										// Making Height Equal One

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();

	gluPerspective(fovY, (GLfloat)width/(GLfloat)height, zNear, 1000.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

BOOL InitGL()
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	
	//glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_POINT_SMOOTH);
	glEnable(GL_DEPTH_TEST); // required for smooth lines!
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	//// modified : try to create opengl 3.x compatible context
	HGLRC tmpRC;
	if (!(tmpRC = wglCreateContext(hDC)))
	{
		MessageBox(0, "Fail to create context", "ERROR", MB_OK);
		return FALSE;
	}

	if (!wglMakeCurrent(hDC, tmpRC))
	{
		MessageBox(0, "Fail to make current", "ERROR", MB_OK);
		return FALSE;
	}

	int attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3, 
		WGL_CONTEXT_MINOR_VERSION_ARB, 2, 
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, 
		0 
	};

	if (wglewIsSupported("WGL_ARB_create_context") == 1) { // If the OpenGL 3.x context creation extension is available
		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
		hRC = wglCreateContextAttribsARB(hDC, NULL, attributes); // Create and OpenGL 3.x context based on the given attributes
		wglMakeCurrent(NULL, NULL); // Remove the temporary context from being active
		wglDeleteContext(tmpRC); // Delete the temporary OpenGL 2.1 context
		wglMakeCurrent(hDC, hRC); // Make our OpenGL 3.0 context current
	}
	else {
		hRC = tmpRC; // If we didn't have support for OpenGL 3.x and up, use the OpenGL 2.1 context
	}
	////

	wglShareLists(hRC, hRC2);

	return TRUE;
}
#endif


// Game things

Render *s_Render;
World *s_World;
TextureMgr *s_Texture;
Player *m_Player;
FileMgr *s_File;
Shader *s_Shader;

void InitClasses() {
	s_File = new FileMgr();
	s_Render = new Render(supportVBO);
	s_World = new World();
	s_Texture = new TextureMgr();
	m_Player = new Player();
	s_Shader = new Shader();
}

void DeInitClasses() {
	if (s_Render != 0)
		delete s_Render;
	if (s_World != 0)
		delete s_World;
	if (s_Texture != 0)
		delete s_Texture;
	if (m_Player != 0)
		delete m_Player;
	if (s_File != 0)
		delete s_File;
	if (s_Shader != 0)
		delete s_Shader;

	s_Render = 0;
	s_World = 0;
	s_Texture = 0;
	m_Player = 0;
	s_File = 0;
	s_Shader = 0;
}
