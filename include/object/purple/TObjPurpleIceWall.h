#ifndef _TOBJPURPLEICEWALL_H
#define _TOBJPURPLEICEWALL_H
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
 * An ice wall. MODEL_ICE_WALL, its texture list and its collision come from the ge-obj archive.
 * The hit box is 85 by 70 by 5 on mPos, with the push flag set. A contact whose shape has flag 8
 * breaks it, unless that shape is in group 8 and also pushes. The break plays 0x238, turns the mesh
 * off, then the next frame bursts the model and finishes the link. Past 2050 it deletes itself.
 *
 * @size 0xDC
 */
class TObjPurpleIceWall : public TSetActor {
public:
	TObjPurpleIceWall(TObject* parent, SetDesc* desc);
	virtual ~TObjPurpleIceWall();

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

	NJS_OBJECT* mModelData;    // _64
	NJS_TEXLIST* mTexList;      // _68
	MeshData* mColliData;    // _6C
	DrawTask* mDrawTask; // _70 list 0x22
	ObjModel mModel;     // _74
	f32 mMatrix[12];     // _94
	MeshObject* mColli;        // _C4 built from mColliData
	Vec3f mCenter;       // _C8 the model's centre moved into the world
	f32 mRadius;         // _D4 the model's radius
	int mAct;            // _D8 0 = waiting for a hit 1 = burst and delete
};

extern "C" {
void initPurpleIceWallSet();
void termPurpleIceWallSet();
}

extern "C" {
TObject* createTObjPurpleIceWall(TObject*, SetDesc*);
}

#endif
