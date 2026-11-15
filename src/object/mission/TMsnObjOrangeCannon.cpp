#define DRAW_VIEW_RETURNS_U16
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "io/Resource.h"
#include "camera/EventCamArc.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "enemy/EnemyLight.h"
#include "egg/ObjEgg.h"
#include "engine/ModelDraw.h"
#include "engine/FxHolder.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/orange/TObjOrangeFireworksBall.h"
#include "object/orange/TObjOrangeFireworks.h"
#include "object/ObjShadow.h"
#include "stage/TEventCtrl.h"
#include "stl/math.h"
#include "ui/AeArc.h"
#include "object/mission/TMsnObjOrangeCannon.h"
#include "object/TObjLauncher.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "camera/TCamPath.h"
#include "stage/TGameState.h"
#include "engine/FxSystem.h"
#include "engine/Color.h"
#include "object/TObjEggRing.h"
#include "engine/Colli.h"
#include "core/VecMath.h"
#include "object/TObjKey00.h"
#include "sound/SoundCtrl.h"
#include "object/TObjPlayer.h"
#include "player/TPlyItem.h"
struct NJS_TEXLIST;
struct CharaColli;
struct DrawState;
struct Fx;

static f32 sDrawDist = gOrangeCannonSetRange - 100.0f; // 0x802C5958
static const f32 sKeepDist = 50.0f + gOrangeCannonSetRange;  // 0x802C595C

/** Message 0x7D which puts the player at a place facing an angle for the stage clear */
struct StageClearMsg {
	u16 mType;  // _00
	Vec3f mPos; // _04
	int mAngle; // _10
};

// geobj_orange_cannon.arc and its exports
extern "C" {
TArc* gOrangeCannonArc;      // 0x802C5960
NJS_OBJECT* gOrangeCannonModels;   // 0x802C5964 *"models"
NJS_TEXLIST* gOrangeCannonTexLists; // 0x802C5968 *"texlists"
NJS_MOTION** gOrangeCannonMotions;  // 0x802C596C "motions"
void* gOrangeCannonPath;     // 0x802C5970 "path"
}

const MotionEntry sMsnOrangeCannonMotions[] = {
	{ 1, 0, -1, 0, 0.0f, 150.0f, 1.0f, 0.0f, 0 },
	{ 1, 0, -1, 0, 150.0f, 150.0f, 0.0f, 0.0f, 0 },
};

// 0x801883B0
// A draw callback node 0 takes the current matrix
extern "C" void copyCameraMatrix(int node, f32* mtx)
{
	if (node == 0) {
		mtxCopy(mtx, (f32*)gjGetMatrix());
	}
}

// 0x801883F0
extern "C" void resetOrangeCannonAssets()
{
	gOrangeCannonArc      = NULL;
	gOrangeCannonModels   = NULL;
	gOrangeCannonTexLists = NULL;
	gOrangeCannonMotions  = NULL;
	gOrangeCannonPath     = NULL;
}

// 0x8018840C
extern "C" void releaseOrangeCannonAssets()
{
	if (gOrangeCannonArc != NULL) {
		releaseTextures(gOrangeCannonTexLists);
		releaseResource(gOrangeCannonArc);
		gOrangeCannonArc      = NULL;
		gOrangeCannonModels   = NULL;
		gOrangeCannonTexLists = NULL;
		gOrangeCannonMotions  = NULL;
		gOrangeCannonPath     = NULL;
	}
}

// 0x8018845C
extern "C" int prepareOrangeCannonAssets()
{
	loadOrangeCannonAssets();
	return 1;
}

// 0x80188480
extern "C" void loadOrangeCannonAssets()
{
	if (gOrangeCannonArc == NULL) {
		loadArc("geobj_orange_cannon.arc", &gOrangeCannonArc);
		gOrangeCannonModels   = *(NJS_OBJECT**)findArcExport(gOrangeCannonArc, "models");
		gOrangeCannonTexLists = (NJS_TEXLIST*)(*(void**)findArcExport(gOrangeCannonArc, "texlists"));
		gOrangeCannonMotions  = (NJS_MOTION**)findArcExport(gOrangeCannonArc, "motions");
		gOrangeCannonPath     = findArcExport(gOrangeCannonArc, "path");
		bindGvmTextures((u8*)findArcExport(gOrangeCannonArc, "texture"), gOrangeCannonTexLists);
	}
}

