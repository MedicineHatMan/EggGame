#include "stage/EventQueue.h"
#include "enemy/boss/EneBoss.h"
#include "stage/EventScript.h"
#include "stage/SetManager.h"
#include "sound/SoundCtrl.h"
#include "core/Vibration.h"
#include "core/TMainTask.h"
#include "object/TObject.h"
#include "stage/TEventCtrl.h"
#include "core/Mem.h"
#include "stage/SetSpawner.h"
struct TObject;
struct TMainTask;

extern "C" {
extern int (*sEventStartCb)(int); // called when an event by number starts

// 0x8006A3C4
extern "C" void eventRequestClear(EventRequest* request)
{
	for (int i = 0; i < 4; i++) {
		request->mIds[i] = 0xFFFF;
	}
	request->mObj = 0;
}
}

typedef char TEventCtrl_size_check[(sizeof(TEventCtrl) == 0x38) ? 1 : -1];

const char* gTEventCtrlName = "TEventCtrl";

extern "C" void clearFlagByte(void*);

// 0x8006A3F0
TEventCtrl::TEventCtrl(TObject* parent)
    : TObject(parent)
{
	_00    = (u32)gTEventCtrlName;
	mSize  = 0x38;
	mState = 0;
	clearFlagByte(&mFlag);
	_24           = 1;
	mCamera       = 0;
	mObjParent    = 0;
	_30           = 0;
	mDesignParent = 0;
}

// 0x8006A470
void TEventCtrl::reset()
{
	if ((s8)mState == 1 && (mFlag.mBits & 2)) {
		end();
	}
	mState = 0;
	clearFlagByte(&mFlag);
	_24     = 1;
	mCamera = 0;
}

// 0x8006A4D4
int TEventCtrl::isFree()
{
	return !(mFlag.mBits & 8);
}

// 0x8006A4E8
void TEventCtrl::start()
{
	mFlag.mBits &= 0xFE;
	mState = 1;
	objPushLayers(&gMainTask);
	mCamera       = setSpawnerCamera();
	mObjParent    = setSetObjParent(gTL_05);
	mDesignParent = setSetDesignParent(gTL_05);
	sePauseVoices();
	vibPauseAll();

	if (mFlag.mBits & 4) {
		if (gEventCurrent.mStart != 0) {
			if (gEventCurrent.mStart(gEventCurrent.mArg, gEventCurrent.mId) == 0) {
				end();
			}
		} else {
			end();
		}
		return;
	}

	for (int i = 0; i < 4; i++) {
		if (sEventReq.mIds[i] == 0xFFFF) {
			break;
		}
		eventHideId(sEventReq.mIds[i]);
	}
	if (sEventReq.mObj != 0) {
		eventHideObj((u8*)sEventReq.mObj);
	}
	if (sEventStartCb != 0) {
		if (sEventStartCb(sEventReq.mIds[0]) == 0) {
			end();
		}
	}
}

// 0x8006A620
void TEventCtrl::end()
{
	objPopLayers(&gMainTask);
	mState = 0;
	mFlag.mBits &= ~2;
	vibResumeAll();
	seResumeVoices();
	setSpawnerMode((int)mCamera);
	setSetObjParent(mObjParent);
	setSetDesignParent(mDesignParent);
	sEventStartCb = 0;
}

// 0x8006A690
void TEventCtrl::draw()
{
	if ((int)mState == 1 && (mFlag.mBits & 2)) {
		end();
	}
	if (canStart() && gEventQueue != 0) {
		if (gEventQueue->pop(&gEventCurrent)) {
			request();
			mFlag.mBits |= 4;
		}
	}
	eventScriptUpdate();
	if ((s8)mState == 0 && (mFlag.mBits & 1)) {
		start();
	}
}

// 0x8006A73C
void TEventCtrl::request()
{
	mFlag.mBits |= 1;
	mFlag.mBits &= ~2;
	mFlag.mBits &= ~4;
}

// 0x8006A764
void TEventCtrl::requestEnd()
{
	mFlag.mBits |= 2;
	mFlag.mBits &= 0xFE;
	mFlag.mBits &= ~4;
}

// 0x8006A78C
int TEventCtrl::isBusy()
{
	u8 busy = 0;
	if ((s8)mState != 0 || (mFlag.mBits & 1)) {
		busy = 1;
	}
	return busy;
}

// 0x8006A7B4
int TEventCtrl::canStart()
{
	u8 can = 0;
	if (!isBusy() && isFree()) {
		can = 1;
	}
	return can;
}

// 0x8006A804
extern "C" void eventInit()
{
	sEventCtrlObj = 0;
	eventRequestClear(&sEventReq);
	sEventStartCb = 0;
	gEventQueue   = new EventQueue();
	gEventCurrent.clear();
}

