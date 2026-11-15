#include "animal/TAnimalRacer.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "core/MathUtil.h"
#include "io/ResBank.h"
#include "animal/TAniRacePath.h"
#include "stage/TEventCtrl.h"
#include "egg/TEggCreatureTypeE.h"
#include "object/ObjShadow.h"
#include "camera/TCamera.h"
#include "ui/AeArc.h"
#include "object/TObjLauncher.h"
#include "engine/Color.h"
#include "core/Mem.h"
#include "engine/DrawState.h"
#include "object/ObjAction.h"
#include "camera/TCamPath.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "ui/MessageTable.h"
#include "object/TObjKey00.h"
#include "ui/TSimpleText.h"
#include "animal/TAniKijiSai.h"
#include "animal/TAniHariAzarashi.h"
#include "animal/TAniKumaKame.h"
#include "animal/TAniPelicanDog.h"
#include "animal/TAniRabitOwl.h"
#include "core/TMainTask.h"
#include "animal/TAniClionePenguin.h"
#include "animal/TAniOrixPanther.h"
#include "animal/TAniBullBoar.h"
#include "animal/TAniDevilMomonga.h"
struct Vec3f;
struct GjActor;
struct NJS_TEXLIST;
struct DrawStateSlot;
struct ColliShape;
struct CharaColli;
struct EnemyLight;
struct DrawState;

// The racer's archive file

typedef char RacerMover_size_check[(sizeof(RacerMover) == 0x60) ? 1 : -1];
typedef char RacerShape_size_check[(sizeof(RacerShape) == 0x30) ? 1 : -1];
typedef char TAnimalRacer_size_check[(sizeof(TAnimalRacer) == 0x288) ? 1 : -1];

// 0x801B11A0
// Each animal's motion table in the order of aniModelArcName
void* racerMotionTable(int kind)
{
	switch (kind) {
	case 0:
		return gClioneMotions;
	case 1:
		return gKijiMotions;
	case 2:
		return gHariMotions;
	case 3:
		return gSaruKoumoriMotions;
	case 4:
		return gKumaMotions;
	case 5:
		return gPelicanMotions;
	case 6:
		return gRacerMotions;
	case 7:
		return gOrixMotions;
	case 8:
		return gBullBoarMotions;
	case 9:
		return gMomongaMotions;
	default:
		return NULL;
	}
}

// 0x801B1240
// The event command load_racer the argument is the animal
int racerLoadEvent(void* arg)
{
	racerLoad(*(u16*)arg);
	return 1;
}

// 0x801B1268
void racerLoad(int kind)
{
	loadArc(aniModelArcName(kind), &gRacerArc);
	gRacerPack = (RacerPack*)gRacerArc->mData;
	bindGvmTextures(gRacerPack->mGvm, (NJS_TEXLIST*)*gRacerPack->mModel);
	gRacerKind = kind;
}

// 0x801B12BC
void racerFree()
{
	if (gRacerPack != NULL) {
		releaseTextures((NJS_TEXLIST*)*gRacerPack->mModel);
		gRacerPack = NULL;
	}
	if (gRacerArc != NULL) {
		releaseResource(gRacerArc);
		gRacerArc = NULL;
	}
}

// 0x801B1310
// TAniRacePath calls this when the race starts
void racerInit()
{
}

// 0x801B1314
void racerExit()
{
	racerFree();
}

// 0x801B1334
TAnimalRacer* createTAnimalRacer(const Vec3f* pos, f32 scale)
{
	return new TAnimalRacer(gTL_05, pos, scale);
}

const char* gTAnimalRacerName = "TAnimalRacer";

// 0x801B1394
TAnimalRacer::TAnimalRacer(TObject* parent, const Vec3f* pos, f32 scale)
    : TObjAction(parent)
    , mStart(*pos)
    , mScale(scale)
{
	setObjId(this, 0xFFFF);
	_00   = (u32)gTAnimalRacerName;
	mSize = 0x288;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	setup();
	setMode(0);
}

