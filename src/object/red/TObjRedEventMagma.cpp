#include "object/red/TObjRedEventMagma.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "camera/TCamera.h"
#include "camera/EventCamRef.h"
#include "object/ObjScript.h"
#include "object/ObjArchive.h"
#include "camera/TCamPath.h"
#include "object/TObjLauncher.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "stage/TGameState.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjKey00.h"
#include "engine/TParticle.h"
#include "sound/SoundCtrl.h"
#include "stage/SetSlot.h"
#include "engine/TFrameBlur.h"
#include "engine/GroundQuery.h"
#include "core/TMainTask.h"
#include "Ninja/ninja.h"
struct SetLink;

extern "C" {

// 0x802C4058
ObjClassName gTObjRedEventMagmaName      = { "TObjRedEventMagma", 0 };
extern const int sRedEventMagmaModels[3] = {
	5,
	6,
	7,
};
extern const int sRedEventMagmaData[3]            = { 5, 6, 7 };
extern const int sRedEventMagmaNodes1[3]          = { 2, 3, 4 };
extern const int sRedEventMagmaNodes2[3]          = { 2, 3, 4 };
extern const MotionEntry sRedEventMagmaMotions[3] = {
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 29.0f, 0.1f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 29.0f, 0.1f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 29.0f, 0.1f, 0.0f, 0 },
};
extern const int sRedEventMagmaFlags[] = {
	39, 2, 10, 1, 40, 46,
};
}

typedef char TObjRedEventMagma_size_check[(sizeof(TObjRedEventMagma) == 0x168) ? 1 : -1];

static const MotionEntry* magmaMotions()
{
	return sRedEventMagmaMotions;
}

static int* magmaTex()
{
	return (int*)sRedEventMagmaData;
}

static int* magmaModel2()
{
	return (int*)sRedEventMagmaNodes1;
}

static int* magmaTex2()
{
	return (int*)sRedEventMagmaNodes2;
}

/** SetRot as the compiler copies it a word pair then z */
struct MagmaRotCopy {
	struct {
		int x, y;
	} xy;
	int z;
};

static void flagShow(void*, MagmaRec* rec);
static void flagHide(void*, MagmaRec* rec);
static void flagClear(void*, MagmaRec* rec);
static void bindRecords(int* dst, int* src);

// 0x800C70F8
void loadMagmaArc()
{
	loadArc("ar_obj_red_event_magma.arc", &sRedEventMagmaArc);
	sRedEventMagmaAssets = sRedEventMagmaArc->mData;
}

// 0x800C7130
void releaseMagmaArc()
{
	releaseResource(sRedEventMagmaArc);
	sRedEventMagmaArc    = 0;
	sRedEventMagmaAssets = 0;
}

// 0x800C7160
extern "C" TObject* createTObjRedEventMagma(TObject* parent, SetDesc* desc)
{
	return new TObjRedEventMagma(parent, desc);
}

// 0x800C71B4
void TObjRedEventMagma::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mTrans.mPos                  = mPos;
	*(MagmaRotCopy*)&mTrans.mRot = *(MagmaRotCopy*)&mRot;
	mScroll.readDesc(desc);
	mModelId = (u16)desc->getInt(0);
	mModelId = mModelId > 2 ? 2 : mModelId;
	mGeo     = getGeObjModels()[sRedEventMagmaModels[mModelId]];
	mTex     = getGeObjTextures()[sRedEventMagmaData[mModelId]];
	mScroll.bindModel(*(void**)((u8*)mGeo + 4), &mTrans);
	mEventId = (u16)desc->getInt(3);
}

// 0x800C72CC
void TObjRedEventMagma::debugPrint(void*)
{
	njPrint(0xA0013, ":MODEL ID");
	njPrint(0xA0016, ":SW ID");
	mScroll.debugPrint();
	mScroll.debugPrintTrans(&mTrans);
}

