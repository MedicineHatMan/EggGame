#ifndef _PLYINPUT_H
#define _PLYINPUT_H

#include "core/types.h"

enum PlyButton {
	PLY_BTN_A  = 0x1,
	PLY_BTN_B  = 0x2,
	PLY_BTN_R  = 0x4,
	PLY_BTN_R2 = 0x8,  // also R
	PLY_BTN_X  = 0x10,
	PLY_BTN_B2 = 0x20, // also B
	PLY_BTN_Y  = 0x40,
};

/**
 * @size 0x18
 */
struct PlyInput {
	PlyInput();

	void clear();
	void read(int pad, f32* camera);
	void readConfirm(int pad);

	f32 mStick;     // _00 the stick's strength cubed 0 in the dead zone
	int mAngle;     // _04 in world space
	u32 mPressed;   // _08
	u32 mReleased;  // _0C
	u32 mHeld;      // _10
	u32 mNotHeld;   // _14
};

#endif
