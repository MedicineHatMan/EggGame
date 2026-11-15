#ifndef _TOBJREDFENCECTRL_H
#define _TOBJREDFENCECTRL_H
struct SetDesc;
struct TObject;
struct MeshObject;

#include "stage/SetManager.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The four-sided red fence. Set int 0 is the switch that starts it and set int 1 is the switch that opens it.
 * It drops, swings, then tips over. Each side is a TObjRedFence.
 *
 * @size 0xB0
 */
class TObjRedFenceCtrl : public TObjAction {
public:
	TObjRedFenceCtrl(TObject* parent, const SetDesc* desc);
	virtual ~TObjRedFenceCtrl();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual Vec3f* getPos();                  // _20
	virtual int getRotY();                    // _28
	virtual int getKind();                    // _2C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void setPos(const Vec3f* pos);    // _6C

	// Names are ours
	void build();
	void fitBox(f32* mtx);
	void watch();
	void fall();
	void swing();
	void open();
	void place(const Vec3f* off);

	u32 _30;             // _30
	MeshObject* mLand;   // _34
	SetLinkMember mLink; // _38
	DrawTask* mDrawTask; // _44
	Vec3f mHome;         // _48 the record position copied again into mTrans
	SetTransform mTrans; // _54
	int mState;          // _98 0 waiting 1 dropping 2 swinging 3 opening
	u16 mSwitch;         // _9C set int 0
	u16 mOpenId;         // _9E set int 1
	f32 mExtra;          // _A0 stored from sdata2 and not read again
	int mAngle;          // _A4 copied into mTrans.mRot.x
	int mSwing;          // _A8
	int mRate;           // _AC
};

extern "C" {
TObject* createTObjRedFenceCtrl(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjRedFenceCtrlName;
}

#endif
