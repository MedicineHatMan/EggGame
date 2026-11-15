#include "engine/MotionPlayer.h"
struct DrawTask;
#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "camera/EventCamArc.h"
#include "stage/SetActor.h"
#include "object/ObjAction.h"
#include "object/ObjModel.h"
#include "object/ObjArchive.h"
#include "object/ObjCommon.h"
#include "engine/CharaColli.h"
#include "ui/TSimpleText.h"
#include "engine/Fx.h"
#include "ui/MessageTable.h"
#include "stage/TEventCtrl.h"
#include "engine/Color.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/TObjEggRing.h"
#include "engine/FxSystem.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "sound/SoundCtrl.h"
#include "stage/SetSlot.h"
#include "object/TObjDivineVoice.h"
struct CharaColli;
struct SetLink;

const char* gTObjDivineVoiceName = "TObjDivineVoice";

const char* sDivineVoiceShapes[3] = { "SPHERE", "CUBE", "CYLINDER" };
const char* sDivineVoiceParam0[3] = { ":RAD", ":WIDTH", ":RAD" };
const char* sDivineVoiceParam1[3] = { "", ":HEIGHT", ":HEIGHT" };
const char* sDivineVoiceParam2[3] = { "", ":DEPTH", "" };

extern "C" {
extern const MotionEntry sDivineVoiceMotions[] = {
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 317.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 11, -1, 0, 120.0f, 317.0f, 1.0f, 0.0f, 0 },
};
}

enum {
	VOICE_WAIT  = 0, // for someone to walk in
	VOICE_TALK  = 1, // while the text is up
	VOICE_CLOSE = 2, // fading out
};

typedef char TObjDivineVoice_size_check[(sizeof(TObjDivineVoice) == 0x1A0) ? 1 : -1];

// 0x801216E4
extern "C" TObject* createTObjDivineVoice(TObject* parent, SetDesc* desc)
{
	return new TObjDivineVoice(parent, desc);
}

// 0x80121738
TObjDivineVoice::TObjDivineVoice(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjDivineVoiceName;
	mSize = 0x1A0;

	mModelData  = getStgObjModels()[46];
	mModelData2 = getStgObjModels()[47];
	mTexList    = getStgObjTextures()[37];

	mMotion.init((const MotionEntry*)sDivineVoiceMotions, gStgObjCommon->mSet.mMotions);
	mMotion.play(0);

	mState.mFlags    = DRAWSTATE_BLEND | DRAWSTATE_AMBIENT;
	mState.mBlend    = 1;
	mState.mAmbient  = 0x000000FF; // black alpha 255
	mState2.mFlags   = DRAWSTATE_AMBIENT;
	mState2.mAmbient = 0;

	mModel.setModel(mModelData);
	mModel.mState  = &mState;
	mModel.mMotion = &mMotion;
	mModel2.setModel(mModelData2);
	mModel2.mState = &mState2;

	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 255, 254, 254);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	initFromDesc(desc);
	reset();
}

// 0x801218D8
TObjDivineVoice::~TObjDivineVoice()
{
	if (mText != 0) {
		mText->mHold = 1;
	}
	removeDrawTask(mDrawTask);
	colliRetire(mShape);
	mShape = 0;
}

// 0x80121988
// Int 0 is the message int 1 the shape and int 2 the message-only flag
void TObjDivineVoice::initFromDesc(SetDesc* desc)
{
	ColliShape shape;
	f32 a;
	f32 b;
	f32 c;

	TSetActor::initFromDesc(desc);
	mMsgId     = desc->getInt(0) < 0 ? 0 : desc->getInt(0) > getMessageCount() - 1 ? getMessageCount() - 1 : desc->getInt(0);
	mShapeKind = desc->getInt(1) < 0 ? 0 : desc->getInt(1) > 2 ? 2 : desc->getInt(1);
	mMsgOnly   = desc->getInt(2) < 0 ? 0 : desc->getInt(2) > 1 ? 1 : desc->getInt(2);
	a          = desc->getFloat(0);
	b          = desc->getFloat(1);
	c          = desc->getFloat(2);
	switch (mShapeKind) {
	case 0:
		colliShapeMakeSphere(&shape, &mPos, 0, a);
		break;
	case 1:
		colliShapeMakeBox(&shape, (const Vec3f*)&mPos, mRot.y, a, b, c, 0);
		break;
	case 2:
		colliShapeMakeCylinder(&shape, &mPos, 0, a, b);
		break;
	}
	shape.mFlags |= 0x10;
	colliCopyShapes(mShape, &shape);
}

// 0x80121BFC
// The lines are the set-file labels
void TObjDivineVoice::debugPrint(void* arg)
{
	njPrintC(0xA0013, ":MSG ID");
	njPrint(0xA0014, ":TYPE %s", sDivineVoiceShapes[mShapeKind]);
	njPrintC(0xA0015, ":MSG ONLY");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, sDivineVoiceParam0[mShapeKind]);
	njPrintC(0xA0018, sDivineVoiceParam1[mShapeKind]);
	njPrintC(0xA0019, sDivineVoiceParam2[mShapeKind]);
	njPrintC(0xA001A, "");
}

