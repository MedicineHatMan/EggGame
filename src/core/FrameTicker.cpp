#include "engine/ShadowTex.h"

FrameTicker* sFrameTicker; // 0x802C4DE8

// 0x8004212C
FrameTicker* frameTickerGet()
{
	if (sFrameTicker == 0) {
		sFrameTicker = new FrameTicker;
	}
	return sFrameTicker;
}

// 0x80042174
FrameTicker::FrameTicker()
{
	_08 = 1;
	_0C = 0;
	_10 = 240.0f;
	_14 = 0.0f;
	_00 = 0;
	_04 = 0;
}

// 0x800421AC
FrameTicker::~FrameTicker()
{
}

// 0x800421F4
void FrameTicker::update()
{
}
