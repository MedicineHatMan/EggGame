#include "object/ObjAction.h"
struct DrawTask;
#include "io/Resource.h"
#include "camera/EventCamArc.h"
#include "stage/EventScript.h"
#include "stage/SetManager.h"
#include "object/ObjCommon.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "camera/TCamera.h"
#include "engine/Color.h"
#include "ui/AeArc.h"
#include "io/NinjaFile.h"
#include "stage/TEventCtrl.h"
#include "player/TPlyComb.h"
#include "object/ObjShadow.h"
#include "object/TObjLauncher.h"
#include "engine/ActorMotion.h"
#include "core/Mem.h"
#include "Ninja/ninja.h"
#include "engine/DrawState.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "stage/TGameState.h"
#include "object/TObjEggRing.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "stage/SetSlot.h"
#include "object/TObjPlayer.h"
#include "sound/Sound3DPlay.h"
#include "ui/MessageTable.h"
#include "engine/LightSet.h"
#include "ui/TSimpleText.h"
#include "object/mission/TObjMissionPrisoner.h"
struct NJS_TEXLIST;
struct SetLink;
struct CharaColli;
struct DrawState;
struct DrawStateSlot;
struct NJS_OBJECT;
struct GjActor;
struct ColliContact;
struct TSimpleText;

extern "C" {

// obj_ms_prisoner.arc and what it holds shared by every prisoner

}

enum PrisonerState {
	PRISONER_CAPTIVE = 0,
	PRISONER_EVENT   = 1, // the rescue event
	PRISONER_TALK    = 2, // shows a message
	PRISONER_HIT     = 3, // was hit while captive
	PRISONER_FREED   = 4, // looks for someone to meet
	PRISONER_WAIT    = 5, // stands by and can be talked to
	PRISONER_TURN    = 6, // turns to face the talker before talking
};

enum PrisonerPhase {
	PRISONER_ENTER  = 0,
	PRISONER_EXIT   = 1,
	PRISONER_UPDATE = 2,
	PRISONER_CHECK  = 3,
};

enum PrisonerFlags {
	PRISONER_FLAG_NOTIFIED = 1,
	PRISONER_FLAG_CAN_TALK = 2,
	PRISONER_FLAG_NO_BONDS = 8,
	PRISONER_FLAG_FAR      = 0x10,
};

// The link's data how far this prisoner got
enum PrisonerProgress {
	PROGRESS_CAPTIVE = 0,
	PROGRESS_FREED   = 1,
	PROGRESS_MET     = 2,
	PROGRESS_TALKED  = 3,
};

/** The event message that puts the rescuer somewhere near */
struct PrisonerPlaceMsg {
	u16 mType;  // _00
	u8 _02[2];
	Vec3f mPos; // _04
	int mRotY;  // _10
};

/** The talk message names the text to show */
struct PrisonerTalkMsg {
	u16 mType;  // _00
	u8 _02[2];
	s16 mMsgId; // _04
};

// Standing captive the event standing free and being hit
static const MotionEntry sPrisonerMotions[4] = {
	{ 0, 0, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ 1, 1, -1, 0, 0.0f, 113.0f, 1.0f, 0.0f, 0 },
	{ 0, 2, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ 1, 3, -1, 0, 0.0f, 66.0f, 1.0f, 0.0f, 0 },
};

/** What the motion walk writes while it poses the character */
struct PrisonerPose {
	int mNode;      // _00
	f32 mFace[12];  // _04 from node 0x38
	f32 mLeft[12];  // _34 from node 0x2D
	f32 mRight[12]; // _64 from node 0x23
	f32 mBody[12];  // _94 from node 0x3A
	f32 mBonds[12]; // _C4 from node 0x1B
};

struct PrisonerSearch {
	TObjAction* mFound; // _00
	ObjMsg mMsg;        // _04
};

struct PrisonerHitCheck {
	TObjAction* mSelf; // _00
	int mHit;          // _04
};

/** Where the rescuer stands in the event ahead of the prisoner */
struct PrisonerOffset : public Vec3f {
	PrisonerOffset(f32 ax, f32 ay, f32 az)
	{
		x = ax;
		y = ay;
		z = az;
	}
};

static PrisonerOffset sEventOffset(-10.0f, 0.0f, 0.0f);

typedef char TObjMissionPrisoner_size_check[(sizeof(TObjMissionPrisoner) == 0x264) ? 1 : -1];

