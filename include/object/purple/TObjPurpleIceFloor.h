#ifndef _TOBJPURPLEICEFLOOR_H
#define _TOBJPURPLEICEFLOOR_H
struct SetDesc;
struct TObject;
struct MeshObject;
struct MeshData;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"

extern "C" void objHeapFree(void*, void*);

/**
 * An ice floor. MODEL_ICE_FLOOR, its texture list and its collision come from the ge-obj archive, and
 * the shatter uses ge-obj model 5. The hit shape is a cylinder, radius 140 and height 2.5, with the
 * push flag set. A shape with flag 0x400 whose onHit sets bit 15 breaks it: sound 0x238, the mesh goes
 * off, and the next frame shatters the model and finishes the link. Past 1050 it deletes itself.
 *
 * @size 0xE0
 */
class TObjPurpleIceFloor : public TSetActor {
public:
	TObjPurpleIceFloor(TObject* parent, SetDesc* desc);
	virtual ~TObjPurpleIceFloor();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74

	void makeShape();
	void updateShape();
	void freeShape();
	static void setColliBox(void* colli, const Vec3f* center, f32 radius);
	void initColli();
	void updateColli();
	void freeColli();
	void stateWait();
	void shatter();

	NJS_OBJECT* mModelData; // _64 MODEL_ICE_FLOOR
	void* mBreakModel;      // _68 ge-obj model 5
	NJS_TEXLIST* mTexList;  // _6C
	MeshData* mColliData;   // _70
	DrawTask* mDrawTask;    // _74 list 0x22
	ObjModel mModel;        // _78
	f32 mMatrix[12];        // _98
	MeshObject* mColli;     // _C8 built from mColliData
	Vec3f mCenter;          // _CC the model's centre moved into the world
	f32 mRadius;            // _D8 the model's radius
	int mAct;               // _DC 0 = waiting for a hit 1 = shatter and delete
};

extern "C" {
void initPurpleIceFloorSet();
void termPurpleIceFloorSet();
}

extern "C" {
TObject* createTObjPurpleIceFloor(TObject*, SetDesc*);
}

#endif