// 0x801B1490
TAnimalRacer::~TAnimalRacer()
{
	if (mMark != 0) {
		mMark->mFlags |= 1;
		mMark = 0;
	}
	if (mBuf != 0) {
		memFree(mBuf);
		mBuf = 0;
	}
	if (mFx != 0) {
		colliRetire((CharaColli*)mFx);
		mFx = 0;
	}
	aniRaceClearRacer(this);
	removeDrawTask(mTask);
	releaseObjId(this);
}

// 0x801B157C
void TAnimalRacer::setup()
{
	void* file;
	s8 kind;

	mRate = 1.0f;
	aniRaceSetGlobal(gRacerKind);
	if (gRacerPack != 0) {
		file = *gRacerPack->mHead;
		mBuf = (GjActor*)(memAlloc(gjActorSize((NJS_OBJECT*)file)));
		gjCreateActor((NJS_OBJECT*)file, (u8*)mBuf);
		mModel = *gRacerPack->mModel;
		mTable = (MotionEntry*)racerMotionTable(gRacerKind);
		mPlayer.init(mTable, gRacerPack->mMotions);
		mPlayer.play(1);
		pose();
	}
	mCourse = (void*)gAnimalCommonModel;
	mInfo   = (RacerInfo*)gAnimalParams[gRacerKind];
	mMotion = mStep = mPrevMode = mMode = -1;
	_38                                 = 0;
	const RacerInfo* info               = mInfo;
	kind                                = (s8)(info->mKind - 3);
	if (gRacerKind == 9) {
		kind = 0;
	}
	mMover.mKind     = (u8)kind;
	mMover.mAccel    = info->mCap;
	mMover.mTopSpeed = 5.0f;
	mMover.mF0C      = 0.05f;
	mMover.mTurn     = 0x400;
	mMover.mPos      = mStart;
	mMover.mGravity  = -0.08166667f;
	mPos             = mStart;
	mVel.z = mVel.y = mVel.x = 0.0f;
	mRotZ = mRotY = mRotX = 0;
	mtxIdentity(mMtx);
	mRef.setId(0xFFFF);
	mClips     = (RacerClip*)*mInfo->mClips;
	mClipCount = (u8)mInfo->mClipCount;
	mJump      = 0;
	mAim       = 0;
	mMark      = 0;
	mSpark     = 0;
	mFlags     = 0;
	mFx        = colliCreate(1, 1);
	if (mFx != 0) {
		colliSetOwner(mFx, (u16)mObjId);
		colliSetMasks(mFx, 0x20, 0, 0, 0x10);
	}
}

// 0x801B1788
void TAnimalRacer::draw()
{
	Vec3f* home;

	if (mFx != 0) {
		colliPlaceShapes(mFx, &mPos, (const ColliShape*)&mShape);
	}
	mNudge.z = mNudge.y = mNudge.x = 0.0f;
	runMode((int)mMode, 2);
	buildMtx();
	if ((mFlags & 2) == 0) {
		applyMotion();
	}
	probeShadow();
	if (mPos.y < -3000.0f) {
		home = (Vec3f*)aniRaceGetPos();
		mPos = *home;
		setMode(1);
	}
}

// 0x801B1844
void TAnimalRacer::vfn_10()
{
	int vis;

	vis = mLight.isVisible(mTask, 1000.0f, &mPos, 40.0f, 0);
	if (vis != 0) {
		mTask->mFlags &= (u16)~1;
	} else {
		mTask->mFlags |= 1;
	}
}

// 0x801B18B4
void TAnimalRacer::render()
{
	DrawStateSlot* slot;
	void* plain;

	setDrawAmbient(&mLight, &mDraw);
	slot = getDrawStateSlot();
	njPushMatrixEx();
	njMultiMatrix(0, mMtx);
	beginDrawState(slot, (const DrawState*)&mDraw);
	njSetTexture(mModel);
	fireClips();
	shadowAlphaBegin();
	if (mBuf != 0) {
		gjDrawActor(mBuf);
	} else {
		plain = gjGetMatrix();
		debugWireSphere((const f32*)plain, &gColorBlack);
	}
	shadowAlphaEnd();
	clearClips();
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
}