// 0x8010A4FC
extern "C" void initMissionPrisoner()
{
	sMissionPrisonerArc = 0;
	sPrisonerModels = 0;
	sPrisonerMotionData = 0;
	sPrisonerTexLists = 0;
}

// 0x8010A514
extern "C" void termMissionPrisoner()
{
	if (sMissionPrisonerArc != 0) {
		releaseTextures(sPrisonerTexLists[0]);
		releaseResource(sMissionPrisonerArc);
		sMissionPrisonerArc = 0;
	}
	sPrisonerModels = 0;
	sPrisonerMotionData = 0;
	sPrisonerTexLists = 0;
}

// 0x8010A568
static void loadMissionPrisoner(int kind)
{
	playerEnsureArc(kind);
	playerLoadSoundBank(kind, 9);
	loadArc("obj_ms_prisoner.arc", &sMissionPrisonerArc);
	if (sMissionPrisonerArc != 0) {
		sPrisonerModels = (NJS_OBJECT**)findArcExport(sMissionPrisonerArc, "prs_models");
		sPrisonerMotionData = (NJS_MOTION**)findArcExport(sMissionPrisonerArc, "prs_motions");
		sPrisonerTexLists = (NJS_TEXLIST**)findArcExport(sMissionPrisonerArc, "prs_texlists");
		void** textures = (void**)findArcExport(sMissionPrisonerArc, "prs_textures");
		if (textures != 0) {
			bindGvmTextures((u8*)textures[0], sPrisonerTexLists[0]);
		}
		if (gPlayerArc != 0) {
			((TArc*)sMissionPrisonerArc)->linkFrom((TArc*)gPlayerArc);
		}
	}
}

// 0x8010A624
extern "C" int loadTObjMissionPrisoner(u8* kind)
{
	if (*kind >= 1 && *kind <= 4) {
		loadMissionPrisoner(*kind - 1);
	}
	return 1;
}

// 0x8010A660
extern "C" TObject* createTObjMissionPrisoner(TObject* parent, SetDesc* desc)
{
	return new TObjMissionPrisoner(parent, desc);
}

const char* gTObjMissionPrisonerName = "TObjMissionPrisoner";

// 0x8010A6B4
TObjMissionPrisoner::TObjMissionPrisoner(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00   = (u32)gTObjMissionPrisonerName;
	mSize = 0x264;
	mModels   = 0;
	mTexLists = 0;
	mKind     = 0;
	mState    = -1;
	mActor    = 0;
	setObjId(this, desc->mObjId);

	mColli = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)mObjId);
		colliSetMasks((CharaColli*)mColli, 4, 0xFF, 0xF6, 0xF2);
		colliShapeSetSphere(colliGetShape((CharaColli*)mColli, 0));
		colliGetShape((CharaColli*)mColli, 0)->mFlags = 0x600;
		colliGetShape((CharaColli*)mColli, 0)->mSizeX = 5.0f;
	}

	mSense = colliCreate(1, 1);
	if (mSense != 0) {
		colliSetOwner(mSense, (u16)mObjId);
		colliSetMasks((CharaColli*)mSense, 0x10, 0xFF, 0xFF, 0xFE);
		colliShapeSetSphere(colliGetShape((CharaColli*)mSense, 0));
		colliGetShape((CharaColli*)mSense, 0)->mFlags = 0x10;
		colliGetShape((CharaColli*)mSense, 0)->mSizeX = 20.0f;
	}

	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
	loadModels();
	mMotion.init(sPrisonerMotions, sPrisonerMotionData);
	mMotion.play(0);
	mShadow._30 = 10.0f;
	mShadow._34 = 10.0f;
	shadowEnable(&mShadow);
	mLightSet = lightSetFindArea(&mPos);

	switch ((int)mLink.get()->getData()) {
	case PROGRESS_CAPTIVE:
		setState(PRISONER_CAPTIVE);
		break;
	case PROGRESS_FREED:
		setState(PRISONER_FREED);
		break;
	case PROGRESS_MET:
	case PROGRESS_TALKED:
		mFlagBits.mBits |= PRISONER_FLAG_NO_BONDS;
		setState(PRISONER_WAIT);
		break;
	}
	updateMotion();
}

// 0x8010A940
TObjMissionPrisoner::~TObjMissionPrisoner()
{
	if (mModels != 0) {
		memFree(mBody);
		memFree(mFace);
		memFree(mLeft);
		memFree(mRight);
	}
	removeDrawTask(mDrawTask);
	colliFree((CharaColli*)mColli);
	colliFree((CharaColli*)mSense);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	unregisterId();
}

