#pragma once
#ifndef _MAP_H_
#define _MAP_H_

#include "common.h"

#include <Windows.h>
#include <process.h>
#include "Block.h"
#include <cstdlib>
#include <map>
#include <queue>

#define CHUNK_W 16
#define CHUNK_L 16
#define CHUNK_H 128



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
	unsigned short loaded:1;
	unsigned short failed:1;
	unsigned short unneeded:1;
	Block *blocks;
};

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

template <class T>
class ThreadSafeQueue : public std::queue<T> {

};

typedef std::map<int3, map_chunk *, id_compare> chunk_list;

class Map {
public :
	Map() {
		counter = 0;
		m_Thread.Start();
	}
	~Map();

	
	int counter;
	chunk_list m_chunks;

	class MapChunkThread {
	private:
		bool active;
		bool queue_lock;
	public:
		std::queue<map_chunk *> jobs;
		HANDLE handle;

		MapChunkThread() {
			queue_lock = false;
			active = false;
		}

		~MapChunkThread() {
			// kill the thread
			active = false;
			// wait for thread to complete
			WaitForSingleObject(handle, INFINITE);
			CloseHandle(handle);
		}

		void Start() {
			if (active == true)
				return;
			active = true;
			// create thread and run it
			handle = (HANDLE)_beginthread(MapChunkThread::threadLoop, 0, this);
		}

		void PushJobs(map_chunk *chk) {
			jobs.push(chk);
		}

		void threadLoadChunk(map_chunk *chk);
		static void threadLoop(void *param);
	}m_Thread;

	void DeleteChunk(map_chunk *chk);
	int ChunkFileExists(int3 id);
	void LoadChunk(int3 id, int urgent);
	map_chunk *Map::CreateChunkFromFile(int3 id);
	map_chunk *LoadChunk(int3 id);
	void LoadNeededChunks(float3 pos, float3 dir);
	void MarkUnneededChunks(float3 pos, float3 dir);
	void DiscardUnneededChunks();
	chunk_list *GetChunkList();
	void PrintChunkStatistics(char *buffer);
};



#endif