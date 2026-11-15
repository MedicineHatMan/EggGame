#ifndef _TOBJBRIDGEITA_H
#define _TOBJBRIDGEITA_H
struct NJS_OBJECT;

#include "object/ObjAction.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"

extern "C" {
void objHeapFree(void*, void*);
extern void* gObjHeap;
extern const f32 gBridgePlankLength; // 0x802C8D40 40
extern const f32 gBridgeWidth;       // 0x802C8D44 50
}

/**
 * @size 0x154
 */
class TObjBridgeIta : public TObjAction {
public:
	TObjBridgeIta(TObject* parent); // 0x800D4DF4
	virtual ~TObjBridgeIta();       // 0x800D4E90

	virtual void draw();      // _0C
	virtual void vfn_10();    // _10
	virtual void render();    // _1C
	virtual Vec3f* getPos();  // _20
	virtual int getRotY();    // _28
	virtual int getKind();    // _2C
	virtual void init();                  // _6C 0x800D4DAC
	virtual void setPos(const Vec3f* pos); // _70 0x800D51A4

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void setPlankKind(int kind); // 0x800D4F1C MODEL_TURIBASHI_ITA01 to 03
	void setRope(int rope);      // 0x800D4FC0 the rope model or the post HASHIRA01 when 0

	NJS_OBJECT* mPlankModel; // _30
	NJS_OBJECT* mSideModel;  // _34
	void* mPlankTex;   // _38
	void* mSideTex;    // _3C
	DrawTask* mDraw;   // _40
	EnemyLight mLight; // _44
	Vec3f mPos;        // _84
	Vec3f mPrevPos;    // _90 where it is drawn
	int mTilt;         // _9C with mRotY and _A4 the X Y and Z angles
	int mRotY;         // _A0
	int _A4;           // _A4
	ObjModel mModel0;  // _A8
	ObjModel mModel1;  // _C8
	ObjPlace mPlace;   // _E8
	f32 mWidth;        // _144
	f32 mScale;        // _148
	f32 mDepth;        // _14C
	u8 _150[4];
};

#endif
