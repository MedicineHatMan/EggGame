#ifndef _OBJECT_TOBJITEM_H_
#define _OBJECT_TOBJITEM_H_
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "engine/Fx.h"
#include "item/ItemParts.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/Stage.h"
struct TObjItem;

#include "core/types.h"

class TCamera;
struct Vec3f;

extern "C" {
void camZeldaInit();
void camZeldaTerm();
TCamera* createTCamZelda5(int, int);
void spawnItemDrop(u16, int, Vec3f*);
void spawnItemThrown(u16, int, Vec3f*);
}

class TObjItem : public TObjAction {
public:
	/** What an item is doing mMode */
	enum {
		ITEM_DROP   = 0, // rising out of what dropped it
		ITEM_IDLE   = 1, // lying there to be picked up
		ITEM_THROWN = 2, // flung out by a player letting go
		ITEM_NONE   = 3,
	};

	/** How it moves mFall */
	enum {
		FALL_RISE  = 0, // up toward mRiseTo
		FALL_HOVER = 1, // a moment at the top
		FALL_DROP  = 2, // falling with mBody
		FALL_BOB   = 3, // bobbing where it landed
		FALL_FLUNG = 4, // flung up until it starts to come down
	};

	/** What runMode is called for */
	enum {
		ITEM_ENTER  = 0,
		ITEM_EXIT   = 1,
		ITEM_UPDATE = 2,
		ITEM_CHECK  = 3,
	};

	TObjItem(TObject* parent, int kind, const Vec3f* pos);
	virtual ~TObjItem();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 0x80062DBC culls it
	virtual void render(); // _1C 0x80062E30
	virtual Vec3f* getPos();
	virtual int getKind();

	void updateFall();                 // 0x80062A6C
	void bounce();                     // 0x80062C9C starts mBody falling from where it is
	void runMode(int mode, int event); // 0x80062CFC
	void setMode(int mode);            // 0x80062D5C
	BOOL offer();                      // 0x80062ED8 to what touches it
	void modeDrop(int event);          // 0x80062FBC
	void modeIdle(int event);          // 0x8006308C
	void modeThrown(int event);        // 0x80063194

	s8 mMode;           // _30 ITEM_
	s8 mFall;           // _31 FALL_
	u16 mDelay;         // _32 frames before it can be picked up
	s16 mLife;          // _34
	u16 mFade;          // _36 frames for mSpin to settle
	u16 mTimer;         // _38 the rise's frames then the bob's angle
	DrawTask* mDraw;    // _3C
	Vec3f mPos;         // _40
	Vec3f mVel;         // _4C
	CharaColli* mShape; // _58
	f32 mAngle;         // _5C
	f32 mSpin;          // _60
	f32 mRiseTo;        // _64
	f32 mLift;          // _68
	Fx* mShine;         // _6C the sparkle while it bobs
	f32 mMtx[12];       // _70
	f32 mShineMtx[12];  // _A0
	ItemHolder mItem;   // _D0
	ItemGlow mGlow;     // _E8
	BombBody mBody;     // _148
};

extern "C" {
extern const char* gTObjItemName;
}

#endif
