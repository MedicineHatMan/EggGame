#ifndef _TOBJFALLTHING_H
#define _TOBJFALLTHING_H

#include "object/ObjAction.h"
#include "Ninja/ninja.h"
#include "object/ObjModel.h"
#include "object/ObjCommon.h"
#include "engine/DrawTask.h"
#include "stage/SetManager.h"
#include "engine/CharaColli.h"
#include "engine/TParticle.h"

enum FallThingType {
	FALL_BOUNCE  = 0, // bounces off the ground spinning until it settles
	FALL_SHATTER = 1, // breaks into mPieceModels when it lands
};

enum FallThingState {
	FALL_FALLING = 0,
	FALL_LANDED  = 1, // counting mLife down
};

/** What a caller hands spawnFallThing */
struct FallSpawn {
	FallSpawn() { }

	// The defaults no model particle bank 2 none a light pull down and no landing sound
	void reset()
	{
		mType         = 0;
		mModel        = 0;
		mTexList      = 0;
		mPos.x        = 0.0f;
		mPos.y        = 0.0f;
		mPos.z        = 0.0f;
		mRot.x        = 0;
		mRot.y        = 0;
		mRot.z        = 0;
		mRadius       = 0.0f;
		mDamage       = 0;
		mParticleBank = 2;
		mParticleId   = 0x200;
		mPieceModels  = 0;
		mPieceFirst   = 0;
		mPieceCount   = 0;
		mPieceTexList = 0;
		mGravity      = -0.08f;
		mLandSe       = -1;
	}

	int mType;                 // _00 a FallThingType
	NJS_OBJECT* mModel;        // _04
	NJS_TEXLIST* mTexList;     // _08
	Vec3f mPos;                // _0C
	SetRot mRot;               // _18
	f32 mRadius;               // _24 kept only when negative
	s16 mDamage;               // _28
	s16 _2A;                   // _2A
	int mParticleBank;         // _2C 2 means no particles
	int mParticleId;           // _30
	NJS_OBJECT** mPieceModels; // _34 what FALL_SHATTER breaks into
	u16 mPieceFirst;           // _38
	u16 mPieceCount;           // _3A
	void* mPieceTexList;       // _3C
	f32 mGravity;              // _40 added to the y speed each frame
	int mLandSe;               // _44 -1 for none
};

/**
 * @size 0x118
 */
class TObjFallThing : public TObjAction {
public:
	TObjFallThing(TObject* parent, const FallSpawn* spawn);
	virtual ~TObjFallThing();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                              // _0C
	virtual void vfn_10();                                            // _10
	virtual void render();                                            // _1C
	virtual int getKind();                                            // _2C
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58

	void fall();
	void land();
	void initColli();
	void placeColli();
	void termColli();
	void updateMatrix();

	ObjModel mObj;                  // _30
	DrawTask* mDraw;                // _50
	TParticleGenerator* mParticles; // _54
	ObjShadow mShadow;              // _58
	CharaColli* mColli;             // _90
	u16 mType;                      // _94 a FallThingType
	u8 _96[2];
	f32 mMtx[12]; // _98
	Vec3f mPos;   // _C8
	SetRot mRot;  // _D4
	Vec3f mVel;   // _E0
	SetRot mSpin; // _EC added to mRot each frame
	f32 mRadius;  // _F8
	s16 mDamage;  // _FC
	u8 _FE[2];
	NJS_OBJECT** mPieceModels; // _100
	u16 mPieceFirst;           // _104
	u16 mPieceCount;           // _106
	void* mPieceTexList;       // _108
	f32 mGravity;              // _10C
	int mLandSe;               // _110
	s16 mState;                // _114 a FallThingState
	s16 mLife;                 // _116 frames left once landed
};

extern "C" TObjFallThing* spawnFallThing(TObject* parent, const FallSpawn* spawn); // 0x800A39E8

extern "C" {
extern u32 sFallThingName[2];
}

#endif
