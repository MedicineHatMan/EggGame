#include "camera/EventCamRef.h"
#include "stage/EventScript.h"
#include "object/ObjAction.h"
#include "ui/TSimpleText.h"
#include "stage/SetManager.h"
#include "egg/EggCreature.h"
#include "ui/MessageTable.h"
#include "engine/TParticle.h"
#include "stage/TEventCtrl.h"
#include "core/TMainTask.h"

int gEventCamId;     // 0x802C5638 the id of the camera handed out last
int gEventCamActive; // 0x802C563C 1 while that camera is out

/**
 * "wait": counts mFrames down, then lets the script go on. The name is ours.
 *
 * @size 0x24
 */
class TEventWait : public TObject {
public:
	TEventWait(TObject* parent, int frames)
	    : TObject(parent)
	{
		mFrames = frames;
	}
	virtual void draw() // _0C 0x8012D3F4
	{
		mFrames--;
		if (mFrames <= 0) {
			eventScriptSignal();
			mFlags |= 1;
		}
	}

	virtual ~TEventWait() { } // 0x8012D444

	int mFrames; // _20
};

/**
 * "msg": shows a message and lets the script go on once it has closed. Only one is shown at a time. The name is
 * ours.
 *
 * @size 0x24
 */
class TEventMsg : public TObject {
public:
	inline TEventMsg(TObject* parent, u16 id);
	virtual ~TEventMsg(); // 0x8012D30C

	virtual void draw(); // _0C 0x8012D394

	TSimpleText* mText; // _20
};

TEventMsg* gEventMsg; // 0x802C5640

// 0x8012CF74
EventCamRef::EventCamRef()
{
	mActive = 0;
	mId     = 0;
}

// 0x8012CF84
EventCamRef::~EventCamRef()
{
	release();
}

// 0x8012CFCC
int EventCamRef::isActive()
{
	return mActive != 0;
}

// 0x8012CFE0
void EventCamRef::set(ObjMsg* msg)
{
	mActive = 1;
	mId     = ((EventCamMsg*)msg)->mId;
}

// 0x8012CFF4
void EventCamRef::release()
{
	if (isActive()) {
		eventCamFree(mId);
	}
	mActive = 0;
	mId     = 0;
}

// 0x8012D03C
void eventCamInit()
{
	gEventCamId     = 100;
	gEventCamActive = 0;
}

// 0x8012D050
void eventCamExit()
{
}

// 0x8012D054
void eventCamReset()
{
	eventCamInit();
}

// 0x8012D074
void eventCamFree(int id)
{
	if (gEventCamActive == 1 && gEventCamId == id) {
		gEventCamActive = 0;
		eventScriptSignal();
	}
}

// 0x8012D0B4
int eventCmdSet(EventCamSetArgs* args)
{
	if (gEventCamActive != 0) {
		return 0;
	}
	EventCamMsg msg;
	msg.mType = 199;
	int taken = 0;
	gEventCamId++;
	u32 id = spawnSetById(args->mSetId);
	eventHideId(id);
	TObjAction* obj = findObjById((u16)id);
	if (obj != 0) {
		msg.mId  = gEventCamId;
		msg.mArg = args->mArg;
		taken    = obj->sendToHandle(&msg);
		if (taken != 0) {
			gEventCamActive = 1;
		}
	}
	if (taken == 0) {
		eventScriptDone();
	}
	return 1;
}

// 0x8012D16C
int eventCmdWait(EventWaitArgs* args)
{
	new TEventWait(gTL_02, args->mFrames);
	return 1;
}

// 0x8012D1C8
int eventCmdWaitFake(void*)
{
	eventScriptDone();
	return 1;
}

// 0x8012D1EC
int eventCmdPtclTest(void*)
{
	TObjAction* obj = findObjById(0);
	if (obj != 0) {
		Vec3f pos;
		pos = *obj->getPos();
		particleOnceByIdDefault(&pos, 0, 7);
	}
	return 1;
}

inline TEventMsg::TEventMsg(TObject* parent, u16 id)
    : TObject(parent)
{
	mText             = 0;
	TSimpleText* text = createSimpleTextPlain(getMessage(id));
	mText             = text;
	if (text == 0) {
		eventScriptSignal();
		mFlags |= 1;
	}
}

// 0x8012D254
int eventCmdMsg(EventMsgArgs* args)
{
	TObject* parent;
	u16 id;
	TEventMsg* msg;
	id     = args->mId;
	parent = gTL_02;
	if (gEventMsg != 0) {
		msg = 0;
	} else {
		msg       = new TEventMsg(parent, id);
		gEventMsg = msg;
	}
	if (msg != 0) {
		return 1;
	}
	return 0;
}

// 0x8012D30C
TEventMsg::~TEventMsg()
{
	if (mText != 0) {
		mText->mHold = 1;
	}
	if (this == gEventMsg) {
		gEventMsg = 0;
	}
}

// 0x8012D394
void TEventMsg::draw()
{
	if (mText != 0 && mText->mState == 3) {
		mText->mHold = 1;
		mText        = 0;
		eventScriptSignal();
		mFlags |= 1;
	}
}
