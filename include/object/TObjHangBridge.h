#ifndef _OBJECT_TOBJHANGBRIDGE_H_
#define _OBJECT_TOBJHANGBRIDGE_H_
#include "Ninja/ninja.h"
#include "egg/ObjEgg.h"
#include "engine/DrawTask.h"
#include "engine/MeshColli.h"
#include "engine/SpringBody.h"
#include "sound/SoundHandle.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjHangBridge;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjHangBridge(TObject*, SetDesc*);
}

class TObjHangBridge : public TSetActor {
public:
	TObjHangBridge(TObject* parent, SetDesc* desc);
	virtual ~TObjHangBridge();
	virtual void draw();   // _0C 0x800D5B38
	virtual void vfn_10(); // _10 0x800D5E34
	virtual void render(); // _1C 0x800D5E38

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void setup(); // _74 0x800D5548

	void updateDeck();                           // 0x800D5EF8
	int pitchOf(Vec3f* dir);                     // 0x800D6174
	void refreshTris();                          // 0x800D6210
	int buildVerts();                            // 0x800D6440
	int buildIndices();                          // 0x800D6588
	int buildMesh();                             // 0x800D66BC
	void buildTris(Vec3f* verts, MeshTri* tris); // 0x800D6738
	void fitBox();                               // 0x800D699C

	NJS_OBJECT* mPostModel; // _64
	void* mPostTex;         // _68
	DrawTask* mDraw;        // _6C
	f32 mMtx[12];           // _70
	Vec3f mPost0;           // _A0
	Vec3f mPost1;           // _AC
	Vec3f mRopeFrom;        // _B8
	Vec3f mRopeTo;          // _C4
	MeshObject* mMesh;      // _D0
	u32 mCount;             // _D4 the planks
	u32 mVertCount;         // _D8 four rows of mCount+1
	u32 mTriCount;          // _DC
	Vec3f* mVerts;          // _E0
	u16* mIndices;          // _E4 freed once the triangles are made
	MeshTri* mTris;         // _E8
	MeshData mMeshData;     // _EC
	u8 _FC[0xC];
	SpringRope* mRope;      // _108
	HitVec* mForces;        // _10C one per rope point pushed down by whoever stands near
	int mAnchor0;           // _110
	int mAnchor1;           // _114
	SoundHandle mSnd;       // _118 the creak
	MeshObject* mPost0Mesh; // _144
	MeshObject* mPost1Mesh; // _148
};

#endif
