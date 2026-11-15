#include "egg/ObjEgg.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "enemy/EnemyLight.h"
#include "object/ObjCommon.h"
#include "engine/CharaColli.h"
#include "engine/DrawState.h"
#include "engine/DrawTask.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "object/ObjAction.h"
#include "stage/TStageScore.h"
#include "object/ObjShadow.h"
#include "camera/TCamera.h"
#include "object/mission/TMsnObjRing.h"
#include "engine/Color.h"
#include "object/TObjLauncher.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCamPath.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "stage/TGameState.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
#include "engine/TParticle.h"
#include "object/TObjKey00.h"
#include "object/ObjScript.h"
struct DrawState;
struct ObjShadow;
struct NJS_OBJECT;
struct ColliShape;
struct SetLink;
struct NJS_QUATERNION;

extern "C" {
static const char* sClassName = "TMsnObjRing";

__declspec(section ".sdata") int sRingBurstEffects[] = {
	249,
	250,
	0,
}; // the burst effect of each kind
}

enum {
	RING_WAIT = 0, // waits for a hit
	RING_GONE = 1, // floats away and fades
	RING_HOP  = 2, // hit falls back home
};

typedef char TMsnObjRing_size_check[(sizeof(TMsnObjRing) == 0x188) ? 1 : -1];

// 0x8010718C
extern "C" void initRingArc()
{
	gRingArc    = 0;
	gRingAsset0 = 0;
	gRingAsset1 = 0;
	gRingAsset2 = 0;
	gRingAsset3 = 0;
}

// 0x801071A8
extern "C" void releaseRingArc()
{
	if (gRingArc != 0) {
		releaseTextures((NJS_TEXLIST*)gRingAsset3);
		releaseTextures((NJS_TEXLIST*)gRingAsset2);
		releaseTextures((NJS_TEXLIST*)gRingAsset1);
		releaseTextures((NJS_TEXLIST*)gRingAsset0);
		releaseResource(gRingArc);
		gRingArc    = 0;
		gRingAsset0 = 0;
		gRingAsset1 = 0;
		gRingAsset2 = 0;
		gRingAsset3 = 0;
	}
}

static void loadRingArc();

// 0x80107210
// The event script's load_ring
extern "C" int scriptLoadRing(void*)
{
	loadRingArc();
	return 1;
}

// 0x80107234
static void loadRingArc()
{
	loadArc("geobj_ring.arc", &gRingArc);
	gRingShineModel = (NJS_OBJECT*)findArcExport(gRingArc, "model");
	gRingAsset0     = findArcExport(gRingArc, "texlist_mini");
	gRingAsset1     = findArcExport(gRingArc, "texlist_big");
	gRingAsset2     = findArcExport(gRingArc, "texlist_eff_mini");
	gRingAsset3     = findArcExport(gRingArc, "texlist_eff_big");
	bindGvmTextures((u8*)findArcExport(gRingArc, "texture"), (NJS_TEXLIST*)gRingAsset0);
	bindGvmTextures((u8*)findArcExport(gRingArc, "texture"), (NJS_TEXLIST*)gRingAsset1);
	bindGvmTextures((u8*)findArcExport(gRingArc, "texture"), (NJS_TEXLIST*)gRingAsset2);
	bindGvmTextures((u8*)findArcExport(gRingArc, "texture"), (NJS_TEXLIST*)gRingAsset3);
}

// 0x8010730C
extern "C" TObject* createTMsnObjRing(TObject* parent, SetDesc* desc)
{
	return new TMsnObjRing(parent, desc);
}

static inline int clamp(int value, int lo, int hi)
{
	if (value < lo) {
		return lo;
	}
	if (value > hi) {
		return hi;
	}
	return value;
}

