#ifndef _OBJECT_LAST_TOBJLASTCROWLEMURESCTRL_H_
#define _OBJECT_LAST_TOBJLASTCROWLEMURESCTRL_H_
#include "stage/SetActor.h"
struct DrawTask;
struct NJS_OBJECT;
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjLastCrowLemuresCtrl;
class TObjLastHeavensGate;
struct SetDesc;
struct TObject;

#include "core/types.h"

struct Vec3f;

extern "C" {
TObject* createTObjLastCrowLemuresCtrl(TObject*, SetDesc*);
}

class TObjLastCrowLemuresCtrl : public TSetActor {
public:
	TObjLastCrowLemuresCtrl(TObject* parent, SetDesc* desc);
	virtual ~TObjLastCrowLemuresCtrl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void arm();  // _74 0x801909F4
	void place(Vec3f* pos, f32* radius) const; // 0x80190A40
	f32 range();         // 0x80190A94 how far a crow may fly
	void stateWatch();   // 0x80190BB0
	void stateOpen();    // 0x80190C54
	void stateDone();    // 0x80190CA0

	NJS_OBJECT* mModel;    // _64
	void* mMotion;   // _68
	DrawTask* mDraw;     // _6C
	int _70;         // _70
	u8 _74[0x1C];    // _74
	int _90;         // _90
	u16 mTimer;      // _94 counts down once the crows are gone
	u16 mSwitchId;   // _96
	TObjLastHeavensGate* mGate; // _98
	int mMode;       // _9C 0 watching 1 opening 2 done
};

#endif
