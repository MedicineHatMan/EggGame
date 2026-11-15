#ifndef _TPLYDEMOLASTSUPER_H
#define _TPLYDEMOLASTSUPER_H
#include "player/Player.h"
struct LastSuperExport;
struct TArc;
struct Fx;

#include "stage/Stage.h"
#include "ui/TSimpleText.h"

extern "C" void objHeapFree(void*, void*);

class TChrPlayer;

/**
 * The super form of the last demo. draw() calls mStep. The text is held once it closes.
 *
 * @size 0x6C
 */
class TCamPath;
class TPlyDemoLastSuper : public TObject {
public:
	TPlyDemoLastSuper(TObject* parent);
	virtual ~TPlyDemoLastSuper();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	TChrPlayer* player();

	// The names of the steps are ours
	void stepOpen();
	void stepBurst();
	void stepPlay();
	void stepClose();

	int mTimer;                         // _20
	TCamPath* mEvent;                   // _24
	TSimpleText* mText;                 // _28
	Fx* mPart;                          // _2C
	f32 mMtx[12];                       // _30
	void (TPlyDemoLastSuper::*mStep)(); // _60
};

extern "C" {
void plyDemoLastSuperBind(TArc*);
}

// event_super the demo's export then what the three light effects play
struct LastSuperExport : PlyDemoExport {
	void* mLights[3]; // _1C
};

extern "C" {
extern u32 gTPlyDemoLastSuperName;
extern struct LastSuperExport* gLastSuperExports;
extern u32 sLastSuperColor0;
extern u32 sLastSuperColor1;
extern u32 sLastSuperColor2;
}

#endif