// 0x800C7334
TObjRedEventMagma::TObjRedEventMagma(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjRedEventMagmaName.mName;
	mSize = 0x168;
	initFromDesc(desc);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	setup();
}

// 0x800C73F4
TObjRedEventMagma::~TObjRedEventMagma()
{
	if ((u32)mSound != -1) {
		seStop(mSound);
	}
	freeFxB();
	freeFx();
	removeDrawTask(mDrawTask);
}

// 0x800C74CC
static inline f32 magmaSqrt(f32 x)
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

void TObjRedEventMagma::setup()
{
	mFxB        = 0;
	mFx         = 0;
	void** rows = *(void***)((u8*)sRedEventMagmaAssets + 8);
	bindRecords(&mCount, (int*)rows[mModelId]);
	mMotion.init(sRedEventMagmaMotions, gGeObjCommon->mSet.mMotions);
	mMotion.play(mModelId);
	if (keyBitIsSet((u16)mEventId)) {
		mMotion.update();
		NJS_MOTION* motion = mMotion.getMotion(mMotion.mCurrent);
		mMotion.mFrame     = (f32)motion->nbFrame - 1.0f;
		bindShut();
		spawnFour();
		setAct(2);
	} else {
		bindOpen();
		spawnRest();
		setAct(0);
	}

	void* model = getGeObjModels()[sRedEventMagmaNodes1[mModelId]];
	void* tex   = getGeObjTextures()[sRedEventMagmaNodes2[mModelId]];
	mModel.setModel(model);
	mModel.mTexList = (NJS_TEXLIST*)tex;
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mModel.mMotion  = &mMotion;

	f32* node = *(f32**)((u8*)model + 0x2C);
	mCentre.set(node[2], node[3], node[4]);
	mRadius = node[8] * node[8];
	mRadius += node[9] * node[9];
	mRadius += node[10] * node[10];
	mRadius = magmaSqrt(mRadius);
	setLinkFree((SetLink*)&mLink);
}

// 0x800C76E0
// Clears bit 0 and sets 0x100 on the record the walker passes
static void flagShow(void*, MagmaRec* rec)
{
	rec->mFlags &= ~1u;
	rec->mFlags |= 0x100;
}

// 0x800C76FC
// Clears 0x100 and sets bit 0
static void flagHide(void*, MagmaRec* rec)
{
	rec->mFlags &= ~0x100u;
	rec->mFlags |= 1u;
}

// 0x800C7718
static void flagClear(void*, MagmaRec* rec)
{
	rec->mFlags = 0;
}

// 0x800C7724
int TObjRedEventMagma::sendToHandle(ObjMsg* msg)
{
	if (msg->mType == 199) {
		mCam.set(msg);
		return 1;
	}
	return 0;
}

// 0x800C7760
void TObjRedEventMagma::setAct(int act)
{
	mStep = 0;
	mAct  = act;
}

// 0x800C7770
void TObjRedEventMagma::draw()
{
	if (isInRange(this, &mPos, 4000.0f) == 0 && mCam.isActive() == 0) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		idle();
		break;
	case 1:
		play();
		break;
	case 2:
		settled();
		break;
	}
}

// 0x800C7814
void TObjRedEventMagma::idle()
{
	mScroll.update();
	if (mCam.isActive() == 0) {
		return;
	}
	spawnFour();
	spawnNine();
	bindShut();
	void** arc = (void**)sRedEventMagmaAssets;
	int index  = mModelId * 4;
	createCamPath(0, 3, *(void**)((u8*)arc[0] + index), *(void**)((u8*)arc[1] + index), 1, 0.0f, 0.1f);
	mSound = sePlayFor(0x221, 0);
	setAct(1);
}

// 0x800C78BC
void TObjRedEventMagma::play()
{
	mMotion.update();
	if (mMotion.isDone() == 0) {
		return;
	}
	freeFxB();
	frameBlurStart(0, 0.0f);
	seStop(mSound);
	mSound = -1;
	ObjMsg msg;
	msg.mType = 0x92;
	sendGameMsg(&msg);
	mCam.release();
	setAct(2);
}

