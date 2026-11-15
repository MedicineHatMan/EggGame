#ifndef _TOBJORANGECLOCKTOWER_H
#define _TOBJORANGECLOCKTOWER_H
struct SetDesc;
struct TObject;

#include "object/ObjAction.h"
#include "Ninja/ninja.h"
#include "stage/SetManager.h"
#include "engine/DrawTask.h"
#include "camera/EventCamRef.h"
#include "Ninja/ninja.h"

class TCamPath;

extern "C" void objHeapFree(void*, void*);

namespace ClockTower {

/**
 * One clock face, a child of the tower. It plays its motion once the tower calls start(). mSorted faces go in
 * draw list 0x22 and are depth sorted; the others go in list 0x21. The name is the DOL's ("ClockTower_TParts").
 *
 * @size 0x70
 */
class TParts : public TObject {
public:
	TParts(TObjAction* parent, NJS_OBJECT* model, NJS_TEXLIST* texture, NJS_MOTION* motion, int sorted, f32 speed); // 0x8012B250
	virtual ~TParts();                                                                                              // 0x8012B36C

	virtual void draw();   // _0C 0x8012B438 steps the motion
	virtual void vfn_10(); // _10 0x8012B4A4 hides the draw task with the parent
	virtual void render(); // _1C 0x8012B538

	void place(const Vec3f* pos, f32 rotY); // 0x8012B3D4
	void start();                           // 0x8012B428

	NJS_OBJECT* mModel;    // _20
	NJS_TEXLIST* mTexture; // _24
	NJS_MOTION* mMotion;   // _28
	DrawTask* mDrawTask;   // _2C
	f32 mFrame;            // _30
	f32 mSpeed;            // _34
	f32 mMatrix[12];       // _38
	int mSorted;           // _68
	u32 mBits;             // _6C bit 0 = playing
};

/** One lamp of the ring a copy of a model node with its texture list */
struct LampLight {
	NJS_OBJECT mObject;    // _00 a copy of the node its sibling cleared
	NJS_TEXLIST* mTexture; // _38
	u32 mBits;             // _3C bit 0 = lit
};

/**
 * The ring of nine lamps on the tower, a child of the tower. Lit lamps draw with a colour that cycles through
 * eight colours once the tower calls start(). The name is the DOL's ("ClockTower_TLamp").
 *
 * @size 0x6C
 */
class TLamp : public TObject {
public:
	enum { LAMP_COUNT = 9 };

	TLamp(TObjAction* parent); // 0x8012B5D8
	virtual ~TLamp();          // 0x8012B858

	virtual void draw();   // _0C 0x8012BBE4
	virtual void vfn_10(); // _10 0x8012BC10
	virtual void render(); // _1C 0x8012BD6C

	static void setColor(GXColor* color);                             // 0x8012B5A8
	static GXColor* getColor();                                       // 0x8012B5D0
	static void setupTev(int stage, int coord, int map, int channel); // 0x8012BC98 the TEV callback for lit lamps

	void place(const Vec3f* pos, f32 rotY); // 0x8012B8D0
	int isLit(u16 index);                   // 0x8012B924
	void light(u16 index);                  // 0x8012B950
	void lightAll();                        // 0x8012B98C
	void clearAll();                        // 0x8012B9D0
	void start();                           // 0x8012BA14
	void cycle();                           // 0x8012BA24

	LampLight* mLights;  // _20 LAMP_COUNT from geo model 7
	LampLight* mGlows;   // _24 LAMP_COUNT from geo model 8
	DrawTask* mDrawTask; // _28
	f32 mMatrix[12];     // _2C
	GXColor mColor;      // _5C
	f32 mPhase;          // _60 0..8 through the colour table
	f32 mSpeed;          // _64
	u32 mBits;           // _68 bit 0 = cycling
};

} // namespace ClockTower

/**
 * The orange clock tower. Set int 0 is the first switch; the next three switches follow it. Five faces
 * and a bell are separate objects. Act 0 waits, act 1 plays one face, act 2 plays the chime, act 3 is
 * finished. Message 199 picks which face (0..3) the clock sequence may open.
 *
 * @size 0x10C
 */
class TObjOrangeClockTower : public TObjAction {
public:
	TObjOrangeClockTower(TObject* parent, SetDesc* desc);
	virtual ~TObjOrangeClockTower();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual Vec3f* getPos() { return &mPos; } // _20 0x8012BFE4
	virtual int getRotY() { return mRot.y; }  // _28 0x8012BFEC
	virtual int getKind() { return 3; }       // _2C 0x8012BFF4
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual int sendToHandle(ObjMsg* msg);    // _4C

	void setup();
	int switchSet(int index);
	int switchesSet();
	int faceReady(u16 index);
	int facesReady();
	void setAct(int act);
	void stateWait();
	void stateFace();
	void stateChime();
	void stateDone();

	ClockTower::TParts* mParts[5]; // _30 the five faces The last one faces the other way
	ClockTower::TLamp* mLamp;      // _44
	SetLinkMember mLink;           // _48
	DrawTask* mDrawTask;           // _54
	f32 mMatrix[12];               // _58
	Vec3f mPos;                    // _88
	SetRot mRot;                   // _94
	u16 mSwitch;                   // _A0 set int 0
	u16 mSwitchEnd;                // _A2 mSwitch + 3
	EventCamRef mClock;            // _A4 the clock sequence's event camera
	int mSlot;                     // _AC message 199
	void* mFx;                     // _B0
	TCamPath* mChime;              // _B4 the event camera over the chime
	f32 _B8;
	f32 _BC;
	f32 _C0;
	u8 _C4[0xC];
	s16 mHand; // _D0 which face is playing or -1 for the chime
	u16 _D2;
	f32 mTimer;    // _D4
	Vec3f mTop;    // _D8
	f32 mRadius;   // _E4
	int mIds[4];   // _E8 set ints 0..3
	f32 mRates[4]; // _F8 set floats 0..3
	int mAct;      // _108
};

extern "C" {
TObject* createTObjOrangeClockTower(TObject*, SetDesc*);
}

#include "stage/Stage.h"
extern "C" {
}

#endif
