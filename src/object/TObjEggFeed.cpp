#include "stage/SetManager.h"
#include "object/TObjEggFeed.h"
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "object/ObjArchive.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "animal/AniMount.h"
#include "camera/TCameraCtrl.h"
#include "engine/Viewport.h"
#include "enemy/boss/EneBoss.h"
#include "stage/TGame.h"
#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "engine/Color.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "engine/Colli.h"
#include "sound/Sound3DPlay.h"
#include "Ninja/ninja.h"
#include "engine/FxSystem.h"
#include "object/TObjLauncher.h"
#include "engine/GroundQuery.h"
#include "camera/TCamPath.h"
#include "ui/AeArc.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "engine/WorldMesh.h"
#include "stage/TEventCtrl.h"
#include "core/TMainTask.h"
#include "enemy/boss/TEnePurpleBoss.h"

const char* gTObjEggFeedName = "TObjEggFeed";
const f32 gEggFeedSetRange   = 600.0f;
int gEggFeedFlag;
char* sEggFeedKindNames[7]  = { "APPLE", "BANANA", "CHERRY", "MELON", "PINEAPPLE", "STRAWBERRY", "WATERMELON" };
char* sEggFeedSizeNames[3]  = { "NRM SIZE", "BIG SIZE", "EXTREME SIZE" };
char* sEggFeedOnOffNames[2] = { "OFF", "ON" };
struct ColliContact;
struct CharaColli;
struct SetLink;
struct CamView;
struct NJS_TEXLIST;

typedef char TObjEggFeed_size_check[(sizeof(TObjEggFeed) == 0x220) ? 1 : -1];

struct FeedContact {
	FeedContact* mNext; // _00
	void* mColli;       // _04
	int mIndex;         // _08
};

struct FeedMsg { // padded to 8
	u16 mType;   // _00
	u16 _02;
	u8 mKind;   // _04
	u8 mAmount; // _05
	u16 _06;
};

struct FeedNote { // padded to 8
	u16 mType;    // _00
	u16 _02;
	u16 mId; // _04
	u16 _06;
};

// MSL's sqrtf inlined where the food measures a distance
static inline f32 feedSqrt(f32 x)
{
	static const f64 _half  = .5;
	static const f64 _three = 3.0;
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		y         = (f32)(x * (_half * guess * (_three - guess * guess * x)));
		return y;
	}
	return x;
}

static inline f32 feedRandSigned()
{
	return 2.0f * (3.0517578e-05f * (f32)rand() - 0.5f);
}

static inline f32 feedRand()
{
	return 0.2f * feedRandSigned();
}

// 0x800CBD8C
// The pair SetManager runs for the stage the flag starts clear
extern "C" void initTObjEggFeed()
{
	gEggFeedFlag = 0;
}

// 0x800CBD98
extern "C" void termTObjEggFeed()
{
}

// 0x800CBD9C
extern "C" TObject* createTObjEggFeed(TObject* parent, SetDesc* desc)
{
	return new TObjEggFeed(parent, desc);
}

// 0x800CBDF0
extern "C" TObjEggFeed* createTObjEggFeedAt(int kind, const Vec3f* pos, u8 sizeKind)
{
	return new TObjEggFeed(gTL_05, kind, pos, sizeKind);
}

// 0x800CBE50
void TObjEggFeed::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	place(desc->getInt(0), &mPos, &mRot, desc->getInt(2), desc->getInt(1));
	mUser     = desc->getInt(3);
	mUserF[0] = desc->getFloat(0);
	mUserF[1] = desc->getFloat(1);
	mUserF[2] = desc->getFloat(2);
	mUserF[3] = desc->getFloat(3);
}

// 0x800CBF78
void TObjEggFeed::debugPrint(void* arg)
{
	(void)arg;
	njPrint(0xA0013, ":TYPE %s", sEggFeedKindNames[mKind]);
	njPrint(0xA0014, ":REBIRTH %s", sEggFeedOnOffNames[mRebirth]);
	njPrint(0xA0015, ":%s", sEggFeedSizeNames[mSizeKind]);
	njPrint(0xA0017, "");
}

