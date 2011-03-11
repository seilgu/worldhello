#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "asmlibran.h"

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

/** from Block.h
	static const int NUL = 0;
	static const int CRATE = 1;
	static const int GRASS = 2;
	static const int SOIL = 3;
**/

// Offset is used to relate relative position in the world
int generate_terrain(short int *terrain, int3 offset, int3 dim) {
	for (int i=0; i<dim.x; i++) {
		for (int j=0; j<dim.y; j++) {
			for (int k=0; k<dim.z; k++) {
				int test = MersenneIRandom(0, 10);

				if (test < 1) {
					terrain[k*dim.x*dim.y + j*dim.x + i] = 3;
				}
				else if (test < 2) {
					terrain[k*dim.x*dim.y + j*dim.x + i] = 2;
				}
				else if (test < 3) {
					terrain[k*dim.x*dim.y + j*dim.x + i] = 1;
				}
				else {
					terrain[k*dim.x*dim.y + j*dim.x + i] = 0;
				}
			}
		}
	}

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
	
	sprintf_s(filename, "../MyCraft/map/%s%s%s.chk", tempi, tempj, tempk);

	FILE *fp = 0;
	fopen_s(&fp, filename, "w");
	fwrite(&chk, 1, sizeof(chunk_header), fp);
	fwrite(terrain, 1, dim.x*dim.y*dim.z*sizeof(short int), fp);
	fclose(fp);

	delete[] terrain;

	return 1;
}

int main(int agrc, char *argv[]) {

	MersenneRandomInit((int)ReadTSC());
	// parameters
	int3 chkid;
	int3 dim(16, 16, 128);
	int world_id = 1;

	for (int i=-5; i<5; i++) {
		for (int j=-5; j<5; j++) {
			for (int k=-5; k<5; k++) {
				chkid.x = i;
				chkid.y = j;
				chkid.z = k;

				generate_chunk(chkid, dim, world_id);
			}
		}
	}

	return 0;
}