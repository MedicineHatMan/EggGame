#ifndef _FX_H
#define _FX_H

#include "core/types.h"

struct FxKey {
	s16 mType; // _00 0 = linear 1 = hermite
	u16 _02;
	u16 mFrame;   // _04 counts down
	u16 mFrames;  // _06 0 when finished
	f32 mFrom;    // _08
	f32 mTo;      // _0C
	f32 mTanFrom; // _10
	f32 mTanTo;   // _14
};

/** A wave. @size 0x10 */
struct FxWave {
	s16 mType;   // _00 1 = sine 2 = square
	u16 mFrames; // _02
	s16 mStep;   // _04
	s16 mAngle;  // _06
	f32 mAmp;    // _08
	f32 mAmpTo;  // _0C
};

/** How one value is worked out. @size 8 */
struct FxChannel {
	f32 mBase; // _00
	u8 mKey;   // _04
	u8 mWave;  // _05
	s8 mMode;  // _06 Bit 0 = key bit 1 = wave bit 2 = adds to the base
	u8 _07;
};

/** One entry in a bank's description table for the script effects */
struct FxDesc {
	u16 mShape;    // _00
	u16 mLife;     // _02
	s16 mTexGroup; // _04
	s16 mMaterial; // _06
	u32 mFlags;    // _08
	u8* mScript;   // _0C
	void* mParams; // _10
};

struct FxMaterialEntry {
	u32 mType;   // _00 0 = simple 1 = full
	void* mData; // _04
};

struct FxTexGroup {
	u16 mCount; // _00
	s16 _02;
	u16* mTextures; // _04
};

struct FxBank {
	u32 _00;
	int mFirst;  // _04
	int mCount;  // _08
	u32* mDescs; // _0C pairs of kind and data
	u32 _10;
	FxMaterialEntry* mMaterials; // _14
	u32 _18;
	u8* mModels;            // _1C pairs of something and a model
	int mTexGroupCount;     // _20
	FxTexGroup* mTexGroups; // _24
	u32 _28;
	void** mTextures; // _2C
};

struct FxGroup;

/** The start of every effect */
struct Fx {
	u16 mIndex; // _00
	u8 mBank;   // _02
	u8 _03;
	u16 mStop; // _04
	u16 _06;
	u32 mFlags;                    // _08 2 = stops by itself 0x4000000 = paused 0x40000000 = remove 0x80000000 = hidden
	void* mOwner;                  // _0C
	Fx* mNext;                     // _10
	Fx* mPrev;                     // _14 the head's points at the tail
	FxGroup* mGroup;               // _18
	f32* mFollow;                  // _1C
	void (*mRemove)(Fx*);          // _20
	void (*mUpdate)(Fx*);          // _24
	void (*mDraw)(Fx*);            // _28
	f32 mMatrix[12];               // _2C
	void (*mEvent)(Fx*, int, f32); // _5C called by script opcode 0xF4
};

struct FxGroup {
	Fx* mHead;  // _00
	int mCount; // _04
	u32 _08[2];
};

typedef Fx* (*FxFactory)(FxBank*, int);

/** An effect run by a script. @size 0x17C */
struct FxShape : public Fx {
	u32 _60;
	u8 mLoops;        // _64
	u8 mTex[3];       // _65
	u16 mWait;        // _68
	u16 mPc;          // _6A
	u16 mLoopPc;      // _6C
	u16 mMarkPc;      // _6E
	FxDesc* mDesc;    // _70
	u16 mColorFrames; // _74
	u16 mAlphaFrames; // _76
	u16 mUvFrames[2]; // _78
	u16 mColorLeft;   // _7C
	u16 mAlphaLeft;   // _7E
	u16 mRotFrames;   // _80
	u8 mColor[4];     // _82 Red green blue alpha
	u8 mColorTo[4];   // _86
	u8 _8A[2];
	f32 mU;           // _8C
	f32 mV;           // _90
	f32 mRot;         // _94
	f32 mRotTo;       // _98
	f32 mUv[2][2];    // _9C
	f32 mUvTo[2][2];  // _AC
	f32 mValues[8];   // _BC
	FxKey mKeys[4];   // _DC
	FxWave mWaves[4]; // _13C
};

