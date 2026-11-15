#ifndef _OBJECT_LAST_TOBJLASTRESONANCESWITCH_H_
#define _OBJECT_LAST_TOBJLASTRESONANCESWITCH_H_
#include "engine/CharaColli.h"
#include "object/TObjPilingSwitch.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjLastResonanceSwitch;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjLastResonanceSwitch(TObject*, SetDesc*);
}

class TObjLastResonanceSwitch : public TObjPilingSwitch {
public:
	TObjLastResonanceSwitch(TObject* parent, SetDesc* desc);
	virtual ~TObjLastResonanceSwitch();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void stateIdle();
	void stateLit();
	void stateHeld();

	CharaColli* mRing; // _1A4 the touch ring
	u16 mTimer;  // _1A8 frames left in the lit state
	s16 _1AA;
	int mMode; // _1AC 0 = idle 1 = lit 2 = held
};

#endif