// 0x8010AA1C
void TObjMissionPrisoner::loadModels()
{
	if (mKind < 4 && sMissionPrisonerArc != 0) {
		TArc* arc = gPlayerArcs[mKind];
		if (arc == 0) {
			return;
		}
		mModels   = (void**)findArcExport(arc, "models");
		mTexLists = (void**)findArcExport(arc, "texlists");
		if (mModels != 0) {
			mBody  = createActor((NJS_OBJECT*)mModels[0]);
			mFace  = createActor((NJS_OBJECT*)mModels[2]);
			mLeft  = createActor((NJS_OBJECT*)mModels[3]);
			mRight = createActor((NJS_OBJECT*)mModels[4]);
		}
		pose();
		if (mModels != 0) {
			void* mem = objHeapAlloc(gObjHeap, sizeof(TPlyComb));
			mActor = new (mem) TPlyComb(this);
			mActor->build(mModels[1]);
			mActor->mTexture = mTexLists[0];
			mActor->setRoot(mBodyMatrix);
		}
	}
}

static void catchPoseNode(void*, void* arg);

// 0x8010AB44
void TObjMissionPrisoner::pose()
{
	if ((mFlagBits.mBits & PRISONER_FLAG_FAR) != 0) {
		return;
	}
	if (sMissionPrisonerArc == 0) {
		return;
	}
	if (mModels == 0) {
		return;
	}
	PrisonerPose pose;
	pose.mNode = 0;
	njPushMatrixEx();
	njUnitMatrix(0);
	int cur = mMotion.mCurrent;
	f32 frame = mMotion.mFrame;
	NJS_MOTION* motion = mMotion.getMotion(cur);
	calcActorMotion(mBody, motion, (NodeCallback)catchPoseNode, frame, (void*)&pose);
	mtxCopy(mFaceMatrix, pose.mFace);
	mtxCopy(mLeftMatrix, pose.mLeft);
	mtxCopy(mRightMatrix, pose.mRight);
	mtxConcat(mBodyMatrix, mMatrix, pose.mBody);
	mtxCopy(mBondsMatrix, pose.mBonds);
	if (mKind == 1) {
		mtxScale(1.5f, 1.5f, 1.5f, mBondsMatrix, mBondsMatrix);
	} else if (mKind == 3) {
		mtxScale(3.0f, 3.0f, 3.0f, mBondsMatrix, mBondsMatrix);
	}
	njSetMatrix(0, mFaceMatrix);
	gjBuildActorVertices(mFace);
	njSetMatrix(0, mLeftMatrix);
	gjBuildActorVertices(mLeft);
	njSetMatrix(0, mRightMatrix);
	gjBuildActorVertices(mRight);
	njPopMatrixEx();
}

// 0x8010ACA8
void TObjMissionPrisoner::updateMotion()
{
	mMotion.update();
	pose();
}

// 0x8010ACE0
void TObjMissionPrisoner::initFromDesc(SetDesc* desc)
{
	mKind    = desc->getInt(0);
	mEventId = desc->getInt(1);
	desc->getPos(&mPos);

	Vec3f pos;
	pos.x = mPos.x;
	pos.y = mPos.y + 5.0f;
	pos.z = mPos.z;
	if (mColli != 0) {
		colliShapeSetPos(colliGetShape(mColli, 0), &pos);
	}
	if (mSense != 0) {
		colliShapeSetPos(colliGetShape(mSense, 0), &pos);
	}

	SetRot rot;
	desc->getRot(&rot);
	mRotY = rot.y;
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRotY);
	shadowSetGround(&mShadow, &mPos, 0);
}

// 0x8010AE00
void TObjMissionPrisoner::debugPrint(void*)
{
	njPrint(0xA0013, ":KIND %d", mKind);
	njPrint(0xA0014, ":EVENT ", mEventId);
}

// 0x8010AE5C
void TObjMissionPrisoner::draw()
{
	if (debugEditActive() != 0) {
		pose();
		return;
	}
	if (!isInRange(this, getPos(), 500.0f)) {
		mFlagBits.mBits |= PRISONER_FLAG_FAR;
		if (mActor != 0) {
			mActor->mFlags |= 4;
		}
	} else {
		mFlagBits.mBits &= ~PRISONER_FLAG_FAR;
		if (mActor != 0) {
			mActor->mFlags &= ~4;
		}
	}
	runState(mState, PRISONER_CHECK);
	runState(mState, PRISONER_UPDATE);
}

