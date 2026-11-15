#ifndef _OBJECT_TOBJKEY00_H_
#define _OBJECT_TOBJKEY00_H_
#include "Ninja/ninja.h"
#include "engine/DrawTask.h"
#include "engine/Fx.h"
#include "engine/MeshColli.h"
#include "engine/dlist.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct KeyGlow;
struct KeyList;
struct KeyNode;
struct SetDesc;
struct TObjKey00;
struct TObject;

#include "core/types.h"

extern "C" {
void clearKeyBit(u16);
void clearKeyGroupHigh();
void clearKeyGroupLow();
void clearKeyGroupMid();
void initKeyBits();
void initTObjKey00();
void loadKeyBits(u8*, u16, u32);
void saveKeyBits(u8*, u16, u32);
void setKeySwitch(u16, int);
void termTObjKey00();
}

extern "C" {
void setKeyBit(u16);
int keyBitIsSet(u16);
void keySetWordBit(u32*, int, int);
TObject* createTObjKey00(TObject*, SetDesc*);
}

struct KeyNode {
	KeyNode()
	{
		mNext = 0;
		mPrev = 0;
		mList = 0;
	}

	KeyNode* mNext;          // _00
	KeyNode* mPrev;          // _04
	TObjKey00* mObj;         // _08
	struct KeyList* mList;   // _0C
};

/** Every live key initTObjKey00 makes it when the stage's objects load */
struct KeyList {
	KeyList();
	~KeyList();
	void add(KeyNode* node);
	void remove(KeyNode* node);
	void clear();

	KeyNode* mTail; // _00
	KeyNode* mHead; // _04
	int mCount;     // _08
};

struct KeyGlow {
	KeyGlow()
	{
		mKind = 0;
		_20   = 0;
	}

	u32 mKind; // _00
	u8 _04[8];
	u32 mColor; // _0C
	u8 _10[0x10];
	u32 _20;
};

class TObjKey00 : public TSetActor {
public:
	TObjKey00(TObject* parent, SetDesc* desc);
	virtual ~TObjKey00();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);

	void build();
	void showEffect();
	void playTake();
	void fitBounds();
	int touched();
	int siblingOpen();
	int lastToOpen();
	void useTouch();
	void useToggle();
	void useHold();
	void useTimed();
	void spawnTimer();

	KeyNode mNode;     // _64
	NJS_OBJECT* mPiece0; // _74
	NJS_OBJECT* mPiece1; // _78 the model's first child
	NJS_OBJECT* mPiece2; // _7C
	NJS_OBJECT* mPiece3; // _80
	MeshObject* mActor;      // _84
	void* mMaterial;   // _88
	DrawTask* mDraw;   // _8C
	KeyGlow mLight0;   // _90
	KeyGlow mLight1;   // _B4
	f32 mMtx[12];      // _D8
	Fx* mEffect;     // _108
	int mKind;         // _10C set int 0 kept in 0..5
	int mModel;        // _110 -1 when this kind has no model
	int mModel2;       // _114
	int mUse;          // _118 set int 1 kept in 0..3
	int mSwitch;       // _11C set int 3 kept in 0..0x7FF
	int mWho;          // _120 0 either 1 the player 2 the egg
	int mHold;         // _124 frames use 3 stays taken
	int mHoldFrame;    // _128
	f32 mScale;        // _12C
	f32 mRadius;       // _130
	int mSpin;         // _134
	int mLatched;      // _138
	int mOpen;         // _13C
};

extern "C" {
extern u8* sKeyFlags;
extern int sKeyReady;
extern const GXColor sKey00Color0;
extern const GXColor sKey00Color1;
extern const GXColor sKey00Color2;
extern const GXColor sKey00Color3;
extern const GXColor sKey00Color4;
extern KeyList* sKeyListObj;
}

#endif