// 0x801B1958
int TAnimalRacer::answerKind()
{
	if (mMode == 0 || mMode == 3 || mMode == 4) {
		return 0x40;
	}
	return 0;
}

// 0x801B1984
int TAnimalRacer::answerRange(void* msg)
{
	if (mMode == 0 || mMode == 3 || mMode == 4) {
		((ObjValueMsg*)msg)->mValue = 20.0f;
		return 1;
	}
	return 0;
}

// 0x801B19B8
int TAnimalRacer::answerAim(void* msg)
{
	TObjAction* obj = (TObjAction*)findObjById(((ObjIdMsg*)msg)->mId);
	if (obj != 0) {
		Vec3f* at = obj->getPos();
		mAim      = (u16)angleToXZ(&mPos, at);
	} else {
		return 0;
	}
	if (eventStartByIds(mObjId, ((ObjIdMsg*)msg)->mId) == 0) {
		return 0;
	}
	mRef.setId(((ObjIdMsg*)msg)->mId);
	mStep = 1;
	return 1;
}

// 0x801B1A5C
void TAnimalRacer::retireShapes()
{
	struct Node {
		Node* mNext;
	};
	Node* node;

	for (node = (Node*)mFx->mContacts; node != 0; node = node->mNext) {
		colliContactNoBit10((ColliContact*)node);
	}
}

// 0x801B1AA0
void TAnimalRacer::setSpeed(f32 rate)
{
	mPlayer.mSpeed = rate;
	mRate          = rate;
}

// 0x801B1AAC
void TAnimalRacer::playMotion(u16 id, f32 rate, int)
{
	if (gRacerPack != 0 && mMotion != id) {
		mMotion = id;
		mPlayer.playBlend(mMotion, rate);
		applyMotion();
	}
}

// 0x801B1B08
void TAnimalRacer::applyMotion()
{
	if (gRacerPack != 0) {
		mPlayer.update();
		pose();
	}
}

// 0x801B1B4C
void TAnimalRacer::pose()
{
	if (mBuf == 0) {
		return;
	}
	gjPushUnitMatrix();
	if (mPlayer.isBlending() != 0) {
		gjMotionLinkActor(mBuf, (void*)mPlayer.getBlend(), mPlayer.mBlendTime);
	} else {
		int cur   = mPlayer.mCurrent;
		f32 frame = mPlayer.mFrame;
		gjMotionActor(frame, mBuf, mPlayer.getMotion(cur));
	}
	njPopMatrixEx();
}

// 0x801B1BE4
void TAnimalRacer::buildMtx()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mRotY);
	mtxRotateZ(mMtx, mMtx, mRotZ);
	mtxScaleV(mMtx, mMtx, &mScale);
}

// 0x801B1C48
void TAnimalRacer::fireClips()
{
	int i;

	if (mClips == 0) {
		return;
	}
	for (i = 0; i < mClipCount; i++) {
		RacerClip* clip = &mClips[i];
		if (clip->mMotion == mPlayer.mCurrent && mPlayer.mFrame > clip->mFrom && mPlayer.mFrame < clip->mTo) {
			aniTexSwapBegin((TexList*)mModel, clip->mA, clip->mB);
		}
	}
}

// 0x801B1D14
void TAnimalRacer::clearClips()
{
	aniTexSwapEnd();
}

// 0x801B1D34
void TAnimalRacer::probeShadow()
{
	EggHits hits;

	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mPos, 0xD) != 0 && (hits.mMask & 1) != 0) {
		shadowSetFromContact(&mShadow, (u8*)&hits);
	}
}

