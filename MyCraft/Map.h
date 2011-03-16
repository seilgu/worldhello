#pragma once
#ifndef _MAP_H_
#define _MAP_H_

#ifndef APPLE

#include <Windows.h>
#include <process.h>

#else

#endif

#include "common.h"
#include "Block.h"
#include <cstdlib>
#include <map>
#include <queue>




/**************************************************
	Header:
		unsigned int MAGIC = 0x1234 ; 4 bytes
		unsigned int version = 1,	; 4 bytes
		unsigned int world_id,		; 4 bytes
		unsigned int width = 16, 
					length = 16, 
					height = 128,	;12 bytes
		int xoffset, 
			yoffset, 
			zoffset,				;12 bytes
	Data:
		short int terrain[width*length*height];
****************************************************/

struct chunk_header {
	unsigned int MAGIC;
	unsigned int version;
	int world_id;
	int width, length, height;
	int idx, idy, idz;
};

struct map_chunk {
	int3 id;
	unsigned short modified:1;
	unsigned short loaded:1;
	unsigned short failed:1;
	unsigned short unneeded:1;
	Block *blocks;
};

typedef std::map<int3, map_chunk *, id_compare> chunk_list;

class Map {
public :
	Map() {
		m_Thread = new MapChunkThread(this);
		m_Thread->Start();
	}
	~Map();

	chunk_list m_chunks;

	class MapChunkThread {
	private:
		bool active;
	public:
		Map *map;
		std::queue<map_chunk *> jobs;

#ifndef APPLE
		HANDLE handle;
#endif

		MapChunkThread(Map *m) {
			map = m;
			active = false;
		}

		~MapChunkThread() {
			End();
		}

		void End() {
#ifndef APPLE
			if (handle == 0)
				return;
			// kill the thread
			active = false;
			// wait for thread to complete
			WaitForSingleObject(handle, INFINITE);
			CloseHandle(handle);
			handle = 0;
#endif
		}

		void Start() {
#ifndef APPLE
			if (active == true)
				return;
			active = true;
			// create thread and run it
			handle = (HANDLE)_beginthread(MapChunkThread::threadLoop, 0, this);
#endif
		}

		void PushJobs(map_chunk *chk) {
			jobs.push(chk);
		}

		void threadLoadChunk(map_chunk *chk);
		static void threadLoop(void *param);
	};
	MapChunkThread *m_Thread;

	void DeleteChunk(map_chunk *chk);
	int ChunkFileExists(int3 id);
	void LoadChunk(int3 id, int urgent);
	void LoadNeededChunks(float3 pos, float3 dir);
	void MarkUnneededChunks(float3 pos, float3 dir);
	void DiscardUnneededChunks();
	chunk_list *GetChunkList();
	void PrintChunkStatistics(char *buffer);
	map_chunk *CreateEmptyChunk();
	void CalculateVisible(int3 id);
	void CheckChunkSide(int3 id, int dir);
};



#endif
