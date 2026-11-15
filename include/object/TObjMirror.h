#ifndef _OBJECT_TOBJMIRROR_H_
#define _OBJECT_TOBJMIRROR_H_
#include "egg/ObjEgg.h"
#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct DrawTask;
struct SetLink;
struct TObjMirror;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjMirror(TObject*, SetDesc*);
}

class TObjMirror : public TObjAction {
public:
	TObjMirror(TObject* parent, SetDesc* desc);
	virtual ~TObjMirror();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getRotY();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual SetLink* getLink(); // _6C

	void reset();
	void drawQuad();

	int mAct;            // _30
	u8 _34[4];           // _34
	f32 mScrollSpeed;    // _38 0.01 + the first float of the record
	u16 mPattern;        // _3C the first int of the record modulo 3
	Vec3f mPos;          // _40
	SetRot mRot;         // _4C
	HitVec mCorners[4];  // _58
	SetLinkMember mLink; // _88
	DrawTask* mDrawTask;     // _94
	u8 _98[0xC];         // _98
};

extern "C" {
extern const char* gTObjMirrorName;
extern const Vec3f sMirrorCorners[4];
}

#endif