// 0x80107360
// Int 0 is the kind 0 or 1
void TMsnObjRing::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mKind    = desc->getInt(0);
	int kind = mKind;
	if (kind < 0) {
		kind = 0;
	} else if (kind > 1) {
		kind = 1;
	}
	mKind   = kind;
	mSwitch = desc->getInt(3);
	int id  = mSwitch;
	if (id < 0) {
		id = 0;
	} else if (id > 0x800) {
		id = 0x800;
	}
	mSwitch = id;

	switch (mKind) {
	case 0:
		mShineTex = gRingAsset2;
		mTexList  = (NJS_TEXLIST*)gRingAsset0;
		mScale    = 1.0f;
		mLife     = 10;
		break;
	case 1:
		mShineTex = gRingAsset3;
		mTexList  = (NJS_TEXLIST*)gRingAsset1;
		mScale    = 1.6f;
		mLife     = 100;
		break;
	}

	mPlace.mFlags = 7;
	mPlace.mPos.x = mPos.x;
	mPlace.mPos.y = mPos.y;
	mPlace.mPos.z = mPos.z;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	f32 scale       = mScale;
	mPlace.mScale.x = scale;
	mPlace.mScale.y = scale;
	mPlace.mScale.z = scale;
	mPlace.update();

	f32 radius  = sRingShapes[mKind].mRadius;
	mShadow._30 = 2.0f * radius;
	mShadow._34 = 0.5 * radius;

	// Three ground probes A hit plants the shadow
	EggHits hits;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mPos, 1) != 0) {
		shadowSetFromContact(&mShadow, (u8*)&hits);
		shadowEnable(&mShadow);
		mOnGround = 1;
	} else {
		shadowDisable(&mShadow);
		mOnGround = 0;
	}
}

// 0x8010754C
// The lines are the set-file labels
void TMsnObjRing::debugPrint(void*)
{
	njPrintC(0xA0013, ":TYPE");
	njPrintC(0xA0016, ":SWID(0~255)");
}

// 0x8010758C
TMsnObjRing::TMsnObjRing(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x188;
	initFromDesc(desc);
	setLinkDone((SetLink*)&mLink);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	setup();
}

// 0x80107644
TMsnObjRing::~TMsnObjRing()
{
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x801076D4
void TMsnObjRing::setup()
{
	mMode           = RING_WAIT;
	mSpin           = 0x80;
	mSpeed          = 0.0f;
	mState.mAmbient = 0xFF;
	mShape          = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFE, 0xF6);
	colliCopyShapes(mShape, (const ColliShape*)&sRingShapes[mKind]);
	colliPlaceShapes(mShape, &mPos, (const ColliShape*)&sRingShapes[mKind]);
}

// What the last hit tells the mission
struct RingMsg {
	u16 mType; // _00 0x91
	u16 _02;
	u8 mScore; // _04
};

// 0x8010778C
void TMsnObjRing::checkHits()
{
	ColliContact* contact;

	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactHasBit8(contact) && colliContactNoBit10(contact)) {
			mLife -= 10;
			if (mLife == 0) {
				RingMsg msg;
				msg.mScore = 1;
				msg.mType  = 0x91;
				if (mKind == 0) {
					sendGameMsg((ObjMsg*)&msg);
					scoreAddExtra(200);
				} else {
					msg.mScore = 5;
					sendGameMsg((ObjMsg*)&msg);
					scoreAddExtra(1000);
				}
				particleOnceByIdDefault(getPos(), 0, sRingBurstEffects[mKind]);
				mSpeed = 2.5f;
				moveDrawTask(mDrawTask, 0x22);
				sePlay3D(0x61, getPos());
				mMode = RING_GONE;
			} else {
				colliDisable(mShape);
				particleOnceByIdDefault(getPos(), 0, sRingBurstEffects[mKind]);
				mSpeed = 2.5f;
				sePlay3D(0x60, getPos());
				mMode = RING_HOP;
			}
			if (colliContactInGroup8(contact)) {
				RingMsg hit;
				TObjAction* other = findObjById(colliContactOwner(contact));
				hit.mType         = 0x2A;
				other->sendToHandle((ObjMsg*)&hit);
			}
		}
	}
}

// 0x80107958
void TMsnObjRing::floatAway()
{
	mSpeed += -0.1f;
	f32 y = mPos.y + mSpeed;
	if ((mState.mAmbient & 0xFF) == 0) {
		mFlags |= 1;
	} else {
		mPos.y = y;
	}
	int spin = mSpin + 0x200;
	mSpin    = spin < 0x80 ? 0x80 : (spin > 0x1000 ? 0x1000 : spin);
	if (mSpeed < 0.0f) {
		addColorAlpha(&mState.mAmbient, -8);
	}
}