// 0x800CC030
TObjEggFeed::TObjEggFeed(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
    , mSpawned(0)
{
	_00   = (u32)gTObjEggFeedName;
	mSize = 0x220;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	makeShape();
	initFromDesc(desc);
}

// 0x800CC100
TObjEggFeed::TObjEggFeed(TObject* parent, int kind, const Vec3f* pos, int sizeKind)
    : TSetActor(parent)
    , mSpawned(1)
{
	_00   = (u32)gTObjEggFeedName;
	mSize = 0x220;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	makeShape();
	mRot.x = 0;
	mRot.y = 0;
	mRot.z = 0;
	place(kind, pos, &mRot, sizeKind, 0);
	mVel.x = feedRand();
	mVel.y = 1.0f;
	mVel.z = feedRand();
	colliDisable(mShape);
}

// 0x800CC27C
TObjEggFeed::~TObjEggFeed()
{
	freeShape();
	removeDrawTask(mTask);
}

// 0x800CC324
void TObjEggFeed::place(int kind, const Vec3f* pos, const SetRot* rot, int sizeKind, int rebirth)
{
	NJS_OBJECT** models;
	NJS_TEXLIST** tex;

	mKind     = kind < 0 ? 0 : (kind > 6 ? 6 : kind);
	mPos.x    = pos->x;
	mPos.y    = pos->y;
	mPos.z    = pos->z;
	mRot.x    = rot->x;
	mRot.y    = rot->y;
	mRot.z    = rot->z;
	mSizeKind = sizeKind < 0 ? 0 : (sizeKind > 2 ? 2 : sizeKind);
	mRebirth  = rebirth < 0 ? 0 : (rebirth > 1 ? 1 : rebirth);
	mEater.setId(0xFFFF);
	mSway.x    = 0.0f;
	mSway.y    = 0.0f;
	mSway.z    = 0.0f;
	mBob       = 0.0f;
	mVel.x     = 0.0f;
	mVel.y     = 0.0f;
	mVel.z     = 0.0f;
	mLife      = 0x4B0;
	mSpin      = 0;
	mPhase     = 0;
	mFeedFlags = 0;
	mFeedFlags |= 0xD;
	switch (mSizeKind) {
	case 0:
		mAmount = 1;
		mScale  = 1.25f;
		break;
	case 1:
		mAmount = 5;
		mScale  = 1.8f;
		break;
	case 2:
		mAmount = 100;
		mScale  = 4.0f;
		break;
	}
	mPos.y = 4.0f * mScale + mPos.y;
	setLinkFree((SetLink*)&mLink);
	mDraw.mFlags = 0;
	models       = getStgObjModels();
	mModel.setModel(*(NJS_OBJECT**)((u8*)models + mKind * 4 + 0x64));
	tex             = getStgObjTextures();
	mModel.mTexList = *(NJS_TEXLIST**)((u8*)tex + mKind * 4 + 0x60);
	mModel.mState   = &mDraw;
	models          = getStgObjModels();
	mShadow.setModel(models[24]);
	tex              = getStgObjTextures();
	mShadow.mTexList = tex[23];
	mShadow.mState   = &mDraw;
	buildMatrix();
	setSphere();
	mLight.reset();
	mLight.setFadeDist(5.0f);
	mShade = mLink.get()->getData() != 0 ? 0 : 0xFF;
}

// 0x800CC568
void TObjEggFeed::makeShape()
{
	mShape = (CharaColli*)colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFE, 0xF6);
}

// 0x800CC5C4
void TObjEggFeed::setSphere()
{
	colliShapeMakeSphere(colliGetShapes(mShape), &mPos, 0, 4.0f * mScale);
}

// 0x800CC60C
void TObjEggFeed::syncPos()
{
	colliShapeSetPos(colliGetShapes(mShape), &mPos);
}

// 0x800CC644
void TObjEggFeed::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x800CC67C
void TObjEggFeed::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	switch (mPhase) {
	case 0:
		idle();
		break;
	case 1:
		eaten();
		break;
	case 2:
		fly();
		break;
	}
}

// 0x800CC6F8
void TObjEggFeed::setPhase(int phase)
{
	mPhase = phase;
}

