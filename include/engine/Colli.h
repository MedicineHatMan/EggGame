#ifndef _ENGINE_COLLI_H_
#define _ENGINE_COLLI_H_
struct TCharaColliCtrl;
struct Vec3f;

#include "core/types.h"

struct CharaColli;
struct ColliContact;
struct ColliShape;

extern "C" {
ColliContact* colliAllocContact();
void colliClear(CharaColli*);
void colliClearDebug();
void colliContactInit(ColliContact*);
int colliCylinderBox(ColliShape*, ColliShape*, int);
int colliCylinderCylinder(ColliShape*, ColliShape*, int);
void colliExitPush();
void colliFreeShapes(CharaColli*);
ColliShape* colliGetBounds(CharaColli*);
int colliHasBounds(CharaColli*);
void colliInitPush();
int colliIsFirm(CharaColli*, int);
int colliIsSensor(CharaColli*, int);
void colliLink(CharaColli*, CharaColli*);
CharaColli* colliNext(CharaColli*);
void colliReset(CharaColli*);
void colliShapeInit(ColliShape*);
void colliShapeSetBox(ColliShape*);
int colliSphereBox(ColliShape*, ColliShape*, int);
int colliSphereCylinder(ColliShape*, ColliShape*, int);
int colliSphereSphere(ColliShape*, ColliShape*, int);
void colliUpdate();
}

extern "C" {
CharaColli* colliCreate(int, int);
void colliSetOwner(CharaColli*, u32);
void colliSetMasks(CharaColli*, int, int, int, int);
void colliCopyShapes(CharaColli*, const ColliShape*);
void colliFree(CharaColli*);
ColliShape* colliGetShapes(CharaColli*);
void colliShapeSetPos(ColliShape*, const Vec3f*);
void colliShapeMakeSphere(ColliShape*, const Vec3f*, int, f32);
void colliShapeMakeCylinder(ColliShape*, const Vec3f*, int, f32, f32);
void colliPlaceShapes(CharaColli*, const Vec3f*, const ColliShape*);
void colliShapeSetBit8(CharaColli*, int);
void colliShapeClearBit8(CharaColli*, int);
void colliRetire(CharaColli*);
void colliShapeSetTag(CharaColli*, int, u32);
void colliShapeSetRadius(ColliShape*, f32);
}

extern "C" {
ColliShape* colliGetShape(CharaColli*, int);
void colliEnable(CharaColli*);
void colliDisable(CharaColli*);
void colliShapeSetCylinder(ColliShape*);
int colliContactInGroup8(const ColliContact*);
void colliCalcBounds(ColliShape*, ColliShape*, int);
void colliSetBounds(CharaColli*, const ColliShape*);
int colliIsDisabled(CharaColli*);
void colliShapeSetBit10(CharaColli*, int);
void colliPlaceShapesBounds(CharaColli*, const Vec3f*, int, const ColliShape*, const Vec3f*);
void colliPlaceShapesRotY(CharaColli*, const Vec3f*, int, const ColliShape*);
void colliRefreshBounds(CharaColli*);
void colliBoundsOn(CharaColli*);
void colliShapeClearBit10(CharaColli*, int);
void colliClearContacts(CharaColli*);
int colliGetContactCount(CharaColli*);
int colliContactInGroup2(const ColliContact*);
void colliShapeSetBoxX(ColliShape*, f32);
void colliShapeSetBoxY(ColliShape*, f32);
void colliShapeSetBoxZ(ColliShape*, f32);
void colliShapeSetBoxRot(ColliShape*, int);
void colliBoundsOff(CharaColli*);
int colliContactInGroup1(const ColliContact*);
void colliAddMasks(CharaColli*, int, int, int);
void colliClearMasks(CharaColli*, int, int, int);
u32 colliGetOwner(CharaColli*);
int colliGetShapeCount(CharaColli*);
int colliInGroup(CharaColli*, int);
u8 colliGetGroup(CharaColli*);
void colliDrawShapes(CharaColli*);
ColliContact* colliGetContact(CharaColli*, int);
void colliSetList(CharaColli*, int);
TCharaColliCtrl* createTCharaColliCtrl();
}

extern "C" {
u32 colliContactOwner(const ColliContact*);
#define colliContactOwnerU16(contact) (((u16 (*)(ColliContact*))colliContactOwner)(contact))
int colliContactTag(ColliContact*);
int colliContactNoBit10(ColliContact*);
void colliShapeSetCylRadius(ColliShape*, f32);
int colliContactHasBit8(ColliContact*);
int colliContactHasBit40(ColliContact*);
void colliCopyShapesTagged(CharaColli*, const ColliShape*, u32);
void colliShapeMakeBox(ColliShape*, const Vec3f*, int, f32, f32, f32, int);
}

extern "C" {
ColliShape* colliContactShape(const ColliContact*);
void colliShapeSetSphere(ColliShape*);
void colliShapeSetCylHeight(ColliShape*, f32);
}

#include "engine/CharaColli.h"
extern "C" {
extern ColliColor sColliColorGreen;
extern ColliColor sColliColorYellow;
extern ColliColor sColliColorGrey;
extern CharaColli* gColliFree;
extern CharaColli* gColliRetired;
extern int gColliContactCount;
extern u32 gColliDebug;
}

#endif