// 0x800C7944
void TObjRedEventMagma::settled()
{
}

// 0x800C7948
void TObjRedEventMagma::bindOpen()
{
	groundWalkGroup((void*)sRedEventMagmaFlags[mModelId * 2], (void*)flagShow, (void*)this);
	groundWalkGroup((void*)sRedEventMagmaFlags[mModelId * 2 + 1], (void*)flagShow, (void*)this);
}

// 0x800C79BC
void TObjRedEventMagma::bindShut()
{
	groundWalkGroup((void*)sRedEventMagmaFlags[mModelId * 2], (void*)flagHide, (void*)this);
	groundWalkGroup((void*)sRedEventMagmaFlags[mModelId * 2 + 1], (void*)flagClear, (void*)this);
}

// 0x800C7A30
// Every row that is not kind 4 or 9
void TObjRedEventMagma::spawnRest()
{
	freeFx();
	mFx = new TObject(gTL_PARTICLE);
	if (mFx == 0) {
		return;
	}
	for (int i = 0; i < mCount; i++) {
		MagmaRec* rec = (MagmaRec*)((u8*)mRecs + i * 0x38);
		if (rec->mKind != 4 && rec->mKind != 9) {
			Vec3f pos;
			pos.set(rec->mPos.x, rec->mPos.y, rec->mPos.z);
			particleGenById(mFx, &pos, 1, rec->mKind);
		}
	}
}

// 0x800C7AF0
void TObjRedEventMagma::spawnFour()
{
	freeFx();
	mFx = new TObject(gTL_PARTICLE);
	if (mFx == 0) {
		return;
	}
	for (int i = 0; i < mCount; i++) {
		MagmaRec* rec = (MagmaRec*)((u8*)mRecs + i * 0x38);
		if (rec->mKind == 4) {
			Vec3f pos;
			pos.set(rec->mPos.x, rec->mPos.y, rec->mPos.z);
			particleGenById(mFx, &pos, 1, rec->mKind);
		}
	}
}

// 0x800C7BA8
void TObjRedEventMagma::spawnNine()
{
	freeFxB();
	mFxB = new TObject(gTL_PARTICLE);
	if (mFxB == 0) {
		return;
	}
	for (int i = 0; i < mCount; i++) {
		MagmaRec* rec = (MagmaRec*)((u8*)mRecs + i * 0x38);
		if (rec->mKind == 9) {
			Vec3f pos;
			pos.set(rec->mPos.x, rec->mPos.y, rec->mPos.z);
			particleGenTimedById(mFxB, &pos, 1, rec->mKind, rec->mCount * 10, 1);
		}
	}
}

// 0x800C7C6C
void TObjRedEventMagma::freeFx()
{
	if (mFx == 0) {
		return;
	}
	mFx->mFlags |= 1;
	mFx = 0;
}

// 0x800C7C90
void TObjRedEventMagma::freeFxB()
{
	if (mFxB == 0) {
		return;
	}
	mFxB->mFlags |= 1;
	mFxB = 0;
}

// 0x800C7CB4
void TObjRedEventMagma::vfn_10()
{
	f32 span     = scrollModelRadius(&mTrans);
	f32 radius   = 10.0f + span;
	f32 reach    = 1000000.0f + radius;
	int onTrans  = cameraIsVisible(reach, radius, &mTrans.mPos, 0);
	int onCentre = cameraIsVisible(reach, mRadius, &mCentre, 0);
	if (onTrans != 0 || onCentre != 0) {
		mDrawTask->mFlags &= 0xFFFE;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x800C7D60
void TObjRedEventMagma::render()
{
	mModel.draw();
	if (mAct == 0) {
		mScroll.draw(&mTrans, mGeo, mTex);
	}
}

// 0x800C7DB0
// The archive word is a count followed by the records
static void bindRecords(int* dst, int* src)
{
	dst[0] = src[0];
	dst[1] = (int)(src + 1);
}
