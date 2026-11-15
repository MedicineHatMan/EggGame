#ifndef _OBJECT_LAST_TOBJLASTHEAVENSGATE_H_
#define _OBJECT_LAST_TOBJLASTHEAVENSGATE_H_
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "stage/SetActor.h"
#include "stage/Stage.h"
struct TObjLastHeavensGate;

#include "core/types.h"

struct Vec3f;

class TObjLastHeavensGate : public TObjAction {
public:
	TObjLastHeavensGate(TSetActor* owner);
	virtual ~TObjLastHeavensGate();

	void* operator new(u32 size) { return objHeapAlloc(gObjHeap, size); }
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int getKind(); // 0x80191948

	void place(const Vec3f* pos); // 0x801917C0
	void setRadius(f32 radius);   // 0x80191880
	void getCenter(Vec3f* pos) const;   // 0x801918B4
	f32 getRadius() const;              // 0x801918FC
	void drawShape();             // 0x80191924

	TSetActor* mOwner; // _30
	CharaColli* mShape;      // _34
	void* mFx;         // _38
	f32 mMtx[12];      // _3C
};

#endif
