#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <vector>
#include "../MyCraft/Block.h"

#ifndef APPLE
#include "asmlibran.h"
#else
#include <time.h>
int MersenneIRandom(int start, int end)
{
	int len = end-start;
	return rand()%len + end;
}
#endif
struct int3 {
	int3(int xx, int yy, int zz) : x(xx), y(yy), z(zz) {}
	int3() : x(0), y(0), z(0) {}
	int x, y, z;
};


// to[4]
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



#define CHUNK_W 16
#define CHUNK_L 16
#define CHUNK_H 128
#define W 10
#define L 10
#define H 3

#define PI 3.14159f


/********* Chunk discription ***********
	File name : ###_###_###.chk
		###s being id.x, id.y, id.z
		// (+/-) 26+10 base  0123456789 ab

	File content :
		Header:
			unsigned int MAGIC = 0x1234 ; 4 bytes
			unsigned int version = 1,	; 4 bytes
			unsigned int world_id,		; 4 bytes
			unsigned int width = 16, 
						length = 16, 
						height = 128,	;12 bytes
			int idx, idy, idz			;12 bytes
		Data:
			short int terrain[width*length*height];

		// world[offsetx + x][offsety + y][offsetz + z]
			= z*(width*length) + y*(width) + x

		   h|  /l
			| /
			|/____w

****************************************/

struct chunk_header {
	unsigned int MAGIC;
	unsigned int version;
	int world_id;
	int width, length, height;
	int idx, idy, idz;
};

float noise2(int x, int y) {
	int n;
    n = x + y * 57;
    n = (n<<13) ^ n;
    return (float)( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0); 
}

float smooth_noise2(int x, int y) {
	float corners = ( noise2(x-1, y-1)+noise2(x+1, y-1)+noise2(x-1, y+1)+noise2(x+1, y+1) ) / 16;
    float sides   = ( noise2(x-1, y)  +noise2(x+1, y)  +noise2(x, y-1)  +noise2(x, y+1) ) /  8;
    float center  =  noise2(x, y) / 4;

    return corners + sides + center;
}

float Interpolate(float a, float b, float x) {
	float ft = x * 3.1415927f;
	float f = (1 - cos(ft)) * 0.5f;
	
	return a*(1-f) + b*f;
}

float interpolate_noise(float x, float y) {
	int integer_X    = (int)x;
    float fractional_X = x - integer_X;

    int integer_Y    = (int)y;
    float fractional_Y = y - integer_Y;

    float v1 = smooth_noise2(integer_X,     integer_Y);
    float v2 = smooth_noise2(integer_X + 1, integer_Y);
    float v3 = smooth_noise2(integer_X,     integer_Y + 1);
    float v4 = smooth_noise2(integer_X + 1, integer_Y + 1);

    float i1 = Interpolate(v1 , v2 , fractional_X);
    float i2 = Interpolate(v3 , v4 , fractional_X);

    return Interpolate(i1 , i2 , fractional_Y);
}

float Perlin2D(float x, float y) {
	float total = 0;
	
	int octave = 8;
	float persistence = 0.6f;

	for (int i=0; i<octave; i++) {
		float freq = pow(2.0f, i);
		float amp = pow(persistence, i);

		total += interpolate_noise(x*freq, y*freq) * amp;
	}

	return total;
}

// Offset is used to relate relative position in the world
int generate_terrain(short int *terrain, int3 offset, int3 dim) {
	printf("generating chunk (%d, %d, %d)...", offset.x, offset.y, offset.z);
	
	if (offset.z != 0) {
		printf("...done\n");
		return 0;
	}
	
	float height[CHUNK_W][CHUNK_L];

	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			height[i][j] = Perlin2D( (float)(offset.x + i + 1048576)/(W*CHUNK_W), (float)(offset.y + j + 1048576)/(L*CHUNK_L));
			//printf(":::: %f ", noise2(offset.x+i, offset.y+j));
		}
	}

	
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			height[i][j] *= 80;
			height[i][j] += 64.0f;
		}
	}

	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				int3 pos(offset.x + i, offset.y + j, offset.z + k);

				terrain[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i] = Block::NUL;
				if (pos.z + 3 < height[i][j])
					terrain[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i] = Block::STONE;
				else if (pos.z + 1 < height[i][j])
					terrain[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i] = Block::SOIL;
				else if (pos.z < height[i][j]) {
					if (height[i][j] > 84)
						terrain[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i] = Block::SNOW;
					else
						terrain[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i] = Block::GRASS;
				}

				else if (pos.z < 57) {
					terrain[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i] = Block::GLASS;
				}
				else if (pos.z == 57 && pos.z - 1 < height[i][j]) {
					terrain[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i] = Block::SAND;
				}
			}
		}
	}

	printf("done\n");
	return 1;
}

// parameter : chunk's id, dimensions, world_id
int generate_chunk(int3 chkid, int3 dim, unsigned int id) {
	chunk_header chk;

	short int *terrain;
	terrain = 0;
	terrain = new short int[dim.x*dim.y*dim.z];

	if (terrain == 0) {	// allocation failed
		return -1;
	}

	memset(terrain, 0, sizeof(short int)*dim.x*dim.y*dim.z);
	
	int3 offset;
	offset.x = chkid.x*dim.x;
	offset.y = chkid.y*dim.y;
	offset.z = chkid.z*dim.z;

	// set structures
	chk.MAGIC = 0x1234;
	chk.version = 0x0001;
	chk.world_id = id;
	chk.width = dim.x;
	chk.length = dim.y;
	chk.height = dim.z;
	chk.idx = chkid.x;
	chk.idy = chkid.y;
	chk.idz = chkid.z;

	generate_terrain(terrain, offset, dim);

	char filename[64];
	char tempi[4], tempj[4], tempk[4];
	tobase36(chkid.x, tempi);
	tobase36(chkid.y, tempj);
	tobase36(chkid.z, tempk);
	
#ifndef APPLE
	sprintf_s(filename, "../MyCraft/map/%s%s%s.chk", tempi, tempj, tempk);
#else
	snprintf(filename, 64, "../MyCraft/map/%s%s%s.chk", tempi, tempj, tempk);
#endif

	FILE *fp = 0;
#ifndef APPLE
	fopen_s(&fp, filename, "w");
#else
	fp = fopen(filename, "w");
#endif
	fwrite(&chk, 1, sizeof(chunk_header), fp);
	fwrite(terrain, 1, dim.x*dim.y*dim.z*sizeof(short int), fp);
	fclose(fp);

	delete[] terrain;

	return 1;
}

void make_noise() {
	printf("generating noise...");
	printf("done\n\n");
}


int main(int agrc, char *argv[]) {
#ifndef APPLE
	MersenneRandomInit((int)ReadTSC());
#else
	srand(time(0));
#endif


	make_noise();

	// parameters
	int3 chkid;
	int3 dim(16, 16, 128);
	int world_id = 1;

	for (int i=-20; i<20; i++) {
		for (int j=-20; j<20; j++) {
			for (int k=-1; k<2; k++) {
				chkid.x = i;
				chkid.y = j;
				chkid.z = k;

				generate_chunk(chkid, dim, world_id);
			}
		}
	}

	system("pause");

	return 0;
}
