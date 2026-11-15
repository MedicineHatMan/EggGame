#ifndef _TENEORANGEBOSSMAP_H
#define _TENEORANGEBOSSMAP_H
struct NJS_OBJECT;
struct NJS_TEXLIST;
struct CharaColli;

#include "core/types.h"
struct NJS_MOTION;
#include "engine/MeshColli.h"

// The orange boss's arena the eight mirrors in a ring round the middle and the one in the middle with the archive they
// come from ar_ene_orange_boss_map.arc and the screen texture they reflect
// 0x8011BE94-0x8011D784 in TEneOrangeBossMirror.cpp

class TObject;
class TObjAction;
class TEneOrangeBossMirror;

#define MIRROR_RING_COUNT 8

/**
 * The mirror's surface: a box collider and the eight-sided mesh it is reflected and hit with.
 * 0x8011CD80-0x8011D6C4.
 *
 * @size 0x1BC
 */
struct MirrorPane {
	MirrorPane();                                        // 0x8011CD80
	~MirrorPane();                                       // 0x8011CD98 retires the collider and frees the mesh
	void init(u16 objId, Vec3f* pos, int angle);         // 0x8011CDF4
	void setPose(Vec3f* pos, int angle);                 // 0x8011CE50
	void initMesh(Vec3f* pos, int angle);                // 0x8011CFB8
	void buildMesh(Vec3f* pos, int angle);               // 0x8011D048
	void initCollider(u16 objId, Vec3f* pos, int angle); // 0x8011D30C
	void placeCollider(const Vec3f* pos, int angle);     // 0x8011D3BC
	void clearShapeBit8();                               // 0x8011D474
	void clearContacts();                                // 0x8011D49C
	int isShapeBit8();                                   // 0x8011D4C4
	void enable();                                       // 0x8011D4F0
	void disable();                                      // 0x8011D514
	void meshOn();                                       // 0x8011D538
	void meshRefresh();                                  // 0x8011D55C
	int checkHit(TObjAction* by);                        // 0x8011D580 1 if something hurt it from the front
	void setDepth(f32 depth);                            // 0x8011D6BC

	CharaColli* mCollider; // _00
	f32 mDepth;            // _04 how far in front of the pos the collider sits
	Vec3f mVerts[8];       // _08
	MeshTri mTris[8];      // _68
	MeshData mData;        // _1A8
	MeshObject* mMesh;     // _1B8
};

extern "C" {
// The archive and the screen texture
void initOrangeBossMap();                    // 0x8011BE94
void shutdownOrangeBossMap();                // 0x8011BF90
NJS_TEXLIST** orangeBossMapTextures();       // 0x8011C074
NJS_OBJECT** orangeBossMapModels();          // 0x8011C07C
NJS_MOTION** orangeBossMapMotion(int index); // 0x8011C084
void mirrorCopyScreen();                     // 0x8011C0B0 the frame into the reflection texture

// The ring of eight and the one in the middle
void mirrorsHide();                                                        // 0x8011C10C
void mirrorsCreate();                                                      // 0x8011C184
TEneOrangeBossMirror* mirrorAt(int slot);                                  // 0x8011C270
void mirrorForget(TEneOrangeBossMirror* mirror);                           // 0x8011C2A0
int mirrorSlotOf(const Vec3f* dir);                                        // 0x8011C2FC
int mirrorSlotOpposite(const Vec3f* dir);                                  // 0x8011C34C
int mirrorSlotAngle(int slot);                                             // 0x8011C384
int mirrorSlotDir(int slot, Vec3f* dir);                                   // 0x8011C38C
void mirrorSlotPos(int slot, Vec3f* pos, int* angle);                      // 0x8011C3D4
int mirrorIsClear(int slot);                                               // 0x8011C444 the mirror there has nothing at _210
int mirrorSlotFacing(f32 width, const Vec3f* dir, int* slot);              // 0x8011C478
void mirrorListRaise(TEneOrangeBossMirror** list, int count);              // 0x8011C594
void mirrorListLower(TEneOrangeBossMirror** list, int count);              // 0x8011C5F0
int mirrorListAllInMode(TEneOrangeBossMirror** list, int count, int mode); // 0x8011C64C
void mirrorsReset();                                                       // 0x8011C6CC
void mirrorCenterClearFlag20();                                            // 0x8011C744
u32 mirrorCenterFlag20();                                                  // 0x8011C77C
u16 mirrorCenterId();                                                      // 0x8011C7BC
void mirrorRingSetFlag1();                                                 // 0x8011C800
void mirrorRingClearFlag1();                                               // 0x8011C85C
void mirrorRingSetFlag2();                                                 // 0x8011C8B8
void mirrorRingSetTarget(int target);                                      // 0x8011C914
void mirrorAimAt(int slot, const Vec3f* pos);                              // 0x8011C978
void mirrorRingRaise();                                                    // 0x8011C9B4
void mirrorRingLower();                                                    // 0x8011C9E0
int mirrorRingAllDown();                                                   // 0x8011CA0C
int mirrorRingAllUp();                                                     // 0x8011CA38
void mirrorRingMoveSe();                                                   // 0x8011CA64
void mirrorCenterSetFlag2();                                               // 0x8011CAAC
void mirrorCenterRaise(int se);                                            // 0x8011CAE4
void mirrorCenterLower(int se);                                            // 0x8011CB24
int mirrorCenterDown();                                                    // 0x8011CB64
int mirrorCenterUp();                                                      // 0x8011CB90
void mirrorCenterTurn(int angle);                                          // 0x8011CBBC
void mirrorCenterSolid();                                                  // 0x8011CBF8
void mirrorCenterHollow();                                                 // 0x8011CC30
void mirrorCenterAimAt(const Vec3f* pos);                                  // 0x8011CC68
void mirrorCenterMoveSe();                                                 // 0x8011CCA4
void mirrorCenterMesh(int on);                                             // 0x8011CCF8
void mirrorCenterSetFlag40();                                              // 0x8011CD48

TEneOrangeBossMirror* createMirrorRing(TObject* parent, const Vec3f* pos, int angle);   // 0x8011D6C4
TEneOrangeBossMirror* createMirrorCenter(TObject* parent, const Vec3f* pos, int angle); // 0x8011D724
}

#endif
