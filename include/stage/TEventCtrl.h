#ifndef _STAGE_TEVENTCTRL_H_
#define _STAGE_TEVENTCTRL_H_
struct EventQueue;
struct TObject;
struct EventRequest;
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct TEventCtrl;

#include "core/types.h"

extern "C" {
int eventCanStart();
void eventCreate();
void eventEnd();
void eventReset();
}

extern "C" {
int eventStartByIds(u16, u16);
void eventHideObj(u8*);
void eventShowId(int);
}

extern "C" {
void eventHideId(int);
// A handle id is already a u16 here and the DOL does not narrow it again
#define eventHideIdU16(id) (((void (*)(u16))eventHideId)(id))
int eventIsBusy();
int eventQueueAdd(int, int);
}

extern "C" {
int eventStartByObj(void*);
}

/**
 * @size 0x38
 */
class TEventCtrl : public TObject {
public:
	TEventCtrl(TObject* parent);
	virtual ~TEventCtrl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // render is TObject's

	void reset();
	int isFree();
	void start();
	void end();
	void request();
	void requestEnd();
	int isBusy();
	int canStart();

	u8 mState;        // _20
	FlagByte mFlag;   // _21 mBits 1 requested 2 end 4 from the queue 8 blocked
	int _24;          // _24
	int mCamera;      // _28
	TObject* mObjParent;// _2C
	int _30;          // _30
	TObject* mDesignParent;// _34
};

struct EventRequest {
	u16 mIds[4]; // _00
	void* mObj;  // _08
};

#include "stage/EventQueue.h"
extern "C" {
extern const char* gTEventCtrlName;
extern TObject* sEventCtrlObj;
extern EventQueue* gEventQueue;
extern EventRequest sEventReq;
extern EventEntry gEventCurrent;
}

extern "C" {
int eventStartById(u16 id, int (*callback)(int));
}

#endif