// 0x801079F8
void TMsnObjRing::hop()
{
	f32 y;
	f32 spin;

	checkHits();
	mSpeed += -0.1f;
	if (mSpeed < 0.0f) {
		colliEnable(mShape);
	}
	y = mPos.y + mSpeed;
	if (mHome.y > y) {
		mPos.y = mHome.y;
		mSpin  = 0x80;
		mMode  = RING_WAIT;
	} else {
		mPos.y = y;
	}
	spin  = mSpin + 0x200 < 0x80 ? 128.0f : ((f32)(mSpin + 0x200) > 2048.0f ? 2048.0f : mSpin + 0x200);
	mSpin = spin;
	colliPlaceShapes(mShape, &mPos, (const ColliShape*)&sRingShapes[mKind]);
}

// 0x80107B04
void TMsnObjRing::draw()
{
	if (objDebugEnabled(this) == 0 && keyBitIsSet(mSwitch) != 0) {
		switch (mMode) {
		case RING_WAIT:
			checkHits();
			break;
		case RING_GONE:
			floatAway();
			break;
		case RING_HOP:
			hop();
			break;
		}
		mRot.y        = mSpin + mRot.y;
		mPlace.mPos.x = mPos.x;
		mPlace.mPos.y = mPos.y;
		mPlace.mPos.z = mPos.z;
		njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
		mPlace.update();
		shadowSetAngle(&mShadow, mRot.y);
	}
}

// 0x80107BDC
void TMsnObjRing::vfn_10()
{
	if (keyBitIsSet(mSwitch) == 0 && objDebugEnabled(this) == 0) {
		mDrawTask->mFlags |= 1;
		shadowDisable(&mShadow);
		return;
	}
	if (mLight.isVisible(mDrawTask, 1000.0f, &mPos, 5.0f + sRingShapes[mKind].mRadius, 0) != 0) {
		mDrawTask->mFlags &= ~1;
		if (mMode != RING_GONE) {
			setDrawAmbient(&mLight, (DrawState*)&mState.mFlags);
		} else {
			mState.mFlags = DRAWSTATE_AMBIENT;
		}
	} else {
		mDrawTask->mFlags |= 1;
	}
	if (mOnGround != 0) {
		Vec3f ground;
		ground = mShadow.mGround;
		if (cameraIsVisible(1000.0f, 5.0f + sRingShapes[mKind].mRadius, &ground, 0) != 0) {
			if (mMode == RING_GONE && mSpeed < 0.0f) {
				shadowDisable(&mShadow);
			} else {
				shadowEnable(&mShadow);
			}
		} else {
			shadowDisable(&mShadow);
		}
	}
}

// 0x80107D58
// Without the shine model it draws a flat black ring
void TMsnObjRing::render()
{
	f32 view[12];
	f32 mtx[12];
	f32 tex[12];
	DrawStateSlot* slot;

	mtxConcat(view, (f32*)gjGetMatrix(), mPlace.mMatrix);
	if (gRingShineModel == 0) {
		mtxRotateX(mtx, view, 0x4000);
		mtxScale(12.5f, 2.0f, 12.5f, mtx, mtx);
		debugWireCylinder(mtx, &gColorBlack);
	} else {
		slot = getDrawStateSlot();
		beginDrawState(slot, &mState);
		njPushMatrix(view);
		njSetTexture(mShineTex);
		gjLoadTextureNum(0, 0, 0, 1);
		PSMTXCopy(_gj_matrix_, tex);
		tex[3]  = 0.0f;
		tex[7]  = 0.0f;
		tex[11] = 0.0f;
		GXLoadTexMtxImm(tex, GX_TEXMTX5, GX_MTX3x4);
		gjSetNumPostTexGens(1);
		gjSetNumPostTevStages(1);
		gjSetPostTevStageCallback((void*)ringShineTev);
		gjSetPostTexGenCallback((void*)ringShineTexGen);
		_nj_control_3d_flag_ |= 0x80000;
		shadowAlphaBegin();
		njSetTexture(mTexList);
		gjDrawObject(gRingShineModel);
		shadowAlphaEnd();
		gjSetNumPostTexGens(0);
		gjSetNumPostTevStages(0);
		_nj_control_3d_flag_ &= ~0x80000;
		njPopMatrixEx();
		endDrawState(slot);
	}
}

// 0x80107EAC
// Adds the shine texture to the colour
extern "C" void ringShineTev(int id)
{
	u8 stage = id;

	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x80107F50
// Texture coordinates from the normals
extern "C" void ringShineTexGen(int gen)
{
	((void (*)(u8, int, int, int, int, int))GXSetTexCoordGen2)(gen, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19);
}

// 0x80107F88
int TMsnObjRing::getKind()
{
	return 3;
}
