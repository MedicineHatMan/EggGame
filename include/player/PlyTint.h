#ifndef _PLAYER_PLYTINT_H_
#define _PLAYER_PLYTINT_H_
struct PlyTint;

#include "core/types.h"

extern "C" {
void plyTintStop(PlyTint*);
void plyTintUpdate(PlyTint*);
int plyTintIsDone(PlyTint*);
void plyTintFadeA(PlyTint*);
void plyTintFadeB(PlyTint*);
void plyTintPulse(PlyTint*);
}

/** @size 0xC */
struct PlyTint {
	s16 mMode;  // _00 0 pulse 1 and 2 fades
	u16 mTimer; // _02 frames left
	u32 mColor; // _04 the result
	f32 mFade;  // _08 1 down to 0
};

#endif