// 0x8010AF3C
void TObjMissionPrisoner::vfn_10()
{
	if (cameraIsVisible(1000.0f, 10.0f, &mPos, 0) == 0) {
		mDrawTask->mFlags |= 1;
	} else {
		mDrawTask->mFlags &= ~1;
	}
}

// 0x8010AFA4
void TObjMissionPrisoner::render()
{
	f32 mtx[12];
	mtxConcat(mtx, (const f32*)gjGetMatrix(), mMatrix);
	if (mModels == 0) {
		mtxScale(5.0f, 5.0f, 5.0f, mtx, mtx);
		debugWireSphere(mtx, &gColorRed);
		return;
	}
	njSetTexture(mTexLists[0]);
	njPushMatrix(mtx);
	DrawStateSlot* slot = getDrawStateSlot();
	lightSetSelect(mLightSet);
	shadowAlphaBegin();
	beginDrawState(slot, getDefaultDrawState());
	beginActorDraw();
	drawActor(mBody);
	drawActor(mFace);
	drawActor(mLeft);
	drawActor(mRight);
	endActorDraw();
	if (sMissionPrisonerArc != 0 && (mFlagBits.mBits & PRISONER_FLAG_NO_BONDS) == 0) {
		njMultiMatrix(0, mBondsMatrix);
		njSetTexture(sPrisonerTexLists[0]);
		gjDrawObject(sPrisonerModels[0]);
	}
	endDrawState(slot);
	njPopMatrixEx();
	mActor->drawModel(getDefaultDrawState());
	shadowAlphaEnd();
}

// 0x8010B0D4
void TObjMissionPrisoner::setState(int state)
{
	if (mState != -1) {
		runState(mState, PRISONER_EXIT);
	}
	if (state != -1) {
		runState(state, PRISONER_ENTER);
	}
	mState = state;
}

// 0x8010B134
void TObjMissionPrisoner::runState(int state, int phase)
{
	switch (state) {
	case PRISONER_CAPTIVE:
		stateCaptive(phase);
		break;
	case PRISONER_EVENT:
		stateEvent(phase);
		break;
	case PRISONER_FREED:
		stateFreed(phase);
		break;
	case PRISONER_WAIT:
		stateWait(phase);
		break;
	case PRISONER_TALK:
		stateTalk(phase);
		break;
	case PRISONER_HIT:
		stateHit(phase);
		break;
	case PRISONER_TURN:
		stateTurn(phase);
		break;
	}
}

static void checkHitContact(void* contact, void* arg, int* out);

// 0x8010B1C0
void TObjMissionPrisoner::stateCaptive(int phase)
{
	if (phase == PRISONER_ENTER) {
		mMotion.play(0);
	} else if (phase == PRISONER_CHECK) {
		PrisonerHitCheck check;
		check.mSelf = this;
		check.mHit  = 0;
		forEachContact((CharaColli*)mColli, (ContactCallback)checkHitContact, &check);
		if (check.mHit != 0) {
			setState(PRISONER_HIT);
		}
	} else if (phase == PRISONER_UPDATE) {
		updateMotion();
	}
}

// 0x8010B250
// The event system calls this once this prisoner's event can run
int TObjMissionPrisoner::startEvent(u32 id)
{
	TObjMissionPrisoner* self = (TObjMissionPrisoner*)findObjById(id);
	if (self != 0) {
		self->setState(PRISONER_EVENT);
	}
	return 1;
}

// 0x8010B288
void TObjMissionPrisoner::stateEvent(int phase)
{
	if (phase == PRISONER_ENTER) {
		mTimer = 0;
		mEvent = eventCamArcPlay(1.0f, 0.0f, 7);
		mEvent->attachToMtx();
		mEvent->clearAutoDelete();

		f32 mtx[12];
		Vec3f out;
		mtxSetTransV(mtx, &mPos);
		mtxRotateY(mtx, mtx, mRotY);
		mtxMultVec(mtx, &sEventOffset, &out);
		mtxSetTransV(mEventMatrix, &out);
		mtxRotateY(mEventMatrix, mEventMatrix, mRotY);

		TObjAction* rescuer = mRescuer.get();
		if (rescuer != 0) {
			eventHideObj((u8*)rescuer);
			PrisonerPlaceMsg msg;
			msg.mType = 0x7E;
			msg.mPos  = out;
			msg.mRotY = mRotY;
			rescuer->sendToHandle((ObjMsg*)&msg);
		}
		mMotion.play(1);
		sePlay3DGroup(0xDA, &mPos, 9);
	} else if (phase == PRISONER_EXIT) {
		if (mEvent != 0) {
			mEvent->mFlags |= 1;
		}
		eventEnd();
	} else if (phase == PRISONER_CHECK) {
		if (mTimer == 180) {
			TObjAction* rescuer = mRescuer.get();
			if (rescuer != 0) {
				ObjMsg msg;
				msg.mType = 0x79;
				rescuer->sendToHandle(&msg);
			}
			setState(PRISONER_WAIT);
		}
	} else if (phase == PRISONER_UPDATE) {
		mTimer++;
		mtxCopy(mEvent->getMtx(), mEventMatrix);
		updateMotion();
	}
}