// 0x800CC700
void TObjEggFeed::idle()
{
	int hit;
	TObjAction* other;
	FeedContact* contact;

	if (isInRange(this, &mPos, 650.0f) == 0) {
		mLink.get()->setData(0);
		mFlags |= 1;
		return;
	}
	hit = 0;
	for (contact = (FeedContact*)mShape->mContacts; contact != 0; contact = contact->mNext) {
		if (colliInGroup((CharaColli*)contact->mColli, 8) != 0) {
			if ((colliGetShape((CharaColli*)contact->mColli, contact->mIndex)->mFlags & 0x400) != 0) {
				other = findObjById(colliContactOwner((ColliContact*)contact));
				if (other != 0) {
					if (inReach(other->getPos(), other->getRadiusV()) != 0) {
						mEater.setId(other->mObjId);
						colliDisable(mShape);
						mLife = 0xC;
						sePlay3D(0x52, &mPos);
						setPhase(1);
					}
					hit = 1;
				}
			}
		}
	}
	if (hit == 0) {
		vecScaleAdd(-0.05f, &mSway, &mSway, &mSway);
		mBob += -0.05f * mBob;
	}
	mRot.y += 0x100;
	if ((mFeedFlags & 1) != 0) {
		if (land() != 0) {
			mFeedFlags &= ~1;
		}
		syncPos();
	} else if (mGround.mMesh == 0) {
		mFeedFlags |= 1;
	}
	buildMatrix();
	mSpin += 0x100;
	if (mSpawned == 1 && hit == 0) {
		if (mLife <= 300) {
			if (mLife == 0) {
				setLinkDone((SetLink*)&mLink);
				mFlags |= 1;
			} else if ((mLife & 1) != 0) {
				mFeedFlags |= 0xC;
			} else {
				mFeedFlags &= ~0xC;
			}
		}
		mLife--;
	}
	{
		int shade = (u8)mShade + 4;
		mShade    = shade > 0xFF ? 0xFF : shade;
	}
}

// 0x800CC960
static void feedScreen(const Vec3f* pos, Vec3f* out)
{
	f32 d;
	f32 c;
	f32 b;
	f32 a;

	getViewport(&d, &c, &b, &a);
	b *= 0.5f;
	a *= 0.5f;
	out->set((pos->x + d) - b, a - (pos->y + c), -((f32*)&gProjection)[20]);
	gjScaleVector(pos->z / ((f32*)&gProjection)[20], out, out);
}

// 0x800CCA10
void TObjEggFeed::eaten()
{
	s16 life;
	f32 mtx[12];
	f32 len;
	TObjAction* eater;
	FeedMsg msg;
	FeedNote note;

	life  = mLife;
	mLife = (s16)(life - 1);
	if (life != 0) {
		return;
	}
	mtxSetTransV(mtx, &mPos);
	mtxScale(mScale, mScale, mScale, mtx, mtx);
	fxSpawnAt(1, 7, 0x6B, mtx);
	if (gEggFeedFlag != 0) {
		eater = mEater.get();
		if (eater != 0) {
			msg.mType   = 0x18;
			msg.mKind   = (u8)mKind;
			msg.mAmount = (u8)mAmount;
			eater->sendToHandle((ObjMsg*)&msg);
			note.mType = 0xB3;
			note.mId   = eater->mHolderId;
			aeSendMessage(&note);
		}
		if (mRebirth != 0 && mSpawned == 0) {
			setLinkTimed((SetLink*)&mLink, 0x708);
		} else {
			setLinkDone((SetLink*)&mLink);
		}
		mLink.get()->setData((void*)1);
		mFlags |= 1;
	} else {
		Mtx34 view;
		Vec3f projected;
		Vec3f screen;

		mFeedFlags &= ~8;
		mFeedFlags |= 2;
		mFeedFlags |= 0x10;
		moveDrawTask(mTask, 0x32);
		njPushMatrixEx();
		camGetDrawView()->apply();
		view = *(Mtx34*)gjGetMatrix();
		njPopMatrixEx();
		njCalcPoint(view.m, &mPos, &projected);
		njCalcScreen((const Vec3f*)&projected, (const f32*)&mPos, &mPos.y);
		mPos.z = -projected.z;
		mVel.x = -1.0f;
		mVel.y = -1.0f;
		len    = feedSqrt(mVel.x * mVel.x + mVel.y * mVel.y);
		mVel.x /= len;
		mVel.y /= len;
		mVel.x *= 8.0f;
		mVel.y *= 8.0f;
		mVel.z = -16.0f;
		feedScreen(&mPos, &screen);
		mtxSetTransV(mMtx, &screen);
		setPhase(2);
	}
}

