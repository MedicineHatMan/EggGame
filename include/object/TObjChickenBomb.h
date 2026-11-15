#ifndef _TOBJCHICKENBOMB_H
#define _TOBJCHICKENBOMB_H
struct SetDesc;
struct TObject;
struct CharaColli;

#include "object/ObjAction.h"
#include "item/TItemManage.h"
#include "engine/CharaColli.h"
#include "object/ObjCommon.h"
#include "engine/DrawTask.h"
#include "item/ItemParts.h"
#include "stage/SetActor.h"
#include "egg/ObjEgg.h"
#include "item/TItem.h"

extern "C" void objHeapFree(void*, void*);
extern "C" void* gObjHeap;

// What the holder's messages carry
struct ChickenBombMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mId;    // _04 the holder
	u16 mNode;  // _06 0x68 the node it hangs from
	Vec3f mVel; // _08 0x71 the throw
};

class TObjChickenBomb : public TObjAction {
public:
	enum {
		BOMB_LYING  = 0, // the fuse burns
		BOMB_HELD   = 1, // hangs from its holder
		BOMB_THROWN = 2, // hops toward the nearest target in front of the thrower
	};

	TObjChickenBomb(TObject* parent, s16 id, const Vec3f* pos); // 0x8018B9D8
	virtual ~TObjChickenBomb();                                 // 0x8018BBC8

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                              // _0C 0x8018BC84
	virtual void vfn_10();                                            // _10 0x8018BD30
	virtual void render();                                            // _1C 0x8018BD98
	virtual Vec3f* getPos();                                          // _20 0x8018C6F4
	virtual int getKind();                                            // _2C 0x8018C6EC
	virtual int sendToHandle(ObjMsg* msg);                            // _4C 0x8018C6FC
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8018C494

	void explode();                    // 0x8018BDDC
	u16 findTarget() const;            // 0x8018BE20
	void run(int state, int event);    // 0x8018C014
	void setState(int state);          // 0x8018C074
	void stateLying(int event);        // 0x8018C0D4
	void stateHeld(int event);         // 0x8018C14C
	void stateThrown(int event);       // 0x8018C208
	int onGrab(ChickenBombMsg* msg);   // 0x8018C4D8 0x67
	int onHang(ChickenBombMsg* msg);   // 0x8018C554 0x68
	int onDrop(ChickenBombMsg* msg);   // 0x8018C59C 0x69
	int onThrow(ChickenBombMsg* msg);  // 0x8018C628 0x71
	int isFlying(ChickenBombMsg* msg); // 0x8018C6C8 0x73

	s8 mState;          // _30
	FlagByte mHang;     // _31 1 = mNode is set
	u16 mNode;          // _32
	Vec3f mDir;         // _34 the throw
	Vec3f mPos;         // _40
	f32 mMtx[12];       // _4C
	DrawTask* mDraw;    // _7C
	CharaColli* mShape; // _80
	ObjShadow mShadow;  // _84
	ObjRef mTarget;     // _BC
	u16 mFuse;          // _C8
	u8 _CA[2];
	ItemHolder mHolder; // _CC
	ItemGlow mGlow;     // _E4
	BombBody mBomb;     // _144
};

typedef char TObjChickenBomb_size_check[(sizeof(TObjChickenBomb) == 0x190) ? 1 : -1];

extern "C" {
TObject* createTObjLastTorch(TObject*, SetDesc*);
}

#endif
