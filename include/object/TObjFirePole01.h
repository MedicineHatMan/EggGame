#ifndef _TOBJFIREPOLE01_H
#define _TOBJFIREPOLE01_H
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
struct NJS_OBJECT;
#include "engine/DrawTask.h"
#include "sound/SoundHandle.h"

class TParticleGenerator;

enum FirePoleState {
	FIRE_POLE_WAIT   = 0, // unlit for mWait frames
	FIRE_POLE_BURN   = 1, // lit for mFire frames unless mWait is 0
	FIRE_POLE_DOUSED = 2, // put out by a hit with bit 7
};

enum FirePoleBits {
	FIRE_POLE_HIDDEN   = 1, // only the flame is drawn
	FIRE_POLE_IN_RANGE = 2, // deleted once out of range
	FIRE_POLE_OUT      = 4, // doused TObjArrowPoint watches for it
};

/**
 * A fire pole, set object 0xD. 0x80056CB0-0x80057920. Vtable 0x80255FC0. TObjArrowPoint also raises a hidden one
 * as the flame the player has to put out. Names other than the class name are ours.
 *
 * @size 0xCC
 */
class TObjFirePole01 : public TSetActor {
public:
	TObjFirePole01(TObject* parent, SetDesc* desc);
	TObjFirePole01(TObject* parent);
	virtual ~TObjFirePole01();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                              // _0C
	virtual void vfn_10();                                            // _10 hidden while off camera
	virtual void render();                                            // _1C
	virtual void initFromDesc(SetDesc* desc);                         // _30
	virtual void debugPrint(void*);                                   // _34
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58
	virtual void vfn_74();                                            // _74 sets it up

	void startSound();  // 0x80057234
	void checkDouse();  // 0x80057278
	void stateWait();   // 0x80057360
	void stateBurn();   // 0x80057400
	void stateDoused(); // 0x800574DC
	void placeColli();  // 0x8005764C

	NJS_OBJECT* mModel; // _64
	void* mTexture;     // _68
	int mFrame;         // _6C of 12
	u8 _70[4];
	DrawTask* mDraw;            // _74
	TParticleGenerator* mFlame; // _78 the fire or the smoke once doused
	int mDamage;                // _7C
	u32 mBits;                  // _80 FirePoleBits
	int mState;                 // _84 a FirePoleState
	f32 mScale;                 // _88
	u32 mTick;                  // _8C
	u32 mRebirth;               // _90 frames
	u32 mWait;                  // _94 frames
	u32 mFire;                  // _98 frames
	u32 mStep;                  // _9C frames for each of mFrame or 0 to hold
	SoundHandle mSnd;           // _A0
};

extern "C" {
TObject* createTObjFirePole01(TObject*, SetDesc*);
}

#endif