// 0x8006A864
extern "C" void eventQuit()
{
	if (gEventQueue != 0) {
		delete gEventQueue;
		gEventQueue = 0;
	}
	sEventStartCb = 0;
	eventRequestClear(&sEventReq);
	if (sEventCtrlObj != 0) {
		sEventCtrlObj->mFlags |= 1;
		sEventCtrlObj = 0;
	}
}

// 0x8006A8D0
extern "C" void eventCreate()
{
	if (sEventCtrlObj == 0) {
		TEventCtrl* obj = new TEventCtrl(gTL_00);
		sEventCtrlObj   = obj;
	}
}

// 0x8006A91C
extern "C" void eventReset()
{
	if (sEventCtrlObj != 0) {
		((TEventCtrl*)sEventCtrlObj)->reset();
	}
	eventRequestClear(&sEventReq);
}

// 0x8006A954
TEventCtrl::~TEventCtrl()
{
	if (sEventCtrlObj == this) {
		sEventCtrlObj = 0;
	}
}

// 0x8006A9C8
extern "C" int eventIsBusy()
{
	if (sEventCtrlObj != 0) {
		return ((TEventCtrl*)sEventCtrlObj)->isBusy();
	}
	return 0;
}

// 0x8006A9FC
extern "C" int eventCanStart()
{
	if (sEventCtrlObj != 0) {
		return ((TEventCtrl*)sEventCtrlObj)->canStart();
	}
	return 0;
}

// 0x8006AA30
extern "C" int eventStartById(u16 id, int (*callback)(int))
{
	if (sEventCtrlObj != 0 && ((TEventCtrl*)sEventCtrlObj)->canStart()) {
		((TEventCtrl*)sEventCtrlObj)->request();
		sEventReq.mIds[0] = id;
		sEventReq.mIds[1] = 0xFFFF;
		sEventReq.mObj    = 0;
		sEventStartCb     = callback;
		return 1;
	}
	return 0;
}

// 0x8006AAB0
extern "C" int eventStartByIds(u16 id, u16 id2)
{
	if (sEventCtrlObj != 0 && ((TEventCtrl*)sEventCtrlObj)->canStart()) {
		((TEventCtrl*)sEventCtrlObj)->request();
		sEventReq.mIds[0] = id;
		sEventReq.mIds[1] = id2;
		sEventReq.mIds[2] = 0xFFFF;
		sEventReq.mObj    = 0;
		sEventStartCb     = 0;
		return 1;
	}
	return 0;
}

// 0x8006AB34
extern "C" int eventStartByObj(void* obj)
{
	if (sEventCtrlObj != 0 && ((TEventCtrl*)sEventCtrlObj)->canStart()) {
		((TEventCtrl*)sEventCtrlObj)->request();
		sEventReq.mIds[0] = 0xFFFF;
		sEventReq.mObj    = obj;
		sEventStartCb     = 0;
		return 1;
	}
	return 0;
}

// 0x8006ABAC
extern "C" int eventQueueAdd(int a, int b)
{
	if (gEventQueue != 0) {
		return gEventQueue->push((void*)a, (int (*)(void*, int))b);
	}
	return -1;
}

// 0x8006ABF0
extern "C" void eventEnd()
{
	if (sEventCtrlObj != 0) {
		((TEventCtrl*)sEventCtrlObj)->requestEnd();
	}
}

// 0x8006AC1C
extern "C" void eventHideObj(u8* obj)
{
	TObject* parent = *(TObject**)(obj + 0x10);
	if (parent == gTL_03On) {
		objSetParent((TObject*)obj, gTL_03B);
	} else if (parent == gTL_04On) {
		objSetParent((TObject*)obj, gTL_04B);
	} else if (parent == gTL_05On) {
		objSetParent((TObject*)obj, gTL_05B);
	} else if (parent == gTL_06On) {
		objSetParent((TObject*)obj, gTL_06B);
	}
}

// 0x8006AC98
extern "C" void eventHideId(int id)
{
	void* obj = findObjById(id);
	if (obj != 0) {
		eventHideObj((u8*)obj);
	}
}

// 0x8006ACC8
extern "C" void eventShowObj(u8* obj)
{
	TObject* parent = *(TObject**)(obj + 0x10);
	if (parent == gTL_03B) {
		objSetParent((TObject*)obj, gTL_03On);
	} else if (parent == gTL_04B) {
		objSetParent((TObject*)obj, gTL_04On);
	} else if (parent == gTL_05B) {
		objSetParent((TObject*)obj, gTL_05On);
	} else if (parent == gTL_06B) {
		objSetParent((TObject*)obj, gTL_06On);
	}
}

// 0x8006AD44
extern "C" void eventShowId(int id)
{
	void* obj = findObjById(id);
	if (obj != 0) {
		eventShowObj((u8*)obj);
	}
}
