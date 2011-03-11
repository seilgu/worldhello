#include "Map.h"
#include <stdio.h>
#include <math.h>

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

map_chunk *Map::LoadChunk(int3 id) {
	return 0;
}

map_chunk *Map::CreateChunkFromFile(int3 id) {
	
	char bx[4], by[4], bz[4];
	char filename[32];
	
	tobase36(id.x, bx);
	tobase36(id.y, by);
	tobase36(id.z, bz);

	sprintf_s(filename, ".\\map\\%s%s%s.chk", bx, by, bz);

	FILE *fp = 0;
	fopen_s(&fp, filename, "r");
	
	if (fp == 0) {
		return 0;
	}

	// Begin reading chunk from file
	map_chunk *chk = new map_chunk();
	if (chk == 0)
		return 0;

	chk->failed = 0;
	chk->loaded = 0;
	chk->unneeded = 0;
	chk->id = id;
	chk->blocks = 0;

	chk->blocks = new Block[CHUNK_W*CHUNK_L*CHUNK_H];

	if (chk->blocks == 0) {
		delete chk;
		return 0;
	}

	chunk_header header;
	fread(&header, 1, sizeof(chunk_header), fp);

	unsigned short int type[CHUNK_W*CHUNK_L*CHUNK_H];
	fread(type, 1, sizeof(unsigned short int)*CHUNK_W*CHUNK_L*CHUNK_H, fp);

	fclose(fp);
	
	// blockss
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				(chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i]).type = type[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i];
			}
		}
	}

	chk->loaded = 1;

	return chk;
}




void Map::DeleteChunk(map_chunk *chk) {
	if (chk == 0)
		return;

	if (chk->loaded == 0 && chk->failed == 0)
		return;

	if (chk->blocks) {
		delete chk->blocks;
		chk->blocks = 0;
	}
	delete chk;
	chk = 0;
}

Map::~Map() {
	chunk_list::iterator it;

	for (it = m_chunks.begin(); it != m_chunks.end(); ) {
		DeleteChunk((*it).second);
		it = m_chunks.erase(it);
	}
}

void Map::PrintChunkStatistics(char *buffer) {
	chunk_list::iterator it;

	int failed = 0, loaded = 0, unneeded = 0, total = 0;
	for (it = m_chunks.begin(); it != m_chunks.end(); ++it) {
		map_chunk *tmp = (*it).second;
		if (tmp->failed)
			failed++;
		if (tmp->unneeded)
			unneeded++;
		if (tmp->loaded)
			loaded++;
		total++;
	}

	sprintf(buffer, "Failed: %d Loaded:%d Unneeded:%d Total:%d", failed, loaded, unneeded, total);
}

void Map::MarkUnneededChunks(float3 pos, float3 dir) {
	int3 id((int)floor(pos.x / (BLOCK_LEN*CHUNK_W)), 
		(int)floor(pos.y / (BLOCK_LEN*CHUNK_L)), 
		(int)floor(pos.z / (BLOCK_LEN*CHUNK_H))); // Currently standing on this chunk

	chunk_list::iterator it;

	for (it = m_chunks.begin(); it != m_chunks.end(); ++it) {
		int3 idchk = (*it).first;
		map_chunk *tmp = (*it).second;
		if (abs(idchk.x - id.x) > 3 || abs(idchk.y - id.y) > 3 || abs(idchk.z - id.z) > 1) {
			if (tmp->loaded == 1 || tmp->failed == 1)
				tmp->unneeded = 1; // BUGGY
		}
	}
}

void Map::DiscardUnneededChunks() { //  ALSO deletes FAILED!!
	chunk_list::iterator it;

	for (it = m_chunks.begin(); it != m_chunks.end(); ) {
		map_chunk *tmp = (*it).second;
		if (tmp == 0) {
			++it;
			continue;
		}

		if (tmp->failed == 1) {
			DeleteChunk(tmp);
			it = m_chunks.erase(it);
		}
		else if (tmp->loaded == 0) { // loading
			++it;
		}
		else if (tmp->unneeded == 1) {
			DeleteChunk(tmp);
			it = m_chunks.erase(it);
		}
		else {
			++it;
		}
	}
}

