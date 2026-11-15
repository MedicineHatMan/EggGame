#ifndef _OBJECT_LAST_TOBJLASTEXBOSS_BGCTRL_H_
#define _OBJECT_LAST_TOBJLASTEXBOSS_BGCTRL_H_
#include "engine/FxHolder.h"
#include "stage/Stage.h"
struct TObjLastExBoss_BGCtrl;

#include "core/types.h"

extern "C" {
void armLastExBg();
void clearLastExBg();
void releaseLastExBg();
}

extern "C" {
void createTObjLastExBoss_BGCtrl();
}

class TObjLastExBoss_BGCtrl : public TObject {
public:
	TObjLastExBoss_BGCtrl(TObject* parent);
	virtual ~TObjLastExBoss_BGCtrl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	int mMode;      // _20 1 = stop
	int _24;        // _24
	int mWait;      // _28 frames until the next piece
	FxHolder mGlow; // _2C
};

extern "C" {
extern const int gGarekiModels[5];
}

#endif