// 0x801B1DA8
void TAnimalRacer::setMode(int mode)
{
	if (mMode != -1) {
		runMode((int)mMode, 1);
	}
	mPrevMode = mMode;
	mMode     = -2;
	if (mode != -1) {
		runMode(mode, 0);
	}
	mMode = (s16)mode;
}

// 0x801B1E14
void TAnimalRacer::runMode(int mode, int phase)
{
	switch (mode) {
	case 0:
		modeWait(phase);
		break;
	case 1:
		modeRun(phase);
		break;
	case 2:
		modeJump(phase);
		break;
	case 3:
		modeDash(phase);
		break;
	case 4:
		modeGlide(phase);
		break;
	case 5:
		modeLand(phase);
		break;
	}
}

// 0x801B1EA8
void TAnimalRacer::modeWait(int phase)
{
	int turned;
	TObjAction* other;
	ObjMsg msg;

	if (phase == 0) {
		setSpeed(1.0f);
		playMotion(1, 0.075f);
		mFlags = 0x10000;
		mStep  = -1;
		colliShapeMakeCylinder((ColliShape*)&mShape, &gVecZero, (int)(5.0f * mScale.z), 5.0f * mScale.x, 5.0f * mScale.y);
		mShape.mFlags |= 0x200;
		if (mFx != 0) {
			colliCopyShapes(mFx, (const ColliShape*)&mShape);
			colliPlaceShapes(mFx, &mPos, (const ColliShape*)&mShape);
		}
	} else if (phase == 2) {
		retireShapes();
		switch (mStep) {
		case 0:
			break;
		case 1:
			turned = angleApproach(mRotY, mAim, 0x400);
			if (turned == mAim) {
				mMark = createSimpleTextPlain(getMessage((u16)aniRaceGetCourse()));
				showSpark();
				mStep = 2;
			}
			mRotY = turned;
			break;
		case 2:
			if (mMark != 0 && mMark->mState == 3) {
				mMark->mHold = 1;
				mMark        = 0;
				other        = mRef.get();
				if (other != 0) {
					msg.mType = 0x30;
					other->sendToHandle(&msg);
				}
				hideSpark();
				eventEnd();
				setKeyBit(600);
				setMode(1);
			}
			break;
		}
	}
}

// 0x801B205C
void TAnimalRacer::modeRun(int phase)
{
	f32 along;

	if (phase == 0) {
		playMotion(2, 0.075f);
		setSpeed(mInfo->mRate);
		mFlags = 0;
		mStep  = (s16)mInfo->mKind;
		if (gRacerKind == 9) {
			mStep = 3;
		}
		mShape.mFlags &= ~0x200;
		mShape.mFlags |= 8;
		if (mFx != 0) {
			colliCopyShapes(mFx, (const ColliShape*)&mShape);
			colliPlaceShapes(mFx, &mPos, (const ColliShape*)&mShape);
		}
	} else if (phase == 2) {
		retireShapes();
		switch (mStep) {
		case 3:
			chase();
			break;
		case 4:
			leap();
			break;
		case 5:
			fly();
			break;
		}
		if ((mFlags & 0x20) == 0) {
			along          = njScalor(&mVel);
			mPlayer.mSpeed = mRate + (1.0f - mRate) / mInfo->mBlend * along;
		}
	}
}

// 0x801B219C
void TAnimalRacer::modeJump(int phase)
{
	if (phase == 0) {
		setSpeed(1.0f);
		playMotion((u16)(*(u32*)mJump & 0xffff), 0.075f);
		aniRaceToStart();
		mSaved = mPos;
		mFlags = 0;
	} else if (phase == 2) {
		retireShapes();
		fall();
	} else if (phase == 1) {
		mFlags |= 4;
		mFlags &= ~1;
		mFlags &= ~2;
		aniRaceRun();
	}
}