void Map::LoadNeededChunks(float3 pos, float3 dir) {
	int3 id((int)floor(pos.x / (BLOCK_LEN*CHUNK_W)), 
		(int)floor(pos.y / (BLOCK_LEN*CHUNK_L)), 
		(int)floor(pos.z / (BLOCK_LEN*CHUNK_H))); // Currently standing on this chunk

	int3 tmp;
	
	// these are the needed 5x5 chunks around the player
	for (int i=-3; i<=3; i++) {
		for (int j=-3; j<=3; j++) {
			for (int k=-1; k<=1; k++) {
				// setup chunk ids
				tmp.x = id.x + i;
				tmp.y = id.y + j;
				tmp.z = id.z + k;

				if (m_chunks.find(tmp) == m_chunks.end()) { // not loaded
					LoadChunk(tmp, 0);					
				}
			}
		}
	}
}

int Map::ChunkFileExists(int3 id) {
	char bx[4], by[4], bz[4];
	char filename[32];
	
	tobase36(id.x, bx);
	tobase36(id.y, by);
	tobase36(id.z, bz);

	sprintf_s(filename, ".\\map\\%s%s%s.chk", bx, by, bz);

	FILE *fp = 0;
	fopen_s(&fp, filename, "r");
	
	if (fp == 0) {
		return 0;
	}

	fclose(fp);

	return 1;
}

void Map::LoadChunk(int3 id, int urgent) {

	if (urgent == 0) {
		if (ChunkFileExists(id) == 0) // will need faster file checking
			return;

		map_chunk *chk = 0;
		chk = new map_chunk();
		if (chk == 0) {
			MessageBox(0, "map_chunk alloc failed", "haha", 0);
			return;
		}

		chk->loaded = 0;
		chk->failed = 0;
		chk->blocks = 0;
		chk->unneeded = 0;
		chk->id = id;
		
		m_chunks.insert( std::pair<int3, map_chunk *>(chk->id, chk) );

		m_Thread.PushJobs(chk);
		
	}
	else {
		map_chunk *chk = 0;
		
		chk = CreateChunkFromFile(id);

		if (chk == 0) {
			return;
		}
		
		// insert chunk
		m_chunks.insert( std::pair<int3, map_chunk *>(chk->id, chk) );
	}
}

chunk_list *Map::GetChunkList() {
	return &m_chunks;
}

/*********************** MapChunkThread class ***************************
	Loads shit
*************************************************************************/

void Map::MapChunkThread::threadLoop(void *param) {
	Map::MapChunkThread *self = (Map::MapChunkThread *)param;
	while (self->active) {
		// peek jobs
		if (self->jobs.empty()) {
			Sleep(100);
		}
		else {
			// process jobs
			map_chunk *chk = self->jobs.front();
			self->threadLoadChunk(chk);
			self->jobs.pop();
		}
	}
}

void Map::MapChunkThread::threadLoadChunk(map_chunk *chk) {
	if (chk == 0)
		return;

	chk->failed = 0;
	chk->unneeded = 0;
	chk->loaded = 0;
	chk->blocks = 0;

	char bx[4], by[4], bz[4];
	char filename[32];
	
	tobase36(chk->id.x, bx);
	tobase36(chk->id.y, by);
	tobase36(chk->id.z, bz);

	sprintf_s(filename, ".\\map\\%s%s%s.chk", bx, by, bz);

	FILE *fp = 0;
	fopen_s(&fp, filename, "r");
	
	if (fp == 0) {
		chk->failed = 1;
		return;
	}

	chk->blocks = new Block[CHUNK_W*CHUNK_L*CHUNK_H];

	if (chk->blocks == 0) {
		MessageBox(0, "ha", "ha", 0);
		chk->failed = 1;
		fclose(fp);
		fp = 0;
		return;
	}
	
	unsigned short int type[CHUNK_W*CHUNK_L*CHUNK_H];
	
	chunk_header header;
	fread(&header, 1, sizeof(chunk_header), fp);

	fread(type, 1, sizeof(unsigned short int)*CHUNK_W*CHUNK_L*CHUNK_H, fp);

	fclose(fp);
	fp = 0;
	
	// blockss
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				(chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i]).type = type[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i];
			}
		}
	}

	chk->loaded = 1;
}