/** More data on a particle, in a circular list through mPrev. @size 0x2C */
struct FxExtra {
	FxExtra* mNext; // _00
	FxExtra* mPrev; // _04
	f32 mA[3];      // _08
	f32 mB[3];      // _14
	f32 mC[3];      // _20
};

/** One particle of an emitter */
struct FxParticle {
	FxParticle* mNext; // _00
	u8 mLoops;         // _04
	u8 _05;
	u16 mFlags; // _06 1 = gravity 2 = damping 0x100 = finished
	u8 _08;
	u8 _09;
	u16 mLife;        // _0A
	u16 mWait;        // _0C
	u16 mPc;          // _0E
	u16 mLoopPc;      // _10
	u16 mMarkPc;      // _12
	u16 mColorFrames; // _14
	u16 mAlphaFrames; // _16
	u16 mRotFrames;   // _18
	u16 mScaleFrames; // _1A
	u16 mExtraCount;  // _1C
	u8 mColor[4];     // _1E Red green blue alpha
	u8 _22[2];
	u8* mScript;  // _24
	f32 mVel[3];  // _28
	f32 mGravity; // _34
	f32 mDamp;    // _38
	f32 mPos[3];  // _3C
	f32 mRot;     // _48
	f32 mScale;   // _4C
	f32 mScaleTo; // _50
	f32 mRotTo;   // _54
	f32 _58;
	u16 mColorLeft;             // _5C
	u16 mAlphaLeft;             // _5E
	u8 mColorTo[4];             // _60
	FxExtra* mExtra;            // _64
	void (*mStep)(FxParticle*); // _68
	struct FxEmitter* mOwner;   // _6C
};

/** An effect with a list of particles. @size 0xC8 */
struct FxEmitter : public Fx {
	u32 _60;
	u16 mKind; // _64
	u16 _66;
	f32 mDir[3];    // _68
	f32 mRate;      // _74
	f32 mAccum;     // _78
	u16 mMax;       // _7C
	u16 mPartLife;  // _7E
	u16 mPartFlags; // _80
	u8 mMaterial;   // _82
	u8 mTexGroup;   // _83
	u8 mTex[3];     // _84
	u8 _87;
	f32 mGravity;      // _88
	f32 mDamp;         // _8C
	f32 mScale;        // _90
	u8* mPartScript;   // _94
	u32 mCount;        // _98
	FxParticle* mKids; // _9C
	f32 _A0;
	f32 _A4;
	f32 mShapeA; // _A8 A count in the low byte for some kinds
	f32 mShapeB; // _AC
	f32 mShapeC; // _B0
	f32 mShapeD; // _B4
	u16 mWait;   // _B8
	u16 mPc;     // _BA
	u16 mLoopPc; // _BC
	u16 mMarkPc; // _BE
	u8 mLoops;   // _C0
	u8 _C1[3];
	u8* mScript; // _C4
};

/** One joint of an FxRig. @size 0x88 */
struct FxBone {
	u32 mFlags;       // _00 1 = move 2 = rotate 4 = scale 8 = the world matrix is old
	f32 mPos[3];      // _04
	int mRot[3];      // _10
	f32 mScale[3];    // _1C
	f32 mWorld[3][4]; // _28
	f32 mLocal[3][4]; // _58
};

/** A group of joints that other effects hang from */
struct FxRig : public Fx {
	u32 _60;
	u8 mLoops; // _64
	u8 _65;
	u16 mWait; // _66
	u16 mPc;   // _68
	u16 _6A;
	u8* mScript;    // _6C
	u16 mLoopPc;    // _70
	u16 mMarkPc;    // _72
	u16 mBoneCount; // _74
	u16 _76;
	FxBone* mBones; // _78
};

#endif