// 0x801B2260
void TAnimalRacer::modeDash(int phase)
{
	int turned;
	TObjAction* other;
	ObjMsg msg;

	if (phase == 0) {
		if (mInfo->mKind == 4 && gRacerKind != 9) {
			playMotion(8, 0.075f);
			setSpeed(mInfo->mRate);
			mStep = 5;
		} else {
			setSpeed(1.0f);
			playMotion(1, 0.075f);
			mStep = -1;
		}
		aniRaceReset();
		mSaved = mPos;
		mFlags = 0;
		mShape.mFlags |= 0x200;
		mShape.mFlags &= ~8;
		if (mFx != 0) {
			colliCopyShapes(mFx, (const ColliShape*)&mShape);
			colliPlaceShapes(mFx, &mPos, (const ColliShape*)&mShape);
		}
	} else if (phase == 2) {
		retireShapes();
		switch (mStep) {
		case 5:
			fly();
			break;
		case 4:
			setSpeed(1.0f);
			playMotion(1, 0.075f);
			mStep = -1;
			break;
		case 1:
			turned = angleApproach(mRotY, mAim, 0x400);
			if (turned == mAim) {
				mMark = createSimpleTextPlain(getMessage((u16)aniRaceGetRank()));
				showSpark();
				mStep = 2;
			}
			mRotY = turned;
			break;
		case 2:
			if (mMark != 0 && mMark->mState == 3) {
				mMark->mHold = 1;
				mMark        = 0;
				other        = mRef.get();
				if (other != 0) {
					msg.mType = 0x30;
					other->sendToHandle(&msg);
				}
				hideSpark();
				eventEnd();
				aniRaceFinish();
				setMode(5);
			}
			break;
		}
	}
}

// 0x801B2494
void TAnimalRacer::modeGlide(int phase)
{
	int turned;
	TObjAction* other;
	ObjMsg msg;

	if (phase == 0) {
		if (mInfo->mKind == 4 && gRacerKind != 9) {
			playMotion(8, 0.075f);
			setSpeed(mInfo->mRate);
			mStep = 5;
		} else {
			setSpeed(1.0f);
			playMotion(1, 0.075f);
			mStep = -1;
		}
		aniRaceReset();
		mSaved = mPos;
		mFlags = 0;
		mShape.mFlags |= 0x200;
		mShape.mFlags &= ~8;
		if (mFx != 0) {
			colliCopyShapes(mFx, (const ColliShape*)&mShape);
			colliPlaceShapes(mFx, &mPos, (const ColliShape*)&mShape);
		}
	} else if (phase == 2) {
		retireShapes();
		switch (mStep) {
		case 5:
			fly();
			break;
		case 4:
			setSpeed(1.0f);
			playMotion(1, 0.075f);
			mStep = -1;
			break;
		case 1:
			turned = angleApproach(mRotY, mAim, 0x400);
			if (turned == mAim) {
				mMark = createSimpleTextPlain(getMessage((u16)aniRaceGetLaps()));
				showSpark();
				mStep = 2;
			}
			mRotY = turned;
			break;
		case 2:
			if (mMark != 0 && mMark->mState == 3) {
				mMark->mHold = 1;
				mMark        = 0;
				other        = mRef.get();
				if (other != 0) {
					msg.mType = 0x30;
					other->sendToHandle(&msg);
				}
				hideSpark();
				eventEnd();
				aniRaceFinish();
				setMode(5);
			}
			break;
		}
	}
}

// 0x801B26C8
void TAnimalRacer::modeLand(int phase)
{
	if (phase == 0) {
		setSpeed(1.0f);
		playMotion(1, 0.075f);
		mSaved = mPos;
		mFlags = 0;
		mStep  = -1;
		mShape.mFlags &= ~0x200;
		mShape.mFlags &= ~8;
		if (mFx != 0) {
			colliCopyShapes(mFx, (const ColliShape*)&mShape);
			colliPlaceShapes(mFx, &mPos, (const ColliShape*)&mShape);
		}
	} else if (phase == 2) {
		retireShapes();
	}
}

/** a minus b returned by value */
static inline Vec3f racerSub(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjSubVector(&v, a, b);
	return v;
}