// 0x80121CEC
void TObjDivineVoice::reset()
{
	mRef.setId(0xFFFF);
	mCamera = 0;
	mText   = 0;
	mSound  = -1;
	setLinkFree((SetLink*)&mLink);
	mMode = VOICE_WAIT;
}

// 0x80121D48
void TObjDivineVoice::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (isInRange(this, &mPos, 900.0f) == 0) {
		mFlags |= 1;
		return;
	}
	switch (mMode) {
	case VOICE_WAIT:
		wait();
		break;
	case VOICE_TALK:
		talk();
		break;
	case VOICE_CLOSE:
		close();
		break;
	}
}

// Message 0x87 hold still for the voice
struct VoiceMsg {
	u16 mType; // _00
	u16 _02;
	u16 mFrom; // _04
};

// 0x80121DEC
void TObjDivineVoice::wait()
{
	TObjAction* obj;
	ColliContact* contact;
	f32 mtx[12];

	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactNoBit10(contact) == 0) {
			continue;
		}
		obj = findObjById(colliContactOwner(contact));
		if (obj == 0 || obj->getKind() != 0) {
			continue;
		}
		mRef.setId(obj->mObjId);
		mAt     = *obj->getPos();
		mAtRotY = obj->getRotY();
		if (mMsgOnly == 0) {
			VoiceMsg msg;
			msg.mType = 0x87;
			msg.mFrom = mObjId;
			if (obj->sendToHandle((ObjMsg*)&msg) == 0) {
				continue;
			}
			mCamera = eventCamArcPlay(1.0f, 0.0f, 2);
			if (mCamera == 0) {
				return;
			}
			((TCamera*)mCamera)->setCut();
			mCamera->attachToMtx();
			mtxSetTransV(mMtx, &mAt);
			mtxRotateY(mMtx, mMtx, mAtRotY);
			mtxCopy(mCamera->getMtx(), mMtx);
			mtxTranslate(mtx, mMtx, 0.0f, 40.0f, 0.0f);
			mtxRotateX(mtx, mtx, 0x4000);
			mFx    = fxSpawnAt(1, 7, 3, mtx);
			u16 id = mObjId;
			eventStartByIds(id, mRef.getId());
		} else {
			eventStartById(mObjId, 0);
		}
		mText = createSimpleTextPlain(getMessage(mMsgId));
		if (mText == 0) {
			return;
		}
		mMode = VOICE_TALK;
	}
}

// 0x80121FDC
void TObjDivineVoice::talk()
{
	int alpha;

	if (mText != 0 && mText->mState == 3) {
		mText->mHold = 1;
		mText        = 0;
		mMode        = VOICE_CLOSE;
		return;
	}
	if (mMsgOnly == 0) {
		if (mSound == 0xFFFFFFFF) {
			mSound = sePlayFor(0x2B2, 0);
		}
		alpha = (mState2.mAmbient & 0xFF) + 8;
		if ((s16)alpha > 0xFF) {
			alpha = 0xFF;
		}
		setColorAlpha(&mState2.mAmbient, alpha & 0xFF);
		mMotion.update();
		if (mMotion.isDone()) {
			mMotion.play(1);
		}
	}
}

// 0x801220B0
void TObjDivineVoice::close()
{
	int alpha;

	if (mMsgOnly != 0) {
		eventEnd();
		setLinkForever((SetLink*)&mLink);
		mFlags |= 1;
	} else {
		alpha = (mState2.mAmbient & 0xFF) - 8;
		if ((s16)alpha < 0) {
			alpha = 0;
			seStop(mSound);
			eventEnd();
			if (mCamera != 0) {
				mCamera->mFlags |= 1;
				mCamera = 0;
			}
			fxRemove(mFx);
			if (mRef.get() != 0) {
				ObjMsg msg;
				msg.mType = 0x79;
				mRef.get()->sendToHandle(&msg);
			}
			setLinkForever((SetLink*)&mLink);
			mFlags |= 1;
		}
		setColorAlpha(&mState.mAmbient, (u32)alpha & 0xFF);
		setColorAlpha(&mState2.mAmbient, alpha & 0xFF);
		mMotion.update();
	}
}

// 0x801221B4
void TObjDivineVoice::vfn_10()
{
}

// 0x801221B8
void TObjDivineVoice::render()
{
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
	}
	if (mMsgOnly != 0 || mMode == VOICE_WAIT) {
		return;
	}
	njSetTexture(mTexList);
	njPushMatrixEx();
	njMultiMatrix(0, mMtx);
	mModel2.draw();
	njPopMatrixEx();
}
