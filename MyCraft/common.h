
#ifndef _COMMON_H_
#define _COMMON_H_

#include <math.h>

#define PI 3.1415926f

#define BLOCK_LEN 1.0f

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

inline float3 cross_prod(float3 a, float3 b) {
	return float3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

inline float3 spher2car(float theta, float phi) {
	return float3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
}

inline float dot_prod(float3 a, float3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline int dot_prod(int3 a, int3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline float dot_prod(int3 a, float3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline float length(float3 &f) {
	return sqrt(f.x*f.x + f.y*f.y + f.z*f.z);
}





#endif