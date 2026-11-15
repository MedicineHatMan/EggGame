#ifndef _STAGEOBJUTIL_H
#define _STAGEOBJUTIL_H

#include "core/types.h"
#include "object/ObjAction.h"
#include "engine/MeshColli.h"
#include "engine/ModelDraw.h"

struct CharaColli;
struct ColliContact;
struct DrawState;
struct SetRot;
struct NJS_OBJECT;

/**
 * Looks through a collider's contacts. findGroup takes the first contact in a group, and findHit the first one whose
 * owner accepts a hit.
 *
 * @size 0x44
 */
struct ContactQuery {
	ContactQuery();  // 0x80051DB0
	~ContactQuery(); // 0x80051DEC

	void reset();                                         // 0x80051E28
	int findGroup(CharaColli* colli, u8 group);           // 0x80051EA0
	int findHit(TObjAction* attacker, CharaColli* colli); // 0x80051F40

	u16 mOwner;             // _00 the owner of the last contact looked at
	u16 _02;                // _02
	u16 mHitOwner;          // _04 the owner that took the hit
	ColliContact* mContact; // _08
	Vec3f mPos;             // _0C where the group contact touched
	Vec3f mHitPos;          // _18 where the hit landed
	u8 mGroup;              // _24
	u8 mHitGroup;           // _25
	HitInfo mHit;           // _28 what the owner's onHit filled in
	u16 mFlags;             // _40 bit 1 = a hit was taken
};

/**
 * A flat square collision mesh, two triangles in the XZ plane, centred on its matrix. The mesh data comes first and
 * the vtable after it, at _D4.
 *
 * @size 0xD8
 */
class MeshPlate {
public:
	MeshData mData;     // _00 what the collision mesh reads
	Vec3f mVerts[4];    // _10
	MeshTri mTris[2];   // _40
	MeshObject* mColli; // _90
	f32 mWidth;         // _94 along X
	f32 mDepth;         // _98 along Z
	f32 mRadius;        // _9C half the diagonal and one more
	u32 mMask;          // _A0 the surface bits of both triangles
	f32 _A4[12];        // _A4

	MeshPlate(u32 mask, f32 width, f32 depth); // 0x8005239C
	void setMatrix(const f32* mtx);            // 0x8005250C
	void debugDraw();                          // 0x80052558

	virtual void setVerts();  // _08 0x80052588
	virtual void setTris();   // _0C 0x800525DC
	virtual void updateBox(); // _10 0x80052660
	virtual ~MeshPlate();     // _14 0x800524B4
};

/** The pieces a broken TObjStageObj throws from its StageModelInfo */
struct ShatterBank {
	int mBank;       // _00 0 = stgobj_common 1 = geobj_common
	int mFirst;      // _04 the first model in the bank
	int mModelCount; // _08 how many models to pick from
	int mCount;      // _0C how many pieces
	f32 mScale;      // _10
	f32 mSpeed;      // _14 the top speed The slowest piece goes half that
};

/** The same with the models passed in a list */
struct ShatterList {
	NJS_OBJECT** mModels; // _00
	void* mTexture;       // _04
	int mFirst;           // _08
	int mModelCount;      // _0C
	int mCount;           // _10
	f32 mScale;           // _14
	f32 mSpeed;           // _18
};

typedef int (*ContactFilter)(ColliContact* contact, void* arg);

extern "C" {
void shatterBank(f32 scale, TObjAction* obj, const Vec3f* hit, const Vec3f* size, const ShatterBank* pieces); // 0x800526F0
void shatterListAt(TObjAction* obj, const Vec3f* hit, const Vec3f* size, const ShatterList* pieces);          // 0x80052A8C
void shatterList(TObjAction* obj, const Vec3f* center, int rotY, const Vec3f* hit, const Vec3f* size,
                 const ShatterList* pieces);                                                                            // 0x80052B2C
void shatterModel(TObjAction* obj, const Vec3f* hit, int rotY, NJS_OBJECT* model, void* texList, f32 scale, f32 speed); // 0x80052E78
void shatterModelRot(TObjAction* obj, const Vec3f* hit, const Vec3f* center, const SetRot* rot, NJS_OBJECT* model, void* texList, f32 scale,
                     f32 speed);                                                                      // 0x8005319C
void dropItem(u8 item, Vec3f* pos);                                                                   // 0x800534A0
int fadeAmbient(DrawState* state, int target, int step);                                              // 0x80053528
int colliFindContact(CharaColli* colli, int group, ContactFilter filter, void* arg);                  // 0x80053584
void colliSetSphere(CharaColli* colli, const Vec3f* pos, u32 flags, const Vec3f* offset, f32 radius); // 0x8005361C
void colliSetCylinder(CharaColli* colli, const Vec3f* pos, u32 flags, const Vec3f* offset, f32 radius,
                      f32 height);                                                                   // 0x800536BC
void modelBoundsWalk(NJS_OBJECT* node, Vec3f* min, Vec3f* max, const Vec3f* base);                   // 0x80053774
void modelBounds(f32 margin, NJS_OBJECT* model, Vec3f* min, Vec3f* max);                             // 0x800538CC
void modelSphere(f32 margin, NJS_OBJECT* model, Vec3f* min, Vec3f* max, Vec3f* center, f32* radius); // 0x80053948
void boxAroundBox(f32* box, const f32* mtx, const Vec3f* lo, const Vec3f* hi);                       // 0x800539BC
void meshDebugDraw(MeshColli* mesh, const MeshData* data);                                           // 0x80053B54
}

#endif
