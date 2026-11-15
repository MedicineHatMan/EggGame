#include "engine/WaterSurface.h"
struct DrawTask;
#include "object/ObjAction.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "sound/SoundHandle.h"
#include "object/ObjCommon.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "engine/TParticle.h"
#include "object/TObject.h"
#include "sound/SoundCtrl.h"
#include "stage/SetSlot.h"
#include "object/green/TObjGreenWaterfall.h"
#include "Ninja/ninja.h"
struct CharaColli;
struct SetLink;

struct WaterfallContact {
	WaterfallContact* mNext; // _00
	u8 _04[8];
	Vec3f mPos; // _0C
};

f32 gGreenWaterfallSetRange = 3000.0f; // 0x802C45D0

static f32 sDrawDist = gGreenWaterfallSetRange;         // 0x802C5940
static f32 sKeepDist = gGreenWaterfallSetRange + 50.0f; // 0x802C5944

typedef char TObjGreenWaterfall_size_check[(sizeof(TObjGreenWaterfall) == 0x13C) ? 1 : -1];

// 0x80186190
extern "C" TObject* createTObjGreenWaterfall(TObject* parent, SetDesc* desc)
{
	return new TObjGreenWaterfall(parent, desc);
}

const char* gTObjGreenWaterfallName = "TObjGreenWaterfall";

// 0x801861E4
TObjGreenWaterfall::TObjGreenWaterfall(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00           = (u32)gTObjGreenWaterfallName;
	mSize         = 0x13C;
	mModel        = (WaterfallNode*)getGeObjModels()[1];
	mTexList      = getGeObjTextures()[1];
	mBreakModel   = getGeObjModels()[2];
	mBreakTexList = getGeObjTextures()[2];
	mWater.setModel(mModel, mTexList);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	mShape    = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 0xFF, 0xFE, 0xDE);
	initFromDesc(desc);
	vfn_74();
}

// 0x80186310
TObjGreenWaterfall::~TObjGreenWaterfall()
{
	seStop3D(&mSound);
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x8018639C
void TObjGreenWaterfall::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mWater.initFromDesc(desc);

	ColliShape* shape   = colliGetShape(mShape, 0);
	WaterfallNode* node = mModel->mChild;
	Vec3f at;
	at.set(node->mPos.x, node->mPos.y, node->mPos.z);
	colliShapeMakeBox(shape, &at, node->mRotY, node->mSizeX, node->mSizeY, node->mSizeZ, 0);
	shape->mFlags |= 8;
}

// 0x80186430
void TObjGreenWaterfall::debugPrint(void*)
{
	njPrintC(0xA0013, "");
	mWater.debugPrint();
}

// 0x80186470
void TObjGreenWaterfall::vfn_74()
{
	for (WaterfallNode* node = mModel->mChild->mNext; node != 0; node = node->mNext) {
		Vec3f at;
		at.set(node->mPos.x, node->mPos.y, node->mPos.z);
		particleGenById(this, &at, 1, 3);
		if (node->mNext == 0) {
			sePlay3DHeld(1000.0f, &mSound, 0x274, 0x64, &at);
		}
	}
	mDrawFlags.mBits |= 1;
	mAct = 0;
}

// 0x80186518
int TObjGreenWaterfall::onHit(const TObjAction* other, int, int* result)
{
	if (((TObjAction*)other)->getKind() == 0) {
		HitInfo* hit = (HitInfo*)result;
		resetHitInfo(hit);
		hit->mDamage = 1;
		return 1;
	}
	return 0;
}

// 0x80186578
void TObjGreenWaterfall::wake()
{
	objDeleteTree(this);
}

// 0x80186598
void TObjGreenWaterfall::draw()
{
	switch (mAct) {
	case 0:
		updateFlow();
		break;
	case 1:
		updateBroken();
		break;
	}
}

// 0x801865DC
// A hit carrying flag 5 breaks it
void TObjGreenWaterfall::updateFlow()
{
	mWater.update();

	WaterfallContact* contact = (WaterfallContact*)mShape->mContacts;
	while (contact != 0) {
		if (colliContactHasBit8((ColliContact*)contact) != 0) {
			TObjAction* other = findObjById(colliContactOwner((ColliContact*)contact));
			if (other != 0) {
				HitInfo hit;
				if (other->onHit(this, 0, (int*)&hit) != 0 && (int)hit.mFlags.testBit(5) != 0) {
					colliDisable(mShape);
					wake();
					mDrawFlags.mBits &= ~1;

					SetRot rot;
					rot.x = 0;
					rot.y = 0;
					rot.z = 0;
					shatterModelRot(0, &contact->mPos, &gVecZero, &rot, (NJS_OBJECT*)mBreakModel, mBreakTexList, 1.0f, 0.1f);
					seStop3D(&mSound);
					sePlay(0x270);
					mAct = 1;
					return;
				}
			}
		}
		contact = contact->mNext;
	}
}

// 0x80186704
void TObjGreenWaterfall::updateBroken()
{
	setLinkForever((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x80186740
void TObjGreenWaterfall::vfn_10()
{
	if (mWater.isVisible() != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8018679C
void TObjGreenWaterfall::render()
{
	if ((mDrawFlags.mBits & 1) != 0) {
		mWater.render();
	}
}
