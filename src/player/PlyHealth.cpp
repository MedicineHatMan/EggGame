#include "player/Player.h"

// 0x8003F6C8
extern "C" void plyHealthInit(PlyHealth* health)
{
	health->mChanged = 0;
}

// 0x8003F6D4
extern "C" void plyHealthSet(PlyHealth* health, int value)
{
	if (value > health->mMax) {
		value = health->mMax;
	} else if (value < health->mMin) {
		value = health->mMin;
	}
	if (health->mNow != value) {
		health->mChanged = 1;
		health->mNow     = value;
	}
}

// 0x8003F714
extern "C" void plyClearWord(u32* word)
{
	*word = 0;
}

// 0x8003F720
// Counts a frame timer down to zero
extern "C" void plyCountDown(u16* timer)
{
	if (*timer == 0) {
		return;
	}
	(*timer)--;
}
