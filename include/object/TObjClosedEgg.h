#ifndef _OBJECT_TOBJCLOSEDEGG_H_
#define _OBJECT_TOBJCLOSEDEGG_H_
#include "mode/TModeGallery.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjClosedEgg;
struct SetDesc;
struct TObjAction;
struct TObject;
struct TPlyEffStSleep;

#include "core/types.h"

extern "C" {
s16 galleryEggFoundBit(s8);
s16 galleryEggModel(s8);
void initClosedEgg();
void loadGalleryEggArcs();
void releaseGalleryEggArcs();
void termClosedEgg();
}

extern "C" {
TPlyEffStSleep* createPlyEffStSleep(TObject*);
TObject* createTObjClosedEgg(TObject*, SetDesc*);
}

class TObjClosedEgg : public TObjAction {
public:
	TObjClosedEgg(TObject* parent, const SetDesc* desc); // 0x801BAC24
	virtual ~TObjClosedEgg();                            // 0x801BACEC

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C 0x801BAEFC
	virtual void vfn_10();                    // _10 0x801BAF54
	virtual void render();                    // _1C 0x801BAFBC
	virtual Vec3f* getPos();                  // _20 0x801BB0F4
	virtual int getKind();                    // _2C 0x801BB0EC
	virtual void initFromDesc(SetDesc* desc); // _30 0x801BAD98
	virtual void debugPrint(void*);           // _34 0x801BAE9C
	virtual f32 getRadiusV();                 // _54 0x801BB104
	virtual SetLink* getLink();               // _70 0x801BB0FC

	u16 mEggType;        // _30
	u16 mKey;            // _32
	void* mModel;        // _34 the egg image on the shell
	Vec3f mPos;          // _38
	int mRotY;           // _44
	DrawTask* mDraw;     // _48
	SetLinkMember mLink; // _4C
	ObjShadow mShadow;   // _58
	f32 mMtx[12];        // _90
};

extern "C" {
extern const char* gTObjClosedEggName;
extern int gGalleryFxNodes[5];
}

extern "C" {
extern GalleryEggInfo gGalleryEggTable[GALLERY_EGG_COUNT]; // 0x80267E18
}

#endif
