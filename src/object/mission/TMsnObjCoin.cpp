#include "stage/SetManager.h"
struct DrawTask;
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/SetActor.h"
#include "object/ObjArchive.h"
#include "enemy/EnemyLight.h"
#include "object/ObjModel.h"
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "object/ObjShadow.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "engine/TParticle.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCamPath.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "sound/SoundCtrl.h"
#include "core/VecMath.h"
#include "object/mission/TMsnObjCoin.h"
#include "core/TMainTask.h"
struct TObject;
struct Vec3f;
struct CharaColli;
struct NJS_OBJECT;
struct DrawStateSlot;
struct SetLink;
struct DrawState;

static void coinTevStage(u8 stage);
static void coinTexGen(u8 coord);

enum {
	COIN_WAIT  = 0, // spinning waiting for a touch
	COIN_FLASH = 1, // squashing about to set the key bit
	COIN_FLY   = 2, // flying to whoever touched it
};

const f32 gCoinSetRange = 700.0f;

// 0x80197BE8
extern "C" TObject* createTMsnObjCoin(TObject* parent, SetDesc* desc)
{
	return new TMsnObjCoin(parent, desc);
}

const char* gTMsnObjCoinName = "TMsnObjCoin";

// 0x80197C3C
TMsnObjCoin::TMsnObjCoin(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00                    = (u32)gTMsnObjCoinName;
	mSize                  = 0x16C;
	NJS_OBJECT** models    = getStgObjModels();
	mModel                 = models[52];
	NJS_TEXLIST** textures = getStgObjTextures();
	mTexList               = textures[45];
	GjModel* part          = mModel->model;
	mRadius                = part->r;
	mDrawTask              = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	mShape                 = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 255, 254, 254);
	mScale.x    = 1.0f;
	mScale.y    = 1.0f;
	mScale.z    = 1.0f;
	mSpin       = 128;
	mPulse      = 0;
	mState      = 0;
	f32 scaled  = mRadius * 2.0f;
	mShadow._30 = scaled;
	mShadow._34 = scaled;
	initFromDesc(desc);
	TParticleGenerator* made = particleGenById((TObject*)this, &mPos, 0, 0x19C);
	mGen                     = (TObject*)made;
	made                     = (TParticleGenerator*)mGen;
	if (made != 0) {
		made->mAttach = mMatrix;
	}
}

// 0x80197DA4
TMsnObjCoin::~TMsnObjCoin()
{
	if (mGen != 0) {
		mGen->mFlags |= 1;
		mGen = 0;
	}
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x80197E60
// Int 0 is the key bit counted from 0x52C
void TMsnObjCoin::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mKey = desc->getInt(0) + 0x52C;
	updateMatrix();
	ColliShape* shape = colliGetShape(mShape, 0);
	colliShapeMakeSphere(shape, &mPos, 0, 2.5f * mRadius);
	shape->mFlags |= 0x10;
	EggHits hits;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mPos, -1) != 0 && (hits.mMask & 1) != 0) {
		shadowSetFromContact(&mShadow, (u8*)&hits);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
}

// 0x80197F4C
// The lines are the set-file labels
void TMsnObjCoin::debugPrint(void*)
{
	njPrintC(0xA0013, ":FLAG ID");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
}

// 0x80197FAC
void TMsnObjCoin::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxScaleV(mMatrix, mMatrix, &mScale);
}

// 0x80198000
// A coin whose key bit is already set and that was never touched goes away at once
void TMsnObjCoin::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (keyBitIsSet(mKey) != 0 && mState == COIN_WAIT) {
		mFlags |= 0x10;
		setLinkForever((SetLink*)&mLink);
		mFlags |= 1;
		return;
	}
	if (isInRange(this, &mPos, 750.0f) == 0) {
		mFlags |= 1;
		return;
	}
	switch (mState) {
	case COIN_WAIT:
		waitTouch();
		break;
	case COIN_FLASH:
		flash();
		break;
	case COIN_FLY:
		fly();
		break;
	}
}

