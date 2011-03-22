
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Render.h"
#include "Map.h"
#include "File.h"

map_chunk *Map::GetChunk(int3 id) {
	chunk_list::iterator it = m_chunks.find(id);
	if (it == m_chunks.end())
		return 0;

	map_chunk *mapchk = it->second;

	if (mapchk->loaded == 0 || mapchk->failed == 1)
		return 0;

	return mapchk;
}

void Map::DeleteChunk(map_chunk *chk) {
	if (chk == 0)
		return;

	if (chk->loaded == 0 && chk->failed == 0)
		return;

	if (chk->blocks) {
		delete[] chk->blocks;
		chk->blocks = 0;
	}
	delete chk;
	chk = 0;
}

Map::~Map() {
	if (m_Thread != 0) {
		m_Thread->End();
		delete m_Thread;
		m_Thread = 0;
	}

	chunk_list::iterator it;

	for (it = m_chunks.begin(); it != m_chunks.end(); ) {
		DeleteChunk((*it).second);
		m_chunks.erase(it++);
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

	snprintf(buffer, 128, "Failed: %d Loaded:%d Unneeded:%d Total:%d", failed, loaded, unneeded, total);
}

void Map::MarkUnneededChunks(float3 pos, float3 dir) {
	int3 id((int)floor(pos.x / (BLOCK_LEN*CHUNK_W)), 
		(int)floor(pos.y / (BLOCK_LEN*CHUNK_L)), 
		(int)floor(pos.z / (BLOCK_LEN*CHUNK_H))); // Currently standing on this chunk

	chunk_list::iterator it;

	for (it = m_chunks.begin(); it != m_chunks.end(); ++it) {
		int3 idchk = (*it).first;
		map_chunk *tmp = (*it).second;
		if (abs(idchk.x - id.x) > 14 || abs(idchk.y - id.y) > 14 || abs(idchk.z - id.z) > 1) {
			if (tmp->loaded == 1 || tmp->failed == 1)
				tmp->unneeded = 1;
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
			m_chunks.erase(it++);
		}
		else if (tmp->loaded == 0) { // loading
			++it;
		}
		else if (tmp->unneeded == 1) {
			DeleteChunk(tmp);
			m_chunks.erase(it++);
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
	for (int i=-14; i<=14; i++) {
		for (int j=-14; j<=14; j++) {
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

extern FileMgr *s_File;
int Map::ChunkFileExists(int3 id) {
	if (s_File == 0) {
		MessageBox(0, "FileMgr not yet initialized!!", "haha", 0);
		return 0;
	}

	return s_File->QueryChunk(id);
}

map_chunk *Map::CreateEmptyChunk() {
	map_chunk *chk = 0;
	chk = new map_chunk();

	if (chk == 0)
		return 0;

	chk->id = int3(0, 0, 0);
	chk->failed = 0;
	chk->unneeded = 0;
	chk->loaded = 0;
	chk->blocks = 0;
	chk->modified = 0;

	return chk;
}

void Map::LoadChunk(int3 id, int urgent) {
#ifdef APPLE
	// mac doesn't use multi-thread
	urgent = 1;
#endif
	if (ChunkFileExists(id) == 0) // will need faster file checking
		return;

	if (m_chunks.find(id) != m_chunks.end()) {
		return;
	}
	
	map_chunk *chk = CreateEmptyChunk();

	chk->id = id;

	if (urgent == 0) { // Use multithread
		// insert first to ensure no further insertion into the same place is possible
		m_chunks.insert( std::pair<int3, map_chunk *>(chk->id, chk) );
		m_Thread->PushJobs(chk);
	}
	else { // No multithread, load now
		m_Thread->threadLoadChunk(chk);
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
			Sleep(20);
		}
		else {
			// process jobs
			map_chunk *chk = self->jobs.front();
			self->jobs.pop();
			self->threadLoadChunk(chk);
		}
	}
}

void Map::MapChunkThread::threadLoadChunk(map_chunk *chk) {
	if (chk == 0) // impossible
		return;

	chk->blocks = new Block[CHUNK_W*CHUNK_L*CHUNK_H];

	if (chk->blocks == 0) {
		MessageBox(0, "ha", "ha", 0);
		chk->failed = 1;
		return;
	}

	Block *blocks = chk->blocks;

	char filename[32];
	print_chunk_filename(chk->id, filename);
	
	FILE *fp = OpenFile(filename);
	
	if (fp == 0) {
		chk->failed = 1;
		return;
	}

	unsigned short int type[CHUNK_W*CHUNK_L*CHUNK_H];
	
	chunk_header header;
	fread(&header, 1, sizeof(chunk_header), fp);

	fread(type, 1, sizeof(unsigned short int)*CHUNK_W*CHUNK_L*CHUNK_H, fp);

	fclose(fp);
	
	// blockss
	int i = CHUNK_W*CHUNK_L*CHUNK_H;
	while (i--) {
		unsigned short int t = type[i];
		blocks[i].setType(t);
		blocks[i].hidden = 1;
		blocks[i].outside = 0;
		blocks[i].data = 0;
	}

	/*for_xyz(i, j, k) {
		unsigned short int t = type[_1D(i, j, k)];

		blocks[_1D(i, j, k)].setType(t);

		blocks[_1D(i, j, k)].hidden = 1;

		blocks[_1D(i, j, k)].outside = 0;
	} end_xyz()*/

	chk->modified = 1;
	
	chk->loaded = 1;
}
