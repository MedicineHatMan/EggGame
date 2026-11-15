#ifndef _SHADOWTEX_H
#define _SHADOWTEX_H

#include "core/types.h"

struct Vec3f;

/**
 * The drop shadow under the player. begin points a camera down at the player from 3000 above, at 60 degrees, and
 * sets a 128x128 viewport; the stage then draws the shadow casters (draw list 0x1F) and end copies them into frame
 * texture 7. The landscape loads that texture with load and projects it with loadMtx. Its code is at 0x80041E48.
 * The names are ours.
 *
 * @size 0x30
 */
struct ShadowTex {
	ShadowTex();  // 0x80041ECC
	~ShadowTex(); // 0x80041F24

	void begin(Vec3f* at); // 0x80041F70
	void end();            // 0x80042030
	void load(int map);    // 0x80042070
	void loadMtx(int id);  // 0x800420B4

	f32 mView[12]; // _00 the shadow camera's view
};

extern "C" {
ShadowTex* shadowTexGet(); // 0x80041E48
void shadowTexCreate();    // 0x80041E50
void shadowTexDestroy();   // 0x80041E94
}

/**
 * A per-frame object every mode makes on first use and updates before it flushes the frame. update is empty in
 * this build, and nothing reads the fields. Its code is at 0x8004212C. The names are ours.
 *
 * @size 0x1C
 */
struct FrameTicker {
	FrameTicker();

	void update();

	int _00; // _00
	int _04; // _04
	int _08; // _08 1
	int _0C; // _0C
	f32 _10; // _10 240
	f32 _14; // _14

	// _18 = vtable
	virtual ~FrameTicker(); // _08
};

extern "C" FrameTicker* frameTickerGet(); // 0x8004212C

extern "C" {
}

#endif