f32 gOrangeCannonSetRange = 800.0f;
static const char* sClassName = "TMsnObjOrangeCannon";

typedef char TMsnObjOrangeCannon_size_check[(sizeof(TMsnObjOrangeCannon) == 0x2A0) ? 1 : -1];

// 0x80188518
extern "C" TObject* createTMsnObjOrangeCannon(TObject* parent, SetDesc* desc)
{
	return new TMsnObjOrangeCannon(parent, desc);
}

// 0x8018856C
TMsnObjOrangeCannon::TMsnObjOrangeCannon(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00 = (u32)sClassName;
	mSize = 0x2A0;
	mShape = colliCreate(2, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 255, 0xF6, 0xF6);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	initFromDesc(desc);
	setUp();
}

// 0x801886A8
TMsnObjOrangeCannon::~TMsnObjOrangeCannon()
{
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x80188768
// The two spheres sit in front of the barrel and behind it turned with the cannon
void TMsnObjOrangeCannon::initFromDesc(SetDesc* desc)
{
	Vec3f at;

	TSetActor::initFromDesc(desc);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);

	at.x = 0.0f;
	at.y = 15.0f;
	at.z = 10.0f;
	vecRotateY(&at, &at, mRot.y);
	vecAdd(&at, &at, &mPos);
	colliShapeMakeSphere(colliGetShape(mShape, 0), &at, 0, 12.0f);
	colliGetShape(mShape, 0)->mFlags |= 0x10;

	at.x = 0.0f;
	at.y = 15.0f;
	at.z = -10.0f;
	vecRotateY(&at, &at, mRot.y);
	vecAdd(&at, &at, &mPos);
	colliShapeMakeSphere(colliGetShape(mShape, 1), &at, 0, 25.0f);

	if (gOrangeCannonModels != 0) {
		mLightPos = mPos;
		mLightPos.y += 20.0f;
		mLightPos.z += -10.0f;
		mLightRadius = 50.0f;
	} else {
		mLightPos    = mPos;
		mLightRadius = 25.0f;
	}
	shadowSetGround(&mShadow, &mPos, 0);
}

// 0x801888F0
void TMsnObjOrangeCannon::debugPrint(void*)
{
	njPrintC(0xA0013, "");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
}

// 0x8018894C
// Places the barrels and the burst from the models and starts fired when key bit 0xFA is set
void TMsnObjOrangeCannon::setUp()
{
	mLight.mTimer = 0;
	mEvent        = 0;
	mFireworks    = 0;
	mTimer        = 0;
	mFx           = 0;
	mShadow._30   = 40.0f;
	mShadow._34   = 40.0f;
	shadowEnable(&mShadow);

	if (gOrangeCannonArc != 0) {
		CannonNode* burst;
		void* texList;
		TArc* arc;

		mMotion.init(sMsnOrangeCannonMotions, gOrangeCannonMotions);
		mModel.setModel(gOrangeCannonModels);
		mModel.mState   = &mState;
		mModel.mTexList = gOrangeCannonTexLists;
		mModel.mMotion  = &mMotion;

		CannonNode* barrel[2] = { 0, 0 };
		CannonNode* first = ((CannonNode*)gOrangeCannonModels)->mChild->mSibling;
		barrel[0] = first;
		barrel[1] = first->mSibling;
		mMuzzle[0] = barrel[0]->mPos;
		mMuzzle[1] = barrel[1]->mPos;
		mMuzzleRot[0].set(barrel[0]->mAng.x, barrel[0]->mAng.y, barrel[0]->mAng.z);
		mMuzzleRot[1].set(barrel[1]->mAng.x, barrel[1]->mAng.y, barrel[1]->mAng.z);
		mtxMultVec(mMatrix, &mMuzzle[0], &mMuzzle[0]);
		mtxMultVec(mMatrix, &mMuzzle[1], &mMuzzle[1]);

		texList = 0;
		arc     = getFireworksBallArc();
		if (arc != 0) {
			burst             = *(CannonNode**)findArcExport(arc, "models");
			texList           = *(void**)findArcExport(arc, "texlists");
			mBurst            = *burst;
			mBurst.mChild     = 0;
		}
		mTrack.set(gOrangeCannonPath, 0, 1.0f);
		mDraw.setModel(&mBurst);
		mDraw.mDrawState = getDefaultDrawState();
		mDraw.mTexList   = (NJS_TEXLIST*)texList;
		mDraw.mTrack     = &mTrack;
		mDraw.mMatrix    = mMatrix;
		mDraw.setNodeFunc((void*)copyCameraMatrix, mFxMtx);
		mDraw.walkNodes();
		mDraw.setNodeFunc(0, 0);
	}

	if (keyBitIsSet(0xFA)) {
		mMotion.play(1);
		mMotion.update();
		mAct = 3;
	} else {
		mMotion.play(0);
		mAct = 0;
	}
	mStep = 0;
}