/** a plus b returned by value */
static inline Vec3f racerAdd(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjAddVector(&v, a, b);
	return v;
}

// 0x801B2788
void TAnimalRacer::chase()
{
	Vec3f* goal;
	Vec3f at;
	Vec3f dest;
	f32 away;

	goal = (Vec3f*)aniRaceGetPos();
	at   = *goal;
	away = distXZ(&at, &mPos);
	if (away <= 10.0f) {
		dest = mPos;
		mFlags |= 0x40;
	} else {
		Vec3f dir = racerSub(&mPos, &at);
		dir.y     = 0.0f;
		if (njUnitVector(&dir) < gUnitVectorMin) {
			dir = gVecZero;
		}
		gjScaleVector(10.0f, &dir, &dir);
		dest = racerAdd(&at, &dir);
		mFlags &= 0x40;
	}
	mMover.mTarget = dest;
	mMover.mPos    = mPos;
	mMover.mVel    = mVel;
	mMover.mRot    = *(Rot3i*)&mRotX;
	updateMover(&mMover);
	if ((mMover.mFlags & 1) != 0) {
		mFlags |= 4;
	}
	if ((mMover.mFlags & 2) != 0) {
		mFlags |= 0x20;
	}
	if ((mMover.mFlags & 0x18) != 0) {
		aniRaceSetSpeed(10.0f);
	}
	mPos            = mMover.mPos;
	mVel            = mMover.mVel;
	*(Rot3i*)&mRotX = mMover.mRot;
	if ((mMover.mFlags & 0x10) != 0) {
		Vec3f launchVel;
		launchVel = mMover.mLaunch;
		launch(&launchVel, mInfo->mJump);
	}
}

// 0x801B29D4
void TAnimalRacer::leap()
{
	Vec3f goal;
	goal      = *(Vec3f*)aniRaceGetPos();
	Vec3f at  = goal;
	f32 speed = njScalor(&mVel);
	if (distXZ(&at, &mPos) <= 30.0f && speed <= 0.4f) {
		playMotion(8, 0.075f);
		setSpeed(mInfo->mRate);
		mStep = 5;
	} else {
		playMotion(9, 0.075f);
		setSpeed(mInfo->mRate);
		Vec3f dir = racerSub(&mPos, &at);
		dir.y     = 0.0f;
		if (njUnitVector(&dir) < gUnitVectorMin) {
			dir = gVecZero;
		}
		gjScaleVector(10.0f, &dir, &dir);
		Vec3f dest = racerAdd(&at, &dir);
		mFlags &= ~0x40;
		f32 y = dest.y;
		y += 15.0f;
		mMover.mTarget.set(dest.x, y, dest.z);
		mMover.mPos = mPos;
		mMover.mVel = mVel;
		mMover.mRot = *(Rot3i*)&mRotX;
		updateMover(&mMover);
		mPos            = mMover.mPos;
		mVel            = mMover.mVel;
		*(Rot3i*)&mRotX = mMover.mRot;
	}
}

