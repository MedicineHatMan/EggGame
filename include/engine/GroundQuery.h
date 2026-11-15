#ifndef _ENGINE_GROUNDQUERY_H_
#define _ENGINE_GROUNDQUERY_H_
struct MeshEntry;
#include "engine/MeshColli.h"
struct ContactHit;
struct ContactSet;
struct MeshColli;
struct MeshGather;
struct MeshGatherPair;
struct MeshList;
struct MeshSphere;
struct EggHit;
struct FlagWord;

#include "core/types.h"

struct Vec3f;

extern "C" {
int colliCylinderContains(const Vec3f*, const Vec3f*, f32, f32);
void groundSetWorldData(void*);
void groundUpdateMoving();
void groundInit();
void groundShutdown();
}

extern "C" {
u32 meshColliProbe(EggHit*, const Vec3f*, u32);
u32 meshColliProbeLine(EggHit*, const Vec3f*, const Vec3f*, u32);
void flagWordSetBit(FlagWord*, int);
}

extern "C" {
void queryBegin(void*);
u32 queryGatherBox(MeshList*, const void*);
void* meshColliCreate();
void meshColliFree(void*);
ContactHit* contactPickSlot(ContactSet*, u32, f32);
void contactCopyResults(void*, ContactSet*);
int contactTakeHit(MeshColli*, int, f32, const Vec3f*, void*);
int meshListQueryDown(const MeshList*, const Vec3f*, u32, MeshHitFn, void*);
int meshListProbeDown(EggHit*, const MeshList*, const Vec3f*, u32);
int meshListQuerySphere(const MeshList*, const MeshSphere*, void*, void*, u32);
int queryGatherWorldSphere(MeshList*, const void*);
int queryGatherWorldLine(MeshList*, const void*, const void*);
int queryGatherWorldBox(MeshList*, const void*);
int queryGatherWorldBall(MeshList*, const void*);
void queryGatherMeshSphere(MeshColli*, MeshGather*);
void queryGatherMeshLine(MeshColli*, MeshGatherPair*);
void queryGatherMeshBall(MeshColli*, MeshGather*);
void queryGatherMeshBox(MeshColli*, MeshGather*);
u32 queryGatherSphere(MeshList*, const void*);
u32 queryGatherLine(MeshList*, const void*, const void*);
}

extern "C" {
void meshTriNormal(MeshColli*, int, Vec3f*);
#define meshTriNormalU16(mesh, tri, out) (((void (*)(MeshColli*, u16, Vec3f*))meshTriNormal)(mesh, tri, out))
u32 groundQueryDown(const void*, void*, void*, void*);
u32 groundQueryLine(const void*, const void*, void*, void*, void*);
u32 meshListProbeLine(EggHit*, const MeshList*, const Vec3f*, const Vec3f*, u32);
u32 queryGatherBall(MeshList*, const void*);
u32 groundQuerySphere(const void*, void*, void*, void*);
void meshTriCorners(MeshColli*, int, Vec3f*, Vec3f*, Vec3f*);
int meshListQueryLine(const MeshList*, const Vec3f*, const Vec3f*, u32, MeshHitFn, void*);
void meshListWalk(MeshList*, void*, void*);
int colliBoxContains(const Vec3f*, const Vec3f*, int, f32, f32, f32);
void meshTriEdge(MeshColli*, u16, u16, Vec3f*, Vec3f*);
void groundWalkGroup(void*, void*, void*);
u32 meshListQuerySphereEx(const MeshList*, const MeshSphere*, void*, void*, void*, void*, u32);
}

struct MeshList {
	MeshEntry* mData; // _00
	int _04;
	u16 mCount; // _08
};

extern "C" {
extern MeshColli* gWorldMesh;
extern void* gMovingMeshes;
}

#endif