// 0x800CCC9C
void TObjEggFeed::fly()
{
	f32 len;
	f32 dx;
	f32 dy;
	Vec3f screen;
	TObjAction* eater;
	FeedMsg msg;
	FeedNote note;

	dx  = sEggFeedTarget.x - mPos.x;
	dy  = sEggFeedTarget.y - mPos.y;
	len = feedSqrt(dx * dx + dy * dy);
	dx /= len;
	dy /= len;
	dx *= 2.0f;
	dy *= 2.0f;
	mVel.x = mVel.x + dx;
	mVel.y = mVel.y + dy;
	vecAdd(&mPos, &mPos, &mVel);
	mVel.x *= 0.95f;
	mVel.y *= 0.95f;
	if (mPos.z < sEggFeedTarget.z) {
		mPos.z = sEggFeedTarget.z;
		mVel.z = 0.0f;
	}
	dx  = sEggFeedTarget.x - mPos.x;
	dy  = sEggFeedTarget.y - mPos.y;
	len = feedSqrt(dx * dx + dy * dy);
	if (len < 8.0f) {
		eater = mEater.get();
		if (eater != 0) {
			msg.mType   = 0x18;
			msg.mKind   = (u8)mKind;
			msg.mAmount = (u8)mAmount;
			eater->sendToHandle((ObjMsg*)&msg);
			note.mType = 0xB3;
			note.mId   = eater->mHolderId;
			aeSendMessage(&note);
		}
		if (mRebirth != 0 && mSpawned == 0) {
			setLinkTimed((SetLink*)&mLink, 0x708);
		} else {
			setLinkDone((SetLink*)&mLink);
		}
		mLink.get()->setData((void*)1);
		mFlags |= 1;
	}
	feedScreen(&mPos, &screen);
	mtxSetTransV(mMtx, &screen);
	mtxRotateY(mMtx, mMtx, mSpin);
	mtxRotateX(mMtx, mMtx, mSpin << 1);
	mtxScale(mScale, mScale, mScale, mMtx, mMtx);
	mScale *= 0.96f;
	if (mScale < 0.1f) {
		mScale = 0.1f;
	}
	mSpin = (s16)(mSpin + 0x1000);
}

// 0x800CCF68
int TObjEggFeed::inReach(Vec3f* pos, f32 radius)
{
	Vec3f gap;
	f32 dist;
	f32 reach;

	reach = 4.0f * mScale;
	vecSub(&gap, &mPos, pos);
	dist = vecLength(&gap) - radius;
	if (dist <= 0.2f) {
		return 1;
	}
	if (dist > reach) {
		return 0;
	}
	dist  = reach - dist;
	mBob  = -dist / reach;
	gap.y = 0.0f;
	vecScale(0.0625f * dist, &mSway, &gap);
	return 0;
}

// 0x800CD034
int TObjEggFeed::land()
{
	EggHits hits;

	mVel.y += -0.04f;
	vecAdd(&mPos, &mPos, &mVel);
	if (mVel.y < 0.0f && meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&mPos, (unsigned long)1) != 0 && (hits.mMask & 1) != 0) {
		if (-(4.0f * mScale - mPos.y) <= hits.mHits[0].mPos.y) {
			colliEnable(mShape);
			mPos.y = 4.0f * mScale + hits.mHits[0].mPos.y;
			mVel.x *= 0.9f;
			mVel.y *= -0.7f;
			mVel.z *= 0.9f;
			if ((f32)fabs(mVel.y) < 0.4f) {
				mVel.x = 0.0f;
				mVel.y = 0.0f;
				mVel.z = 0.0f;
				riderSetMesh(&mGround, (RiderList*)(*(void**)(hits.mHits[0]._3C + 8)));
				return 1;
			}
		} else {
			colliDisable(mShape);
		}
	}
	return 0;
}

// 0x800CD17C
void TObjEggFeed::buildMatrix()
{
	f32 bob;
	f32 scale = mScale;
	f32 sx    = scale * (1.0f - 0.5f * (bob = mBob));
	Vec3f pos = mPos;

	vecAdd(&pos, &pos, &mSway);
	mtxSetTransV(mShadowMtx, &pos);
	mtxTranslate((float*)mShadowMtx, (const float*)mShadowMtx, 0.0f, 4.0f * -mScale, 0.0f);
	mtxScale(sx, scale * (1.0f + bob), sx, mShadowMtx, mShadowMtx);
	mtxTranslate((float*)mShadowMtx, (const float*)mShadowMtx, 0.0f, 4.0f, 0.0f);
	mtxRotateY(mMtx, mShadowMtx, mRot.y);
}

