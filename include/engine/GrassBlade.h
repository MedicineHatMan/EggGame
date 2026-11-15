#ifndef _GRASSBLADE_H
#define _GRASSBLADE_H

#include "stage/Stage.h"

class TObjAction;

/** As njDrawTexture3DExSetData takes it */
struct GrassVtx {
	f32 x, y, z; // _00
	f32 u, v;    // _0C
	u32 col;     // _14
};

enum GrassBladeFlags {
	BLADE_CUT   = 0x1, // cut down or flattened by a push
	BLADE_RESET = 0x2, // asked to grow back
	BLADE_BENT  = 0x4, // nudged aside
};

enum GrassBladeState {
	BLADE_STAND  = 0, // first frame only then sway
	BLADE_SWAY   = 1,
	BLADE_FALL   = 2,
	BLADE_LYING  = 3,
	BLADE_REGROW = 4,
	BLADE_BEND   = 5,
};

enum GrassBladeEvent {
	BLADE_ENTER  = 0,
	BLADE_CHECK  = 1,
	BLADE_UPDATE = 2,
	BLADE_EXIT   = 3,
};

/**
 * @size 0xCC
 */
class GrassBlade {
public:
	GrassBlade(f32 radius, const Vec3f* center, int index);
	~GrassBlade();

	int isCut();
	int cut(TObjAction* other);
	int push(f32 radius, const Vec3f* pos);
	void bend(TObjAction* other);
	int fall();
	int reset();
	int rise(f32 rate);
	int bendOver();
	void readCameraAngle();
	int hasCameraTurned();
	void placeAll(GrassVtx* src, GrassVtx* dst, const Vec3f* pos, int angX, int angY, int angZ);
	void placeTip(GrassVtx* src, GrassVtx* dst);
	void placeBase(GrassVtx* src, GrassVtx* dst);
	void placeVtx(f32* mtx, GrassVtx* src, GrassVtx* dst, const Vec3f* pos);
	void setState(int state);
	void exitState();
	void changeState(int state);
	void update();
	void runState(int state, int event);
	void stateStand(int event);
	int swayStep(int frame, int frames, int amount);
	void stateSway(int event);
	void stateFall(int event);
	void stateLying(int event);
	void stateRegrow(int event);
	void stateBend(int event);
	void draw();
	void drawFaded(u32 alpha);

	u8 _00[4];
	int mState;        // _04 a GrassBladeState
	int mIndex;        // _08 which blade of the group
	u32 mFlags;        // _0C GrassBladeFlags
	Vec3f mPos;        // _10
	int mDir;          // _1C the way the tip leans
	int mTilt;         // _20 how far the tip leans
	int mTiltTarget;   // _24 for BLADE_BEND
	int mCameraAngle;  // _28 the base faces this way
	GrassVtx mBase[3]; // _2C
	GrassVtx mVtx[3];  // _74
	int mStep;         // _BC the step within the state
	int mFrame;        // _C0
	int mFrames;       // _C4
	int mAmount;       // _C8
};

extern "C" GrassBlade* createGrassBlade(f32 radius, const Vec3f* center, int index);

#endif