// 0x80188BE8
int TMsnObjOrangeCannon::sendToHandle(ObjMsg* msg)
{
	return 0;
}

// 0x80188BF0
void TMsnObjOrangeCannon::draw()
{
	if (objDebugEnabled(this) == 0) {
		switch (mAct) {
		case 0:
			updateWait();
			break;
		case 1:
			updateStart();
			break;
		case 2:
			updateFire();
			break;
		case 3:
			updateDone();
			break;
		}
	}
}

// 0x80188C7C
// Waits for a fireworks ball to touch the cannon
void TMsnObjOrangeCannon::updateWait()
{
	if (!isInRange((void*)this, (const Vec3f*)&mPos, sKeepDist)) {
		mFlags |= 1;
		return;
	}

	ColliContact* contact = ((CharaColli*)mShape)->mContacts;
	for (; contact != 0; contact = contact->mNext) {
		if (contact->mShape != 0) {
			continue;
		}
		if (colliContactNoBit10(contact) == 0) {
			continue;
		}
		TObjOrangeFireworksBall* ball = (TObjOrangeFireworksBall*)findObjById((u16)colliContactOwner(contact));
		if (ball == 0 || ball->_00 != (u32)gTObjOrangeFireworksBallName) {
			continue;
		}
		mPlayer.setId(playerGetDrawView());
		if (mPlayer.get() != 0) {
			StageClearMsg msg;
			msg.mType  = 0x7D;
			msg.mPos.x = 30.0f;
			msg.mPos.y = 0.0f;
			msg.mPos.z = 30.0f;
			mtxMultVec(mMatrix, &msg.mPos, &msg.mPos);
			msg.mAngle = mRot.y + 0x8000;
			mPlayer.get()->sendToHandle((ObjMsg*)&msg);
		}
		ball->remove();
		mAct = 1;
	}
}

// 0x80188DB0
// The event's start hides the player for it and starts firing
static int onEventStart(int id)
{
	TMsnObjOrangeCannon* cannon = (TMsnObjOrangeCannon*)findObjById(id);
	if (cannon == 0) {
		return 0;
	}
	eventHideIdU16(cannon->mPlayer.getId());
	cannon->mAct = 2;
	return 1;
}

// 0x80188E04
void TMsnObjOrangeCannon::updateStart()
{
	if (eventStartById((u16)mObjId, onEventStart)) {
		mAct = 3;
	}
}

// 0x80188E4C
void TMsnObjOrangeCannon::updateFire()
{
	switch (mStep) {
	case 0:
		stepOpen();
		break;
	case 1:
		stepAim();
		break;
	case 2:
		stepShoot();
		break;
	case 3:
		stepWait();
		break;
	case 4:
		stepEnd();
	default:
		mAct = 3;
		break;
	}
}