// 0x801980E4
// Spins until something touches it then remembers who
void TMsnObjCoin::waitTouch()
{
	for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactNoBit10(contact) != 0) {
			mRef.setId(colliContactOwnerU16(contact));
			particleOnceById(gTL_05, &mPos, 0, 0x19D);
			sePlay3D(0x5F, &mPos);
			mState = COIN_FLASH;
		}
	}
	mRot.y += mSpin;
	updateMatrix();
}

// 0x80198188
// Squashes in and out once then sets the key bit
void TMsnObjCoin::flash()
{
	f32 s    = njSin(mPulse);
	mScale.x = 1.0f + s;
	mScale.y = 1.0f - s;
	mScale.z = 1.0f;
	mPulse += 0x800;
	if (mPulse == 0) {
		mScale.x = 1.0f;
		mScale.y = 1.0f;
		mScale.z = 1.0f;
		sePlay(0x2A);
		setKeyBit(mKey);
		mState = COIN_FLY;
	}
	mRot.y += mSpin * 4;
	updateMatrix();
}

// 0x80198228
// Flies to whoever touched it shrinking and goes away on arrival
void TMsnObjCoin::fly()
{
	if (mRef.get() == 0) {
		setLinkForever((SetLink*)&mLink);
		mFlags |= 1;
		return;
	}
	Vec3f to = *mRef.get()->getPos();
	to.y += 5.0f;
	vecSub(&to, &to, &mPos);
	if (vecLength(&to) < 5.0f) {
		setLinkForever((SetLink*)&mLink);
		mFlags |= 1;
	}
	vecNormalize(&to, &to);
	vecScaleAdd(5.0f, &mPos, &to, &mPos);
	Vec3f shrink;
	shrink.x = 0.08f;
	shrink.y = 0.08f;
	shrink.z = 0.08f;
	vecSub(&mScale, &mScale, &shrink);
	if (mScale.x < 0.1f) {
		setLinkForever((SetLink*)&mLink);
		mFlags |= 1;
	}
	mRot.y += mSpin * 16;
	updateMatrix();
}

// 0x80198370
void TMsnObjCoin::vfn_10()
{
	if (mLight.isVisible(mDrawTask, 600.0f, &mPos, mRadius, 0) != 0) {
		mDrawTask->mFlags &= ~1;
		setDrawAmbient(&mLight, &mState2);
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x801983EC
// Draws the model with a second environment-mapped texture stage
void TMsnObjCoin::render()
{
	f32 mtx[12];
	mtxConcat(mtx, (f32*)gjGetMatrix(), mMatrix);
	DrawStateSlot* slot = getDrawStateSlot();
	beginDrawState(slot, (const DrawState*)&mState2);
	njPushMatrix(mtx);
	njSetTexture(mTexList);
	gjLoadTextureNum(1, 0, 0, 1);
	f32 env[12];
	PSMTXCopy(_gj_matrix_, env);
	env[3]  = 0.0f;
	env[7]  = 0.0f;
	env[11] = 0.0f;
	GXLoadTexMtxImm(env, GX_TEXMTX5, GX_MTX3x4);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)coinTevStage);
	gjSetPostTexGenCallback((void*)coinTexGen);
	_nj_control_3d_flag_ |= 0x80000;
	shadowAlphaBegin();
	gjDrawObject(mModel);
	shadowAlphaEnd();
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= ~0x80000;
	njPopMatrixEx();
	endDrawState(slot);
}

// 0x801984F4
// The added stage colour += texture / 2
static void coinTevStage(u8 stage)
{
	u8 st = (u8)stage;
	GXSetTevColorIn(st, GX_CC_CPREV, GX_CC_TEXC, GX_CC_HALF, GX_CC_ZERO);
	GXSetTevColorOp(st, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(st, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(st, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(st, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x80198598
// Its texture coordinates come from the normals through matrix 0x2D
static void coinTexGen(u8 coord)
{
	GXSetTexCoordGen2(coord, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19);
}
