#ifndef _ENGINE_MESHPOOL_H_
#define _ENGINE_MESHPOOL_H_
struct MeshTreeNode;
struct MeshData;
struct MeshLink;
struct MeshList2;
struct MeshPool;

#include "core/types.h"

class MeshObject;

extern "C" {
int boxLongestAxis(const f32*);
void meshSetState200(MeshObject*, int);
int meshSortCmpX(const u16*, const u16*);
int meshSortCmpY(const u16*, const u16*);
int meshSortCmpZ(const u16*, const u16*);
void meshTreeNodeCtor();
}

extern "C" {
void meshColliSetData(MeshObject*, MeshData*);
void meshColliSetBox(MeshObject*, const f32*);
void meshColliRefresh(MeshObject*);
void meshClearState400(MeshObject*);
void meshSetState100(MeshObject*, int);
void meshSetFlag2(MeshObject*, int);
void meshPoolBox(MeshPool*, f32*, const u16*, u32);
void meshTreeRebuild(MeshPool*);
u32 meshTreeBuild(MeshPool*, const f32*, u16*, u32, u32);
int meshTreeSplitX(MeshPool*, const f32*, u16*, u32);
int meshTreeSplitY(MeshPool*, const f32*, u16*, u32);
int meshTreeSplitZ(MeshPool*, const f32*, u16*, u32);
void meshLinkListCtor(MeshList2*);
MeshList2* meshLinkListDtor(MeshList2*, s16);
void meshLinkListAppend(MeshList2*, MeshLink*);
void meshLinkListRemove(MeshList2*, MeshLink*);
void meshLinkListUnlinkAll(MeshList2*);
}

extern "C" {
MeshPool* meshPoolInit(MeshPool*, int);
MeshPool* meshPoolDestroy(MeshPool*, s16);
MeshObject* meshPoolCreate(MeshPool*);
void meshPoolFree(MeshPool*, MeshObject*);
void meshPoolUpdate(MeshPool*);
}

struct MeshTreeNode { // size 0x20
	u16 mIsLeaf;      // _00
	u16 mSlot;        // _02
	u16 mLeft;        // _04
	u16 mRight;       // _06
	f32 mMin[3];      // _08
	f32 mMax[3];      // _14
};

template <typename T>
struct MeshVector {
	T* mData;  // _00
	int mSize; // _04
	int mCap;  // _08
};

struct MeshList2 {
	MeshLink* mTail; // _00
	MeshLink* mHead; // _04
	int mCount;      // _08
};

/** @size 0x40 */
struct MeshPool {
	MeshList2 mList;                 // _00
	MeshVector<MeshTreeNode> mNodes; // _0C
	MeshVector<MeshObject*> mSlots;  // _18
	MeshVector<u16> mOrder;          // _24
	MeshVector<u16> mScratch;        // _30
	u16 mNodeCount;                  // _3C
};

extern "C" {
extern MeshObject** sMeshSortSlots;
}

extern "C" {
void meshU16VecCtor(MeshVector<u16>* v);
void meshU16VecClear(MeshVector<u16>* v);
void meshU16VecAlloc(MeshVector<u16>* v, int cap);
void meshU16VecCopy(MeshVector<u16>* v, u16* to, const u16* from, int count);
void meshU16VecResize(MeshVector<u16>* v, int size);
void meshU16VecReserve(MeshVector<u16>* v, int cap);
void meshU16VecPush(MeshVector<u16>* v, const u16* value);
void meshPtrVecCtor(MeshVector<MeshObject*>* v);
void meshNodeVecCtor(MeshVector<MeshTreeNode>* v);
void meshNodeVecResize(MeshVector<MeshTreeNode>* v, int size);
void meshPtrVecResize(MeshVector<MeshObject*>* v, int size);
void meshNodeVecReserve(MeshVector<MeshTreeNode>* v, int cap);
void meshPtrVecReserve(MeshVector<MeshObject*>* v, int cap);
void meshPtrVecAlloc(MeshVector<MeshObject*>* v, int cap);
void meshPtrVecCopy(MeshVector<MeshObject*>* v, MeshObject** to, MeshObject* const* from, int count);
void meshNodeVecCopy(MeshVector<MeshTreeNode>* v, MeshTreeNode* to, const MeshTreeNode* from, int count);
void meshNodeVecAlloc(MeshVector<MeshTreeNode>* v, int cap);
}

typedef void (*MeshPoolFn)(MeshObject*, void*);
extern "C" {
void meshPoolQuery(MeshPool* pool, const f32* box, MeshPoolFn fn, void* arg);
void meshTreeQuery(MeshPool* pool, const f32* box, MeshPoolFn fn, void* arg, u32 index);
}

#endif
