#ifndef _TOBJPURPLEICEFENCED_H
#define _TOBJPURPLEICEFENCED_H
struct SetDesc;
struct TObject;
struct MeshColli;
struct MeshData;

#include "stage/SetActor.h"
struct DrawTask;
#include "object/ObjModel.h"
#include "enemy/EnemyLight.h"
#include "Ninja/ninja.h"

extern "C" void objHeapFree(void*, void*);

/**
 * An ice fence. The model, texture list and collision are MODEL_ICE_FENCE_D and the two names beside it.
 * One box, 23 by 16 by 8, sits 16 above mPos. onHit reports the record's first int and sets hit flag 13.
 * Past 850 it deletes itself so the set can spawn it again. Act 1 finishes the link instead, and this
 * class only stores act 0.
 *
 * @size 0x144
 */
class TObjPurpleIceFenceD : public TSetActor {
public:
	TObjPurpleIceFenceD(TObject* parent, SetDesc* desc);
	virtual ~TObjPurpleIceFenceD();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                          // _0C
	virtual void vfn_10();                        // _10
	virtual void render();                        // _1C
	virtual void initFromDesc(SetDesc* desc);     // _30
	virtual void debugPrint(void*);               // _34
	virtual int onHit(const TObjAction* other, int, int* info); // _58
	virtual void vfn_74();                        // _74

	void makeShape();
	void updateShape();
	void freeShape();
	static void setColliBox(void* colli, const Vec3f* center, f32 radius);
	void initColli();
	void updateColli();
	void freeColli();
	void stateWait();
	void stateDone();

	NJS_OBJECT* mModelData;  // _64
	NJS_TEXLIST* mTexList;    // _68
	MeshData* mColliData;  // _6C
	DrawTask* mDrawTask;   // _70
	ObjModel mModel;   // _74
	ObjDrawState mState;  // _94
	f32 mMatrix[12];   // _B8
	MeshColli* mColli;      // _E8 built from mColliData
	int mPower;        // _EC the record's first int what onHit reports
	Vec3f mCenter;     // _F0 the model's centre moved into the world
	f32 mRadius;       // _FC the model's radius
	EnemyLight mLight; // _100
	int mAct;          // _140 0 = stay 1 = finish the link and delete
};

extern "C" {
void initPurpleIceFenceDSet();
void termPurpleIceFenceDSet();
}

extern "C" {
TObject* createTObjPurpleIceFenceD(TObject*, SetDesc*);
}

#endif