// 0x80188ED0
void TMsnObjOrangeCannon::stepOpen()
{
	mEvent = eventCamArcPlay(1.0f, 0.0f, 0x100);
	if (mEvent == 0) {
		mStep = 4;
	} else {
		((TCamera*)mEvent)->setCut();
		mTimer = 0;
		mStep  = 1;
	}
}

// The motion's frame rounded to the nearest whole frame
static inline f32 cannonFrame(MotionPlayer* motion) { return (f32)floor(0.5f + motion->mFrame); }

// 0x80188F38
// At frame 115 of the motion the burst's glow starts
void TMsnObjOrangeCannon::stepAim()
{
	mMotion.update();
	if (cannonFrame(&mMotion) == 115.0f) {
		mFx    = fxSpawnFollow(1, 8, 6, mFxMtx);
		mTimer = 0;
		mStep  = 2;
	} else {
		mTimer++;
	}
}

// 0x80188FC0
// When the burst's motion ends the glow stops and the fireworks go up from the burst
void TMsnObjOrangeCannon::stepShoot()
{
	if (mTrack.isDone()) {
		Vec3f pos;
		fxStop(mFx);
		mFx    = 0;
		mTimer = 0;
		mtxGetPos(mFxMtx, &pos);
		mFireworks = createTObjOrangeFireworks(500.0f, this, &pos, 3);
		mStep      = 3;
	} else {
		mTimer++;
	}

	mMotion.update();
	mTrack.update();
	mDraw.setNodeFunc((void*)copyCameraMatrix, mFxMtx);
	mDraw.walkNodes();
	mDraw.setNodeFunc(0, 0);

	if (cannonFrame(&mMotion) == 120.0f) {
		f32 mtx[12];
		mtxSetTransV(mtx, &mMuzzle[0]);
		mtxRotateY(mtx, mtx, mMuzzleRot[0].y);
		mtxRotateX(mtx, mtx, mMuzzleRot[0].x);
		fxSpawnAt(1, 8, 9, mtx);
		mtxSetTransV(mtx, &mPos);
		fxSpawnAt(1, 8, 0xC, mtx);
		sePlay(0x247);
	}
}

// 0x8018910C
void TMsnObjOrangeCannon::stepWait()
{
	if (mEvent->isDone()) {
		if (mFireworks != 0) {
			mFireworks->mFlags |= 1;
			mFireworks = 0;
		}
		mFireworks = 0;
		if (mEvent != 0) {
			mEvent->mFlags |= 1;
			mEvent = 0;
		}
		mStep = 4;
	}
}

// 0x80189190
void TMsnObjOrangeCannon::stepEnd()
{
	ObjMsg done;
	ObjMsg clear;

	if (mPlayer.get() != 0) {
		done.mType = 0x79;
		mPlayer.get()->sendToHandle(&done);
	}
	eventEnd();
	clear.mType = 0x9E;
	sendGameMsg(&clear);
}

// 0x80189200
void TMsnObjOrangeCannon::updateDone() { }

// 0x80189204
void TMsnObjOrangeCannon::vfn_10()
{
	if (eventIsBusy()) {
		mState.mFlags = 0;
		mDrawTask->mFlags &= ~1;
	} else if (mLight.isVisible(mDrawTask, sDrawDist, &mLightPos, mLightRadius, 0)) {
		setDrawAmbient(&mLight, &mState);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x801892A8
// Without its archive the cannon draws as a black ball
void TMsnObjOrangeCannon::render()
{
	if (gOrangeCannonArc != 0) {
		f32 mtx[12];
		mtxConcat(mtx, gjGetMatrix(), mMatrix);
		shadowAlphaBegin();
		njPushMatrix(mtx);
		mModel.draw();
		njPopMatrixEx();
		if (mAct == 2 && mStep == 2) {
			mDraw.draw();
		}
		shadowAlphaEnd();
	} else {
		f32 mtx[12];
		mtxConcat(mtx, gjGetMatrix(), mMatrix);
		mtxScale(mLightRadius, mLightRadius, mLightRadius, mtx, mtx);
		debugWireSphere(mtx, &gColorBlack);
	}
}

inline FxVec::FxVec() { }
