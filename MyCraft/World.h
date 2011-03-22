#pragma once
#ifndef _WORLD_H_
#define _WORLD_H_

#include "common.h"
#include "Map.h"

/*************************************************************************
	A singleton class that handles Objects in the World.
	Dynamically loading chunks and object management.
*************************************************************************/

#define WORLD_SIZE 100

class World {
public:
	float gravity;
	float dt;
	World() {
		gravity = -9.8f;
		dt = 0.001f;
	}
	~World() {}

	int LoadWorld();
	void UpdateWorld();
	Block *GetBlocks(int3 id);

	chunk_list *GetRenderChunks(float3 pos, float3 dir);
	
	// Rendering

	Map world_map;
};

#endif
