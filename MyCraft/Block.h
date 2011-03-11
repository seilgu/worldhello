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

	unsigned short active;
	unsigned short hidden;
	unsigned short type;

	Block() {
		active = 0;
		hidden = 0;
		type = Block::NUL;
	}
};

#endif