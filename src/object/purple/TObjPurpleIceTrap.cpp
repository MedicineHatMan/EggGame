#include "object/purple/TObjPurpleIceTrap.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "object/ObjScript.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "object/ObjAction.h"
#include "engine/TParticle.h"
#include "ui/AeArc.h"
#include "object/orange/TObjOrangeBalloon.h"
#include "object/TObjLauncher.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "core/VecMath.h"
#include "stage/SetSlot.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;
struct SetLink;

// 0x802C4310
ObjClassName gTObjPurpleIceTrapName = { "TObjPurpleIceTrap", 0 };

// A particle generator mPos is where the spray is drawn
typedef char TObjPurpleIceTrap_size_check[(sizeof(TObjPurpleIceTrap) == 0xF4) ? 1 : -1];

// 0x80125D94
extern "C" TObject* createTObjPurpleIceTrap(TObject* parent, SetDesc* desc)
{
	return new TObjPurpleIceTrap(parent, desc);
}

// 0x80125DE8
void TObjPurpleIceTrap::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);

	mEternal = CLAMP_RANGE(desc->getInt(0), 0, 1);
	mPower   = desc->getInt(1) + 0x19;
	mOnTime  = desc->getInt(2) + 0x78;
	mOffTime = desc->getInt(3) + 0xB4;
	placeShapes();

	Vec3f off;
	off.x = 0.0f;
	off.y = 0.0f;
	off.z = 2.0f;
	f32 mtx[12];
	mtxSetRotY(mtx, mRot.y);
	mtxRotateX(mtx, mtx, mRot.x);
	mtxMultVec(mtx, &off, &off);
	vecAdd(&off, &off, &mPos);
	mSpray->mPos = off;
	mSpray->setRot(mRot.x, mRot.y);
}

// 0x80125F68
void TObjPurpleIceTrap::debugPrint(void*)
{
	njPrintC(0xA0013, ":ETERNAL FLAG");
	njPrintC(0xA0014, "+25:POWER");
	njPrint(0xA0015, "+%d:ON TIME", 0x78);
	njPrint(0xA0016, "+%d:OFF_TIME", 0xB4);
}

// 0x80125FE4
int TObjPurpleIceTrap::onHit(const TObjAction* other, int, int* result)
{
	if (((TObjAction*)other)->getKind() != 3) {
		resetHitInfo((HitInfo*)result);
		((HitInfo*)result)->mDamage = mPower;
		return 1;
	}
	return 0;
}

// 0x80126048
TObjPurpleIceTrap::TObjPurpleIceTrap(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00       = (u32)gTObjPurpleIceTrapName.mName;
	mSize     = 0xF4;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape    = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xF6, 0xF6);
	mSpikes = colliCreate(7, 1);
	colliSetOwner(mSpikes, (u16)mObjId);
	colliSetMasks(mSpikes, 4, 0xFF, 0xFF, 0xFE);
	mSpray = particleGenById(this, &mPos, 0, 0xE9);
	initFromDesc(desc);
	vfn_74();
}

// 0x80126170
TObjPurpleIceTrap::~TObjPurpleIceTrap()
{
	if (mSpray != 0) {
		mSpray->mFlags |= 1;
		mSpray = 0;
	}
	colliRetire(mSpikes);
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x80126228
void TObjPurpleIceTrap::vfn_74()
{
	mTimer = 0;
	updateMatrix();
	mTrack.set(gGeObjCommon->mSet.mMotions[0], 1, 0.3f);
	mModel.setModel(getGeObjModels()[4]);
	mModel.mTexList   = getGeObjTextures()[1];
	mModel.mDrawState = getDefaultDrawState();
	mModel.mTrack     = &mTrack;
	setLinkFree((SetLink*)&mLink);

	if (mEternal != 0) {
		mSpray->mFlags &= ~4;
		colliGetShapes(mShape)->mFlags |= 8;
		colliEnable(mSpikes);
		mAct = 3;
	} else {
		mTrack.mLoop = 0;
		mTrack.mRate = 0.5f;
		mSpray->mFlags |= 4;
		colliGetShapes(mShape)->mFlags &= ~8;
		colliDisable(mSpikes);
		mAct = 0;
	}
}

// 0x80126338
void TObjPurpleIceTrap::placeShapes()
{
	f32 mtx[12];
	mtxSetRotY(mtx, mRot.y);
	mtxRotateX(mtx, mtx, mRot.x);
	colliShapeMakeSphere(colliGetShapes(mShape), &mPos, 0, 20.0f);

	Vec3f step;
	step.x = 0.0f;
	step.y = 0.0f;
	step.z = 12.0f;
	mtxMultVec(mtx, &step, &step);

	Vec3f pos = mPos;
	ColliShape shapes[7];
	for (int i = 0; i < 7; i++) {
		njAddVector(&pos, &step);
		colliShapeMakeSphere(&shapes[i], &pos, 0, 12.0f);
		shapes[i].mFlags |= 0x18;
	}
	colliCopyShapes(mSpikes, shapes);
	colliRefreshBounds(mSpikes);
}

// 0x80126430
void TObjPurpleIceTrap::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateZYX((f32*)mMatrix, (const f32*)mMatrix, (const int*)&mRot);
}

// 0x80126474
void TObjPurpleIceTrap::draw()
{
	if (!isInRange(this, &mPos, 900.0f)) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		mSpray->mFlags &= ~4;
		mSpray->mFlags &= ~0x10;
		updateMatrix();
	} else {
		switch (mAct) {
		case 0:
			open();
			break;
		case 1:
			hold();
			break;
		case 2:
			cool();
			break;
		case 3:
			eternal();
			break;
		}
	}
}

