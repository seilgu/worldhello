#pragma once
#ifndef _BLOCK_H_
#define _BLOCK_H_

// most efficient if you stuff it to 4N bytes
struct Block {
public:
	static const int NUL = 0;
	static const int CRATE = 1;
	static const int GRASS = 2;
	static const int SOIL = 3;

	unsigned short type;
	unsigned short modified:1;
	//unsigned short active:1;
	unsigned short hidden:1;
	unsigned short opaque:1;

	Block() {
		modified = 0;
		hidden = 0;
		opaque = 0;
		type = Block::NUL;
	}
};

#endif