
#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "common.h"

#define BODY_RECT_W 2.0f
#define BODY_RECT_L 2.0f
#define BODY_RECT_H 9.0f

class Player {
public:
	Player() {}
	~Player() {}

	float3 eyepos;
	float theta, phi;
	float3 dir;

	rect3f body_rect;

};

#endif