// 0x8010B448
// Asks each thing near a freed prisoner whether it is a rescuer
static void findRescuer(void* contact, void* arg, int* out)
{
	PrisonerSearch* search = (PrisonerSearch*)arg;
	if ((colliContactShape((ColliContact*)contact)->mFlags & 0x10) != 0) {
		return;
	}
	TObjAction* other = findObjById(colliContactOwner((ColliContact*)contact));
	if (other != 0 && other->sendToHandle(&search->mMsg) != 0) {
		search->mFound = other;
		*out           = 0;
	}
}

// 0x8010B4C8
void TObjMissionPrisoner::stateFreed(int phase)
{
	if (phase == PRISONER_ENTER) {
		mMotion.play(0);
	} else if (phase == PRISONER_CHECK) {
		PrisonerSearch search;
		search.mMsg.mType = 0x81;
		search.mFound     = 0;
		forEachContact((CharaColli*)mSense, (ContactCallback)findRescuer, &search);
		if (search.mFound != 0) {
			mRescuer.setId(search.mFound->mObjId);
			ObjMsg msg;
			msg.mType = 0x77;
			search.mFound->sendToHandle(&msg);
			mLink.get()->setData((void*)PROGRESS_MET);
			mFlagBits.mBits |= PRISONER_FLAG_NO_BONDS;
			setState(-1);
			eventStartById(mObjId, (int (*)(int))startEvent);
		}
	} else if (phase == PRISONER_UPDATE) {
		updateMotion();
	}
}

// 0x8010B5B0
void TObjMissionPrisoner::stateWait(int phase)
{
	if (phase == PRISONER_ENTER) {
		mFlagBits.mBits |= PRISONER_FLAG_CAN_TALK;
		mMotion.play(2);
	} else if (phase == PRISONER_UPDATE) {
		updateMotion();
	}
}

// 0x8010B600
void TObjMissionPrisoner::stateTalk(int phase)
{
	if (phase == PRISONER_ENTER) {
		mWindow = (int*)createSimpleTextPlain(getMessage(mMsgId));
		Vec3f pos;
		pos.x = mPos.x;
		pos.y = mPos.y + 5.0f;
		pos.z = mPos.z;
		mCamera = createTCamConversation(0, 2, &pos, -0xAAA, 0x1555);
		mMotion.play(2);
	} else if (phase == PRISONER_EXIT) {
		if (mWindow != 0) {
			mWindow[0x58 / 4] = 1;
		}
		if (mCamera != 0) {
			mCamera->mFlags |= 1;
		}
		eventScriptSignal();
	} else if (phase == PRISONER_CHECK) {
		if (mWindow != 0 && mWindow[0x24 / 4] == 3) {
			TObjAction* rescuer = mRescuer.get();
			if (rescuer != 0) {
				ObjMsg msg;
				msg.mType = 0x30;
				rescuer->sendToHandle(&msg);
			}
			setState(PRISONER_WAIT);
			if ((u32)mLink.get()->getData() == PROGRESS_MET) {
				ObjMsg notice;
				notice.mType = 0xAA;
				sendGameMsg(&notice);
				mLink.get()->setData((void*)PROGRESS_TALKED);
			}
		}
	} else if (phase == PRISONER_UPDATE) {
		updateMotion();
	}
}

// 0x8010B75C
void TObjMissionPrisoner::stateHit(int phase)
{
	if (phase == PRISONER_ENTER) {
		mMotion.play(3);
	} else if (phase == PRISONER_CHECK) {
		if ((mFlagBits.mBits & PRISONER_FLAG_NOTIFIED) == 0 && mMotion.isDone() != 0) {
			mFlagBits.mBits |= PRISONER_FLAG_NOTIFIED;
			ObjMsg notice;
			notice.mType = 0xA6;
			sendGameMsg(&notice);
		}
	} else if (phase == PRISONER_UPDATE) {
		updateMotion();
	}
}

