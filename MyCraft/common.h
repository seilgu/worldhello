
#ifndef _COMMON_H_
#define _COMMON_H_

#include <math.h>
#include <stdio.h>

#define PI 3.1415926f

#define BLOCK_LEN 1.0f

#define HLSMAX		360
#define RGBMAX		255
#define UNDEFINED	(HLSMAX*2/3)

#ifndef APPLE
#include <Windows.h>
WORD HueToRGB(WORD n1, WORD n2, WORD hue);
DWORD HLStoRGB(WORD hue, WORD lum, WORD sat);
#endif

#ifdef APPLE
//////////////////////////////////  trash area
#include <unistd.h>
#include <OpenGL/glu.h>
typedef GLushort	WORD;
typedef GLuint		DWORD;
typedef GLint		LONG;

void MessageBox(int handler, const char *message, const char *title, int button);
void Sleep(int millisec);
#endif

#ifdef _MSC_VER
#define snprintf sprintf_s
#endif

#define CHUNK_W 16
#define CHUNK_L 16
#define CHUNK_H 128

#define CLAMPW(i) ((i)<0 ? 0 : ((i)>=CHUNK_W ? CHUNK_W-1 : (i)))
#define CLAMPL(i) ((i)<0 ? 0 : ((i)>=CHUNK_L ? CHUNK_L-1 : (i)))
#define CLAMPH(i) ((i)<0 ? 0 : ((i)>=CHUNK_H ? CHUNK_H-1 : (i)))

struct int3;

void tobase36(int from, char to[]);
void print_chunk_filename(int3 id, char *dst);
FILE *OpenFile(char *filename);

struct int2 {
	int x, y;
	int2() : x(0), y(0) {}
	int2(int xx, int yy) : x(xx), y(yy) {}
};

struct int3 {
	int x, y, z;
	int3() : x(0), y(0), z(0) {}
	int3(int xx, int yy, int zz) : x(xx), y(yy), z(zz) {}
	int3 operator-(int3 &i2);
	int3 operator+(int3 &i2);
	bool operator!=(int3 &i2);
};

struct float2 {
	float x, y;
	float2() : x(0), y(0) {}
	float2(float xx, float yy) : x(xx), y(yy) {}
};

struct rect3f {
	float x, y, z;
	float w, l, h;
};

struct float3 {
	float x, y, z;
	float3() : x(0), y(0), z(0) {}
	float3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
	float3 operator+(float3 &f2);
	float3 operator-(float3 &f2);
	float3 operator+=(float3 &f2);
	float3 operator-=(float3 &f2);
	float3 operator*(float f2);
	float3 operator/(float f2);
};

float3 operator*(float c, float3 &f2);

struct double3 {
	double x, y, z;
};

inline float dist2(float x, float y, float z) {
	return x*x + y*y + z*z;
}

inline float dist2(float3 f) {
	return f.x*f.x + f.y*f.y + f.z*f.z;
}

inline void normalize(float3 &v) {
	float len = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	if (len == 0) return;
	v.x /= len;
	v.y /= len;
	v.z /= len;
}

inline float3 cross_prod(float3 &a, float3 &b) {
	return float3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

inline float3 spher2car(float theta, float phi) {
	return float3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
}

inline float dot_prod(float3 &a, float3 &b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline int dot_prod(int3 &a, int3 &b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline float dot_prod(int3 &a, float3 &b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline float length(float3 &f) {
	return sqrt(f.x*f.x + f.y*f.y + f.z*f.z);
}

struct id_compare
{
	bool operator()(int3 a, int3 b) const { // is a < b ?
		if (a.x != b.x)
			return (a.x - b.x < 0);
		else if (a.y != b.y)
			return (a.y - b.y < 0);
		else
			return (a.z - b.z < 0);
	}
};




#endif
