#ifndef _OBJECT_TOBJARROWPOINT_H_
#define _OBJECT_TOBJARROWPOINT_H_
#include "object/ObjAction.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjArrowPoint;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjArrowPoint(TObject*, SetDesc*);
}

class TObjArrowPoint : public TSetActor {
public:
	TObjArrowPoint(TObject* parent, SetDesc* desc);
	virtual ~TObjArrowPoint();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void stateWait();                  // 0x80054188
	void stateTrack();                 // 0x800541C8
	void stateFly();                   // 0x80054220
	void stateFire();                  // 0x80054358
	void stateDone();                  // 0x800543B0
	void wakeSkull();                  // 0x800543B4
	void launchFrom(TObjAction* from); // 0x80054424
	void raiseMarker();                // 0x800546C4
	inline void makeMarker();
	void aimAtSkull(); // 0x80054784

	ObjRef mFrom;    // _64 the skull
	ObjRef mTrack;   // _70 the ball a TObjLauncher
	ObjRef mMarker;  // _7C the fire a TObjFirePole01
	TObject* mChild; // _88
	u16 mSkull;      // _8C
	u16 mKey;        // _8E
	u8 _90[4];
	int mAct;   // _94 an ArrowPointState
	f32 mScale; // _98
	f32 mAnim;  // _9C
};

#endif
