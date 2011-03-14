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
#else
WORD HueToRGB(WORD n1, WORD n2, WORD hue)
{
	/* range check: note values passed add/subtract thirds of range */ 
	if (hue < 0)
		hue += HLSMAX;

	if (hue > HLSMAX)
		hue -= HLSMAX;

	/* return r,g, or b value from this tridrant */ 
	if (hue < (HLSMAX/6))
		return ( n1 + (((n2-n1)*hue+(HLSMAX/12))/(HLSMAX/6)) );
	if (hue < (HLSMAX/2))
		return ( n2 );
	if (hue < ((HLSMAX*2)/3))
		return ( n1 +    (((n2-n1)*(((HLSMAX*2)/3)-hue)+(HLSMAX/12))/(HLSMAX/6))
		);
	else
		return ( n1 );
}

DWORD HLStoRGB(WORD hue, WORD lum, WORD sat)
{
	WORD R,G,B;                /* RGB component values */ 
	WORD  Magic1,Magic2;       /* calculated magic numbers (really!) */ 

	if (sat == 0) {            /* achromatic case */ 
		R=G=B=(lum*RGBMAX)/HLSMAX;
		if (hue != UNDEFINED) {
			/* ERROR */ 
		}
	}
	else  {                    /* chromatic case */ 
		/* set up magic numbers */ 
		if (lum <= (HLSMAX/2))
			Magic2 = (lum*(HLSMAX + sat) + (HLSMAX/2))/HLSMAX;
		else
			Magic2 = lum + sat - ((lum*sat) + (HLSMAX/2))/HLSMAX;
		Magic1 = 2*lum-Magic2;

		/* get RGB, change units from HLSMAX to RGBMAX */ 
		R = (HueToRGB(Magic1,Magic2,hue+(HLSMAX/3))*RGBMAX +
			(HLSMAX/2))/HLSMAX;
		G = (HueToRGB(Magic1,Magic2,hue)*RGBMAX + (HLSMAX/2)) / HLSMAX;
		B = (HueToRGB(Magic1,Magic2,hue-(HLSMAX/3))*RGBMAX +
			(HLSMAX/2))/HLSMAX;
	}
	return(RGB(R,G,B));
}
#endif

FILE *OpenFile(char *filename) {
	FILE *fp = 0;
#ifndef APPLE
	fopen_s(&fp, filename, "r");
#else
	fp = fopen(filename, "r");
#endif
	return fp;
}

float3 operator*(float c, float3 &f2) {
	return float3(c*f2.x, c*f2.y, c*f2.z);
}

void print_chunk_filename(int3 id, char *dst) {
	char bx[4], by[4], bz[4];
	
	tobase36(id.x, bx);
	tobase36(id.y, by);
	tobase36(id.z, bz);

#ifndef APPLE
	snprintf(dst, 32, ".\\map\\%s%s%s.chk", bx, by, bz);
#else
	snprintf(dst, 32, "./map/%s%s%s.chk", bx, by, bz);
#endif
}

// ugly transform to[4]
void tobase36(int from, char to[]) {
	if (from >= 0) {
		to[0] = '+';
	}
	else {
		to[0] = '-';
		from = -from;
	}

	to[3] = '\0';

	int tmp;
	for (int i=2; i>0; i--) {
		tmp = from%36;
		if (tmp >= 0 && tmp <= 9) {
			to[i] = '0' + tmp;
		}
		else {
			to[i] = 'A' + (tmp - 10);
		}
		from /= 36;
	}
}

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