// 0x80126554
void TObjPurpleIceTrap::open()
{
	if (mTrack.isDone()) {
		sePlay3D(0x237, &mPos);
		colliGetShapes(mShape)->mFlags |= 8;
		colliEnable(mSpikes);
		mSpray->mFlags &= ~4;
		mTrack.mLoop = 1;
		mTrack.mRate = 1.0f;
		mAct         = 1;
	}
	mTrack.update();
}

// 0x801265E0
void TObjPurpleIceTrap::hold()
{
	if (++mTimer > mOnTime) {
		mTimer = 0;
		colliGetShapes(mShape)->mFlags &= ~8;
		colliDisable(mSpikes);
		mSpray->mFlags |= 4;
		mTrack.mLoop = 0;
		mTrack.mRate = 0.5f;
		mAct         = 2;
	}
	mTrack.update();
}

// 0x80126674
void TObjPurpleIceTrap::cool()
{
	if (!mTrack.isDone()) {
		mTrack.update();
		return;
	}
	if (++mTimer > mOffTime) {
		mTimer        = 0;
		mTrack.mFrame = 0.0f;
		mTrack.mRate  = 0.5f;
		mAct          = 0;
	}
}

// 0x801266EC
void TObjPurpleIceTrap::eternal()
{
	mTrack.update();
}

// 0x80126710
void TObjPurpleIceTrap::vfn_10()
{
	sIceTrapVisible = cameraIsVisible(800.0f, 96.0f, &mPos, 0);
	if (sIceTrapVisible != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8012677C
void TObjPurpleIceTrap::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
}

// One node of the balloon's octree
struct TrapNode {
	u16 mLeaf;     // _00
	u16 mCount;    // _02
	u16* mIndices; // _04
	u16 mChild[8]; // _08
};

struct TrapTree {
	Vec3f* mPoints;   // _00
	void* _04;        // _04
	TrapNode* mNodes; // _08
	f32 mMinX;        // _0C
	f32 mMinY;        // _10
	f32 mMinZ;        // _14
	f32 mMaxX;        // _18
	f32 mMaxY;        // _1C
	f32 mMaxZ;        // _20
};

struct TrapBox {
	f32 mMinX;
	f32 mMinY;
	f32 mMinZ;
	f32 mMaxX;
	f32 mMaxY;
	f32 mMaxZ;
};

// 0x801267C0
static void walkTrapNodes(f32 minX, f32 minY, f32 minZ, f32 maxX, f32 maxY, f32 maxZ, TrapTree* tree, int index, TrapBox* box, TrapHitFn fn,
                          void* arg)
{
	f32 bMinX      = box->mMinX;
	f32 bMinY      = box->mMinY;
	f32 bMinZ      = box->mMinZ;
	f32 bMaxX      = box->mMaxX;
	f32 bMaxY      = box->mMaxY;
	f32 bMaxZ      = box->mMaxZ;
	TrapNode* node = (TrapNode*)((u8*)tree->mNodes + index * 0x18);
	if (bMaxX < minX || bMaxY < minY || bMaxZ < minZ || bMinX > maxX || bMinY > maxY || bMinZ > maxZ) {
		return;
	}

	if (node->mLeaf == 1) {
		u16* ids;
		int n = node->mCount;
		if (n == 0) {
			return;
		}
		ids = node->mIndices;
		for (; n != 0; n--, ids++) {
			Vec3f* point = &tree->mPoints[*ids];
			if (point->x >= bMinX && point->y >= bMinY && point->z >= bMinZ && point->x <= bMaxX && point->y <= bMaxY
			    && point->z <= bMaxZ) {
				fn(*ids, arg);
			}
		}
		return;
	}

	f32 midX = gBalloonRandOffset * (minX + maxX);
	f32 midY = gBalloonRandOffset * (minY + maxY);
	f32 midZ = gBalloonRandOffset * (minZ + maxZ);
	walkTrapNodes(minX, minY, minZ, midX, midY, midZ, tree, node->mChild[0], box, fn, arg);
	walkTrapNodes(minX, minY, midZ, midX, midY, maxZ, tree, node->mChild[1], box, fn, arg);
	walkTrapNodes(midX, minY, minZ, maxX, midY, midZ, tree, node->mChild[2], box, fn, arg);
	walkTrapNodes(midX, minY, midZ, maxX, midY, maxZ, tree, node->mChild[3], box, fn, arg);
	walkTrapNodes(minX, midY, minZ, midX, maxY, midZ, tree, node->mChild[4], box, fn, arg);
	walkTrapNodes(minX, midY, midZ, midX, maxY, maxZ, tree, node->mChild[5], box, fn, arg);
	walkTrapNodes(midX, midY, minZ, maxX, maxY, midZ, tree, node->mChild[6], box, fn, arg);
	walkTrapNodes(midX, midY, midZ, maxX, maxY, maxZ, tree, node->mChild[7], box, fn, arg);
}

// 0x80126B4C
extern "C" void queryTrapNodes(TrapTree* tree, TrapBox* box, TrapHitFn fn, void* arg)
{
	walkTrapNodes(tree->mMinX, tree->mMinY, tree->mMinZ, tree->mMaxX, tree->mMaxY, tree->mMaxZ, tree, 0, box, fn, arg);
}
