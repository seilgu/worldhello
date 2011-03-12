#include "common.h"

#ifdef APPLE
void MessageBox(int handler, const char *message, const char *title, int button)
{
	fprintf(stderr, "Title %s:\n\t%s\n", title, message);
}
void Sleep(int millisec)
{
	sleep(millisec);
}
#endif

float3 float3::operator+(float3 &f2) {
	return float3(x+f2.x, y+f2.y, z+f2.z);
}

float3 float3::operator-(float3 &f2) {
	return float3(x-f2.x, y-f2.y, z-f2.z);
}

float3 float3::operator+=(float3 &f2) {
	x += f2.x; y += f2.y; z += f2.z;
	return *this;
}

float3 float3::operator-=(float3 &f2) {
	x -= f2.x; y -= f2.y; z -= f2.z;
	return *this;
}

float3 float3::operator*(float f2) {
	return float3(x*f2, y*f2, z*f2);
}
float3 float3::operator/(float f2) {
	return float3(x/f2, y/f2, z/f2);
}

bool int3::operator!=(int3 &i2) {
	if (x != i2.x || y != i2.y || z != i2.z)
		return true;
	return false;
}

int3 int3::operator-(int3 &i2) {
	return int3(x - i2.x, y - i2.y, z - i2.z);
}

int3 int3::operator+(int3 &i2) {
	return int3(x + i2.x, y + i2.y, z + i2.z);
}