// 0x801B2C28
void TAnimalRacer::fly()
{
	Vec3f ground;
	Vec3f goal;
	f32 speed;
	goal       = *(Vec3f*)aniRaceGetPos();
	Vec3f want = goal;
	Vec3f from = mPos;
	speed      = njScalor(&mVel);
	EggHits hits;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&want, 1) != 0) {
		ground = hits.mHits[0].mPos;
	}
	Vec3f dir = racerSub(&mPos, &ground);
	dir.y     = 0.0f;
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	gjScaleVector(10.0f, &dir, &dir);
	njAddVector(&ground, &dir);
	Vec3f step = racerSub(&ground, &from);
	speed += 0.05f;
	f32 cap = mInfo->mCap * njScalor(&step);
	if (speed > cap) {
		speed = cap;
	}
	if (speed > mInfo->mBlend) {
		speed = mInfo->mBlend;
	}
	mRotZ = angleApproach(mRotZ, 0, 0x80);
	Vec3f flat;
	flat.x = 0.0f;
	flat.y = 0.0f;
	flat.z = 1.0f;
	vecRotateY(&flat, &flat, mRotY);
	flat.y = -0.5f;
	if (njUnitVector(&flat) < gUnitVectorMin) {
		flat = gVecZero;
	}
	gjScaleVector(speed, &flat, &flat);
	Vec3f ahead = from;
	ahead.y += 10.0f;
	Vec3f next;
	{
		Vec3f sum;
		gjAddVector(&sum, &from, &flat);
		next = sum;
	}
	if (meshColliProbeLine((EggHit*)&hits, (const Vec3f*)&ahead, (const Vec3f*)&next, 0x2201) != 0) {
		flat.x = flat.z = 0.0f;
	}
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&ahead, 1) != 0) {
		f32 rise = hits.mHits[0].mPos.y;
		rise -= from.y;
		if (rise > flat.y) {
			flat.x = 0.0f;
			flat.y = 0.0f;
			flat.z = 0.0f;
			if (mRotZ == 0) {
				mFlags |= 0x40;
				mStep = 4;
			}
		}
	}
	mVel = flat;
	njAddVector(&mPos, &mVel);
}

// 0x801B2F5C
void TAnimalRacer::launch(const Vec3f* vel, void* jump)
{
	mVel  = *vel;
	mJump = jump;
	setMode(2);
}

// 0x801B2F9C
void TAnimalRacer::fall()
{
	if ((mFlags & 4) != 0 && mPlayer.isDone() != 0) {
		setMode((int)mPrevMode);
	}
	if (mPlayer.passedFrame((f32)((int*)mJump)[1]) != 0) {
		mFlags |= 1;
	} else if (mPlayer.passedFrame((f32)((int*)mJump)[2]) != 0 && (mFlags & 4) == 0) {
		mFlags |= 2;
	}
	if ((mFlags & 1) == 0) {
		return;
	}
	EggHits hits;
	Vec3f at = mPos;
	at.y += 10.0f;
	mFlags &= ~4;
	mVel.y += -0.08166667f;
	if (mVel.y < 0.0f && meshColliProbe((EggHit*)&hits, (const Vec3f*)&at, -1) != 0 && (hits.mMask & 1) != 0) {
		f32 ground = hits.mHits[0].mPos.y;
		if (!(ground - mPos.y < mVel.y)) {
			mVel.y = 0.0f;
			mPos.y = ground;
			njAddVector(&mPos, &mVel);
			mVel.z = mVel.y = mVel.x = 0.0f;
			mFlags |= 4;
			mFlags &= ~1;
			mFlags &= ~2;
			return;
		}
	}
	njAddVector(&mPos, &mVel);
}

// 0x801B318C
void TAnimalRacer::showSpark()
{
	if (mSpark != 0) {
		return;
	}
	Vec3f at = mPos;
	at.y += 20.0f * mScale.y;
	mSpark = (TObject*)createTCamConversation(0, 2, &at, 0xE38, 0x1555);
}

// 0x801B3208
void TAnimalRacer::hideSpark()
{
	if (mSpark == 0) {
		return;
	}
	mSpark->mFlags |= 1;
	mSpark = 0;
}

// 0x801B322C
int TAnimalRacer::onHit(const TObjAction*, int, int* result)
{
	resetHitInfo((HitInfo*)result);
	return 1;
}

// 0x801B3254
Vec3f* TAnimalRacer::getPos()
{
	return &mPos;
}

// 0x801B325C
int TAnimalRacer::getRotY()
{
	return mRotY;
}

// 0x801B3264
int TAnimalRacer::getKind()
{
	return 4;
}

// 0x801B326C
int TAnimalRacer::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x40:
		return answerKind();
	case 0x8A:
		return answerRange(msg);
	case 0x70:
		return answerAim(msg);
	}
	return 0;
}