// 0x8010B7EC
static int turnTowards(f32 rate, int from, int to)
{
	f32 step = rate * (1.0f / 60.0f);
	if (step > 1.0f) {
		step = 1.0f;
	}
	return (int)(step * angleDiff(from, to) + from);
}

// 0x8010B884
void TObjMissionPrisoner::stateTurn(int phase)
{
	if (phase == PRISONER_ENTER) {
		mTimer = 0;
		eventHideObj((u8*)mRescuer.get());
	} else if (phase == PRISONER_CHECK) {
		if (mTimer == 30) {
			setState(PRISONER_TALK);
		}
	} else if (phase == PRISONER_UPDATE) {
		mRotY = turnTowards(10.0f, mRotY, mTargetRotY);
		mtxSetTransV(mMatrix, &mPos);
		mtxRotateY(mMatrix, mMatrix, mRotY);
		updateMotion();
		mTimer++;
	}
}

// 0x8010B938
int TObjMissionPrisoner::onTalk(ObjMsg* msg)
{
	mMsgId = ((PrisonerTalkMsg*)msg)->mMsgId;
	setState(PRISONER_TURN);
	return 1;
}

// 0x8010B968
int TObjMissionPrisoner::onGetFlags()
{
	int flags = 0;
	if ((mFlagBits.mBits & PRISONER_FLAG_CAN_TALK) != 0) {
		flags |= 0x40;
	}
	return flags;
}

// 0x8010B980
int TObjMissionPrisoner::onMeet(ObjMsg* msg)
{
	switch ((u8)mState) {
	case PRISONER_WAIT:
		break;
	default:
		return 0;
	}
	Vec3f target;
	target = *findObjById(((ObjIdMsg*)msg)->mId)->getPos();
	mTargetRotY = angleToXZ(&mPos, &target);
	mRescuer.setId(((ObjIdMsg*)msg)->mId);
	return eventScriptRequest(mEventId, (u16)mObjId);
}

// 0x8010BA18
int TObjMissionPrisoner::onFree()
{
	if (mState == PRISONER_CAPTIVE) {
		mLink.get()->setData((void*)PROGRESS_FREED);
		setState(PRISONER_FREED);
		return 1;
	}
	return 0;
}

// 0x8010BA70
// Called at each node of the skeleton
static void catchPoseNode(void*, void* arg)
{
	PrisonerPose* pose = (PrisonerPose*)arg;
	switch (pose->mNode) {
	case 0x1B:
		njGetMatrix(pose->mBonds);
		break;
	case 0x23:
		njGetMatrix(pose->mRight);
		break;
	case 0x2D:
		njGetMatrix(pose->mLeft);
		break;
	case 0x38:
		njGetMatrix(pose->mFace);
		break;
	case 0x3A:
		njGetMatrix(pose->mBody);
		break;
	}
	pose->mNode++;
}

// 0x8010BB04
// Something touching a captive prisoner looks for a contact that hurts
static void checkHitContact(void* contact, void* arg, int* out)
{
	PrisonerHitCheck* check = (PrisonerHitCheck*)arg;
	if ((colliContactShape((ColliContact*)contact)->mFlags & 8) == 0) {
		return;
	}
	TObjAction* other = findObjById(colliContactOwner((ColliContact*)contact));
	if (other == 0) {
		return;
	}
	HitInfo info;
	if (other->onHit(check->mSelf, colliContactTag((ColliContact*)contact), (int*)&info) != 0 && info.mFlags.testBit(0xE) != 0) {
		check->mHit = 1;
		*out        = 1;
	}
}

// 0x8010BBB4
Vec3f* TObjMissionPrisoner::getPos()
{
	return &mPos;
}

// 0x8010BBBC
int TObjMissionPrisoner::getKind()
{
	return 3;
}

// 0x8010BBC4
SetLink* TObjMissionPrisoner::getLink()
{
	return mLink.get();
}

// 0x8010BBCC
int TObjMissionPrisoner::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x1003:
		return onTalk(msg);
	case 0x40:
		return onGetFlags();
	case 0x70:
		return onMeet(msg);
	case 0x1000:
		return onFree();
	}
	return 0;
}
