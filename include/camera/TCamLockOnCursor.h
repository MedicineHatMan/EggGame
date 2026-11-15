#ifndef _TCAMLOCKONCURSOR_H
#define _TCAMLOCKONCURSOR_H
struct TArc;
#include "core/types.h"
struct LockOnSize;
struct DrawTask;
struct LockOnUV;

#include "stage/Stage.h"

class TObjAction;

// TCamLockOnCursor.cpp TCameraCtrl keeps one per player

/** One lock-on mark an object in range where it is and how far */
struct LockOnMark { // size 0x14
	LockOnMark() { }
	void reset(); // 0x8002FAFC

	// Copies field by field which the DOL's lfs/stfs pairs show
	LockOnMark& operator=(const LockOnMark& other)
	{
		mId    = other.mId;
		mPos.x = other.mPos.x;
		mPos.y = other.mPos.y;
		mPos.z = other.mPos.z;
		mDist  = other.mDist;
		return *this;
	}

	u16 mId;    // _00 the object's id 0xFFFF when unused
	Vec3f mPos; // _04 where the mark goes the object's position raised by its height
	f32 mDist;  // _10 the distance along the ground for the locked one
};

/**
 * A player's lock-on cursor. Each frame it gathers the ten nearest objects in front of its player into mMark, nearest
 * first, and keeps mCurrent while it stays within mKeep. It draws a mark on each, the locked one bigger.
 *
 * @size 0x12C
 */
class TCamLockOnCursor : public TObject {
public:
	enum {
		LOCK_ENABLED = 1, // looks for marks at all
		LOCK_LOCKED  = 2, // mCurrent is locked
	};

	enum {
		LOCK_MARKS = 10,
	};

	TCamLockOnCursor(TObject* parent);
	virtual ~TCamLockOnCursor();
	virtual void draw();   // _0C 0x80030498
	virtual void vfn_10(); // _10 0x80030558 shown only in its own player's view
	virtual void render(); // _1C 0x800308D4

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void init();                                                         // 0x8002FC6C
	void setOwner(u16 id);                                               // 0x8002FCE4
	BOOL hasMarks();                                                     // 0x8002FCF8
	void lock();                                                         // 0x8002FD0C onto the nearest mark
	void unlock();                                                       // 0x8002FD8C
	void nextMark();                                                     // 0x8002FDC8
	void enable();                                                       // 0x8002FE34
	void disable();                                                      // 0x8002FE44
	void setRange(f32 far, int angle, f32 near, f32 keep);               // 0x8002FEA4
	void setHeight(f32 height);                                          // 0x8002FEB8
	void useNormalRange();                                               // 0x8002FEC0
	void useAltRange();                                                  // 0x8002FF08 the same numbers
	void useUnlimitedRange();                                            // 0x8002FF50
	u16 getLockId();                                                     // 0x8002FF98
	void getCurrent(LockOnMark* out);                                    // 0x8002FFA0
	BOOL canTarget(TObjAction* obj);                                     // 0x8002FFCC
	BOOL inRange(f32 dotMin, const Vec3f* fwd, Vec3f* dir, f32* outLen); // 0x80030014
	void scan(const Vec3f* from, int rotY);                              // 0x800300FC
	BOOL project(f32 maxY, const Vec3f* pos, Vec3f* out);                // 0x800305B0
	int markKind(u16 id);                                                // 0x80030668 a player's id else 4
	void drawPlayerMark(const Vec3f* at, int kind, BOOL locked);         // 0x80030680
	void drawOtherMark(const Vec3f* at, int kind, BOOL locked);          // 0x800307B0
	void drawMark(const Vec3f* at, u16 id, BOOL locked);                 // 0x8003086C

	DrawTask* mDraw;              // _20
	void* mTex;                   // _24
	u16 mId;                      // _28 the owner's object id
	int mCount;                   // _2C marks in use
	LockOnMark mMark[LOCK_MARKS]; // _30 nearest first
	int mIndex;                   // _F8 the mark nextMark is on
	LockOnMark mCurrent;          // _FC the locked one
	f32 mFar;                     // _110 nothing farther is marked
	int mAngle;                   // _114 half the angle in front that counts
	f32 mNear;                    // _118 anything nearer counts whatever the angle
	f32 mHeight;                  // _11C nothing more than this above or below counts
	f32 mKeep;                    // _120 the locked one is kept while within this
	u32 mBits;                    // _124 LOCK_ bits
	f32 mBlink;                   // _128 0 to 1 over and over
};

/** A part of the lock-on texture */
struct LockOnUV {
	f32 mU0, mV0, mU1, mV1;
};

/** A mark's size the player marks use mW the others mH */
struct LockOnSize {
	f32 mW, mH;
};

extern "C" {
extern const LockOnUV gLockOnUV[5];
extern const LockOnUV gLockOnFreeUV[10];
extern const LockOnSize gLockOnBigSize;
extern const LockOnSize gLockOnSmallSize;
extern TArc* gIcon3dArc;
}

#endif
