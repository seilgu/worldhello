
#include <math.h>




#define WIDTH	1200
#define HEIGHT	800
#define PI2			6.2831853f

int _width	= WIDTH;
int _height = HEIGHT;

HDC			hDC = 0;
HWND		hWnd = 0;
HINSTANCE	hInst = 0;
HGLRC		hRC = 0;
HINSTANCE	hInstance = 0;
BOOL		active = TRUE;

BOOL		keys[256];

LARGE_INTEGER lastTick, currTick;
double tickFreq;

GLuint				box;
GLuint				base;
GLuint				texture[1];							// Crate.bmp texture
GLYPHMETRICSFLOAT	gmf[256];

int DrawGLScene();
GLvoid ReSizeGLScene(GLsizei, GLsizei);
BOOL InitGL();
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


GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
		height=1;										// Making Height Equal One

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();

	//glOrtho(0, width, 0, height, -1, 1);
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 10000.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}


BOOL InitGL()
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

	BuildFont();

	setVSync(1);

	return TRUE;
}

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

	if (!wglMakeCurrent(hDC, hRC))
	{
		MessageBox(0, "Fail to make current", "ERROR", MB_OK);
		return FALSE;
	}

	return TRUE;
}
