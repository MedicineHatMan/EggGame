#ifndef _OBJECT_TOBJWINDCOLLI_H_
#define _OBJECT_TOBJWINDCOLLI_H_
struct TWindCtrl;
#include "object/ObjAction.h"
struct DrawTask;
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjWindColli;
struct TObject;

#include "core/types.h"

class SetDesc;
struct Vec3f;

extern "C" {
void prepareTObjWindColli(SetDesc*);
void windCreate();
void windGet(Vec3f*);
}

extern "C" {
TObject* createTObjWindColli(TObject*, SetDesc*);
}

/**
 * @size 0x80
 */
class TObjWindColli : public TObjAction {
public:
	TObjWindColli(TObject* parent, const SetDesc* desc);
	virtual ~TObjWindColli();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual int getKind() { return 3; }
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void* arg);
	virtual SetLink* getLink() { return mLink.get(); } // _6C

	u8 mType;            // _30
	f32 mMatrix[12];     // _34 the shape a unit sphere or box in it
	Vec3f mWind;         // _64
	SetLinkMember mLink; // _70
	DrawTask* mDraw;         // _7C
};

/**
 * Holds one wind vector, set when it is created from gWindDefault. A DEFAULT TObjWindColli overwrites it.
 * @size 0x2C
 */
class TWindCtrl : public TObject {
public:
	TWindCtrl(TObject* parent);
	virtual ~TWindCtrl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void getWind(Vec3f* out) const;
	void setWind(const Vec3f* wind)
	{
		mWind.x = wind->x;
		mWind.y = wind->y;
		mWind.z = wind->z;
	}

	Vec3f mWind; // _20
};

extern "C" {
extern const char* gTWindCtrlName;
extern Vec3f gWindDefault;
extern Vec3f sWindLine[2];
extern const char* sWindTypeNames[3];
extern TWindCtrl* gWindCtrl;
}

#endif
