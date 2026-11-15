#ifndef _TITEMONCE_H
#define _TITEMONCE_H

#include "item/TItemEquipment.h"
#include "engine/DrawTask.h"

/**
 * @size 0x104
 */
class TItemOnce : public TItem {
public:
	TItemOnce(TObject* parent, const ItemSpec* spec);
	virtual ~TItemOnce();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();     // _0C
	virtual void vfn_10();   // _10 empty
	virtual void render();   // _1C
	virtual Vec3f* getPos(); // _20
	virtual void vfn_6C();   // _6C

	int mActive;        // _38 set by vfn_6C
	s8 mState;          // _3C 0 rising 1 waiting
	u8 _3D;             // _3D
	u16 mTimer;         // _3E
	ItemSpec mSpec;     // _40
	ItemHolder mHolder; // _48
	ItemGlow mGlow;     // _60
	f32 mMtx[12];       // _C0
	Vec3f mPos;         // _F0
	int mAngle;         // _FC the owner's getRotY
	DrawTask* mDraw;    // _100
};

extern "C" {
extern const char* gTItemOnceName;
extern f32 gItemOnceRise;
extern const f32 gItemOnceStartHeight;
}

#endif
