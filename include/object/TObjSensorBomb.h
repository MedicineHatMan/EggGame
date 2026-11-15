#ifndef _TOBJSENSORBOMB_H
#define _TOBJSENSORBOMB_H
struct CharaColli;

#include "item/TItem.h"
struct DrawTask;
struct NJS_OBJECT;
struct NJS_TEXLIST;
#include "object/ObjCommon.h"
#include "engine/CharaColli.h"
#include "item/ItemParts.h"

extern "C" void objHeapFree(void*, void*);

/** Item 0x15. See TObjSensorBomb.cpp. @size 0x154 */
class TObjSensorBomb : public TItem {
public:
	TObjSensorBomb(TObject* parent);
	virtual ~TObjSensorBomb();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual Vec3f* getPos();
	virtual void place(const Vec3f* pos);

	s8 mState;        // _38 0 = held 1 = falling 2 = arming
	s16 mTimer;       // _3A
	Vec3f mPos;       // _3C
	DrawTask* mDrawTask;  // _48
	CharaColli* mColli;     // _4C
	ObjShadow mShadow; // _50
	ItemHolder mItem; // _88
	ItemGlow mGlow;   // _A0
	BombBody mBody;   // _100
	NJS_OBJECT* mModel;     // _14C
	NJS_TEXLIST* mTexList;   // _150
};

extern "C" {
extern const char* gTObjSensorBombName;
}

#endif
