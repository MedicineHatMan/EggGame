#ifndef _OBJECT_LAST_TOBJLASTCROWLEMURES_H_
#define _OBJECT_LAST_TOBJLASTCROWLEMURES_H_
#include "engine/CharaColli.h"
#include "engine/ModelDraw.h"
#include "stage/SetActor.h"
#include "stage/Stage.h"
struct TObjLastCrowLemures;

#include "core/types.h"

class TObjLastCrowLemures : public TObject {
public:
	TObjLastCrowLemures(TSetActor* owner);
	virtual ~TObjLastCrowLemures();

	void* operator new(u32 size) { return objHeapAlloc(gObjHeap, size); }
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();

	void touch(int on); // 0x80190FB4

	TSetActor* mOwner;   // _20
	MotionTrack mTrack;  // _24
	CharaColli* mShape;        // _40
	Vec3f mPos;          // _44
	Vec3f mTarget;       // _50 where it is flying
	f32 mScale;          // _5C
	int mYaw;            // _60
	f32 mMtx[12];        // _64
	f32 mSpeed;          // _94
};

#endif