// 0x800CD28C
void TObjEggFeed::vfn_10()
{
	Vec3f view;
	Vec3f pos;

	if (objDebugEnabled(this) != 0) {
		mDraw.mFlags = 0;
		mTask->mFlags &= ~1;
	} else if ((mFeedFlags & 2) != 0) {
		mDraw.mFlags    = 0;
		mTask->mSortKey = drawTaskDepthKey(0.0f);
		mTask->mFlags &= ~1;
	} else {
		const Vec3f* at = &colliGetShapes(mShape)->mPos;
		pos.set(at->x, at->y, at->z);
		if (mLight.checkView(500.0f, colliGetShapes(mShape)->mSizeX, &pos, &view) != 0) {
			if ((u8)mShade == 0xFF) {
				setDrawAmbient(&mLight, &mDraw);
			} else {
				mDraw.mFlags = 0x200;
				setColorAlpha(&mDraw.mAmbient, (u8)mShade);
			}
			mTask->mSortKey = drawTaskDepthKey(view.z);
			mTask->mFlags &= ~1;
		} else {
			mTask->mFlags |= 1;
		}
	}
}

// 0x800CD3C0
void TObjEggFeed::render()
{
	if ((mFeedFlags & 0x10) != 0) {
		if (eventIsBusy() == 0) {
			drawUnit();
		}
	} else {
		drawPass();
	}
}

// 0x800CD410
void TObjEggFeed::drawPass()
{
	if (gEggFeedFlag != 0) {
		if ((mFeedFlags & 8) != 0) {
			drawSpecial();
		}
	} else {
		if ((mFeedFlags & 4) != 0) {
			drawModel();
		}
		if ((mFeedFlags & 8) != 0) {
			drawShadow();
		}
	}
}

// 0x800CD47C
void TObjEggFeed::drawUnit()
{
	gjPushUnitMatrix();
	njMultiMatrix(0, mMtx);
	mModel.draw();
	njPopMatrixEx();
}

// 0x800CD4C0
void TObjEggFeed::drawModel()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMtx);
	mModel.draw();
	njPopMatrixEx();
}

// 0x800CD504
void TObjEggFeed::drawShadow()
{
	njPushMatrixEx();
	njMultiMatrix(0, mShadowMtx);
	mShadow.draw();
	njPopMatrixEx();
}

static const GXColor sEggFeedWhite = { 0xFF, 0xFF, 0xFF, 0xFF };

// The feed quad indexed GX arrays
const f32 sEggFeedPositions[12] __attribute__((aligned(32))) = {
	-4.0f, -4.0f, 0.0f, -4.0f, 4.0f, 0.0f, 4.0f, 4.0f, 0.0f, 4.0f, -4.0f, 0.0f,
};
const f32 sEggFeedTexCoords[8] __attribute__((aligned(32))) = {
	0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
};

// 0x800CD548
static void setupFeedTex(u16 kind)
{
	volatile u16* fifo;

	gjLoadPosMatrix(0);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetZCompLoc(GX_FALSE);
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0, sEggFeedWhite);
	gjLoadTextureNum(kind, 0, 0, 0);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
	GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetArray(GX_VA_POS, (void*)sEggFeedPositions, 0xC);
	GXSetArray(GX_VA_TEX0, (void*)sEggFeedTexCoords, 8);
	pushGXCullMode(2);
	GXBegin(GX_QUADS, GX_VTXFMT0, 4);
	fifo  = (volatile u16*)0xCC008000;
	*fifo = 0;
	*fifo = 0;
	*fifo = 1;
	*fifo = 1;
	*fifo = 2;
	*fifo = 2;
	*fifo = 3;
	*fifo = 3;
	popGXCullMode();
}

// 0x800CD73C
void TObjEggFeed::drawSpecial()
{
	f32 mtx[12];

	mtxTranspose(mtx, (f32*)gjGetMatrix());
	mtxConcat(mtx, mShadowMtx, mtx);
	mtxConcat(mtx, (f32*)gjGetMatrix(), mtx);
	njPushMatrix(mtx);
	NJS_TEXLIST* tex = getStgObjTextures()[46];
	if (njGetCurrentTexList() != tex) {
		njSetTexture(*(void**)((u8*)getStgObjTextures() + 0xB8));
	}
	setupFeedTex((u16)mKind);
	njPopMatrixEx();
}

// 0x800CD7D0
// is __sinit_TObjEggFeed_cpp which builds sEggFeedTarget
EggFeedTarget sEggFeedTarget(570.0f, 400.0f, 40.0f);
