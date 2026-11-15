#ifndef _OBJECT_MISSION_TMSNOBJCOIN_H_
#define _OBJECT_MISSION_TMSNOBJCOIN_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "enemy/EnemyLight.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TMsnObjCoin;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTMsnObjCoin(TObject*, SetDesc*);
}

// Vtable 0x802634D8
class TMsnObjCoin : public TSetActor {
public:
	TMsnObjCoin(TObject* parent, SetDesc* desc);
	virtual ~TMsnObjCoin();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C the update
	virtual void vfn_10(); // _10 visibility
	virtual void render(); // _1C
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);

	void updateMatrix(); // 0x80197FAC
	void waitTouch();    // 0x801980E4
	void flash();        // 0x80198188
	void fly();          // 0x80198228

	NJS_OBJECT* mModel;        // _64
	NJS_TEXLIST* mTexList;      // _68
	f32 mRadius;         // _6C
	DrawTask* mDrawTask;     // _70
	EnemyLight mLight;   // _74
	ObjDrawState mState2; // _B4
	f32 mMatrix[12];     // _D8
	int mSpin;           // _108 spin per frame
	Vec3f mScale;        // _10C
	u16 mPulse;          // _118
	u8 _11A[2];          // _11A
	ObjShadow mShadow;   // _11C
	TObject* mGen;       // _154
	u16 mKey;            // _158 the key bit
	u8 _15A[2];          // _15A
	ObjRef mRef;         // _15C whoever touched it
	int mState;          // _168
};

extern "C" {
extern const char* gTMsnObjCoinName;
}

#endif
