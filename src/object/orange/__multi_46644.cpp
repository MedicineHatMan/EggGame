#include "object/orange/TObjOrangeBattery.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "object/ObjScript.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "object/ObjAction.h"
#include "camera/TCameraCtrl.h"
#include "sound/Sound3DPlay.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "core/Pad.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "stage/SetSlot.h"
#include "object/TObjKey00.h"
#include "core/VecMath.h"
#include "object/ObjModel.h"
struct SetLink;
struct MeshColli;
struct MeshData;
struct MeshObject;
struct ColliShape;
struct DrawStateSlot;
struct CharaColli;

// TObjOrangeBattery, 0x80120610-0x801216E4. Vtable 0x8025E0F8.
// Striking the sphere (hit flag 6) starts MOTION_BATTERY. The two bolts track a node of that motion until
// the frame runs out, then the switch is set and the open node stays drawn. TObjDivineVoice starts at 0x801216E4.

extern "C" {

// 0x802C42D8
ObjClassName gTObjOrangeBatteryName = { "TObjOrangeBattery", 0 };
}

typedef char TObjOrangeBattery_size_check[(sizeof(TObjOrangeBattery) == 0x3FC) ? 1 : -1];
typedef char BatteryBolt_size_check[(sizeof(BatteryBolt) == 0x114) ? 1 : -1];

// 0x80120610
extern "C" TObject* createTObjOrangeBattery(TObject* parent, SetDesc* desc)
{
	return new TObjOrangeBattery(parent, desc);
}

// 0x80120664
TObjOrangeBattery::TObjOrangeBattery(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00        = (u32)gTObjOrangeBatteryName.mName;
	mSize      = 0x3FC;
	mModelData = findGeObjModel("MODEL_BATTERY");
	mTex       = findGeObjData("TLS_MODEL_BATTERY");
	mMotion    = findGeObjData("MOTION_BATTERY");
	copyNodes(mModelData);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 0xFF, 0xFF, 0xD6);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	initFromDesc(desc);
	mMesh         = meshColliCreate();
	void** meshes = gGeObjCommon->mSet.mMeshes;
	meshColliSetData((MeshObject*)mMesh, (MeshData*)meshes[2]);
	meshColliSetMatrix((MeshColli*)mMesh, (const f32*)mMatrix);
	if (sBatteryOffsetAGuard == 0) {
		sBatteryOffsetA.x    = 0.0f;
		sBatteryOffsetA.y    = 22.814571f;
		sBatteryOffsetA.z    = -4.935901f;
		sBatteryOffsetAGuard = 1;
	}
	if (sBatteryOffsetBGuard == 0) {
		sBatteryOffsetB.x    = 56.215065f;
		sBatteryOffsetB.y    = 56.215065f;
		sBatteryOffsetB.z    = 56.215065f;
		sBatteryOffsetBGuard = 1;
	}
	Vec3f boxMax;
	Vec3f boxMin;
	vecAdd(&boxMax, &sBatteryOffsetA, &sBatteryOffsetB);
	vecSub(&boxMin, &sBatteryOffsetA, &sBatteryOffsetB);
	f32 box[6];
	boxAroundBox(box, mMatrix, &boxMin, &boxMax);
	meshColliSetBox((MeshObject*)mMesh, (const f32*)box);
	vfn_74();
}

// 0x8012087C
TObjOrangeBattery::~TObjOrangeBattery()
{
	meshColliFree(mMesh);
	removeDrawTask(mDrawTask);
	colliRetire(mShape);
	mShape = 0;
}

// 0x80120920
void TObjOrangeBattery::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mSwitch = (s16)desc->getInt(0);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	GjModel* model = mNodes[0].model;
	mCenter.x      = model->center.x;
	mCenter.y      = model->center.y;
	mCenter.z      = model->center.z;
	mtxMultVec(mMatrix, &mCenter, &mCenter);
	mRadius = mNodes[0].model->r;
	u8 shape[0x30];
	colliShapeMakeSphere((ColliShape*)shape, &mCenter, 0, mRadius);
	colliCopyShapes(mShape, (const ColliShape*)shape);
}

// 0x801209E0
void TObjOrangeBattery::debugPrint(void*)
{
	njPrintC(0xA0013, ":SW ID");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x80120A7C
void TObjOrangeBattery::vfn_74()
{
	mFrame = 0.0f;
	mtxIdentity(mAnim);
	mModel.setModel(mModelData);
	mModel.mTexList = mTex;
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mBolts[0].aim(&mEnds[0], &mEnds[1]);
	mBolts[1].aim(&mEnds[0], &mEnds[1]);
	mBolts[0].mSpread = 0.1f;
	mBolts[1].mSpread = 0.1f;
	mBolts[0].mWidth  = 0.5f;
	mBolts[1].mWidth  = 0.5f;
	mBolts[0].mColor  = 0xFFFFFF00;
	mBolts[1].mColor  = 0xFFFFFF40;
	setLinkFree((SetLink*)&mLink);
	if (keyBitIsSet(mSwitch) == 1) {
		mAct = 2;
	} else {
		mAct = 0;
	}
}

// 0x80120B44
// Three nodes of MODEL_BATTERY, then the two rest positions from the sibling chain.
void TObjOrangeBattery::copyNodes(NJS_OBJECT* model)
{
	NJS_OBJECT* body  = model->child;
	NJS_OBJECT* node  = body->child;
	mNodes[0]         = *node;
	node              = node->sibling;
	mNodes[1]         = *node;
	node              = node->sibling;
	mNodes[2]         = *node;
	mNodes[2].sibling = 0;
	mNodes[1].sibling = 0;
	mNodes[0].sibling = 0;

	NJS_OBJECT* end = body->sibling->child;
	mEnds[0].set(end->pos[0], end->pos[1], end->pos[2]);
	end = end->sibling;
	mEnds[1].set(end->pos[0], end->pos[1], end->pos[2]);
}

// 0x80120CF4
void TObjOrangeBattery::draw()
{
	if (objDebugEnabled(this) != 0) {
		mBolts[0].reset();
		mBolts[1].reset();
		return;
	}
	if (!isInRange(this, &mPos, 900.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateCharge();
		break;
	case 2:
		stateDone();
		break;
	}
}

// 0x80120DAC
void TObjOrangeBattery::stateWait()
{
	for (void* contact = *(void**)((u8*)mShape + 0x80); contact != 0; contact = *(void**)contact) {
		if (colliContactHasBit8((ColliContact*)contact) == 0) {
			continue;
		}
		TObjAction* other = findObjById(colliContactOwner((ColliContact*)contact));
		if (other == 0) {
			continue;
		}
		HitInfo hit;
		if (other->onHit(this, 0, (int*)&hit) == 0) {
			continue;
		}
		if (hit.mFlags.testBit(6) == 0) {
			continue;
		}
		sePlay3D(0x252, &mPos);
		padVibStart(0, (int)this, 0xFF, 0, 0x24);
		mAct = 1;
	}
}

// 0x80120E88
void TObjOrangeBattery::stateCharge()
{
	GjModel* model = mNodes[0].model;
	Vec3f origin;
	origin.x  = model->center.x;
	origin.y  = model->center.y + 15.0f;
	origin.z  = model->center.z;
	f32 reach = 1.5f * model->r;

	Vec3f dir;
	dir.x = 0.0f;
	dir.z = reach;
	dir.y = dir.x;
	f32 spin[12];
	int yaw = (int)(65536.0f * 3.0517578e-05f * (f32)(u32)rand());
	mtxSetRotY(spin, yaw);
	int pitch = (int)(16384.0f * 2.0f * (3.0517578e-05f * (f32)(u32)rand() - 0.5f));
	mtxRotateX(spin, spin, pitch);
	mtxMultVec(spin, &dir, &dir);
	vecAdd(&dir, &dir, &origin);
	mBolts[0].aim(&origin, &dir);

	dir.x = 0.0f;
	dir.z = reach;
	dir.y = dir.x;
	yaw   = (int)(65536.0f * 3.0517578e-05f * (f32)(u32)rand());
	mtxSetRotY(spin, yaw);
	pitch = (int)(16384.0f * 2.0f * (3.0517578e-05f * (f32)(u32)rand() - 0.5f));
	mtxRotateX(spin, spin, pitch);
	mtxMultVec(spin, &dir, &dir);
	vecAdd(&dir, &dir, &origin);
	mBolts[1].aim(&origin, &dir);

	mFrame += 0.5f;
	u32 length = *(u32*)((u8*)mMotion + 4);
	if (mFrame <= (f32)length) {
		njStartMotionObj(mMotion, mFrame);
		njSetNextMotionNodeEx();
		njSetNextMotionNodeEx();
		f32 pos[3];
		int ang[3];
		f32 scale[3];
		f32 quat[4];
		gjGetMotionNodeData((NJS_OBJECT*)mNodes, (f32*)pos, (s32*)ang, (f32*)scale, (f32*)quat);
		mtxFromQuat(mAnim, quat);
		mtxScale(scale[0], scale[1], scale[2], mAnim, mAnim);
	} else {
		mBolts[0].aim(&mEnds[0], &mEnds[1]);
		mBolts[1].aim(&mEnds[0], &mEnds[1]);
		setKeyBit(mSwitch);
		mAct = 2;
	}
}

// 0x8012115C
void TObjOrangeBattery::stateDone()
{
	mBolts[0].reset();
	mBolts[1].reset();
}

// 0x80121194
void TObjOrangeBattery::vfn_10()
{
	Vec3f screen;
	if (cameraIsVisible(800.0f, mRadius, &mCenter, &screen) != 0) {
		f32 fade = (300.0f + screen.z) / 100.0f;
		if (fade < 0.5f) {
			fade = 0.5f;
		}
		mBolts[0].mWidth    = fade;
		mBolts[1].mWidth    = fade;
		mDrawTask->mSortKey = drawTaskDepthKey(screen.z);
		mDrawTask->mFlags &= 0xFFFE;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80121238
void TObjOrangeBattery::render()
{
	njSetTexture(mTex);
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	if (mAct == 1) {
		njMultiMatrix(0, mAnim);
	}
	if (mAct != 0) {
		mBolts[0].draw();
		mBolts[1].draw();
	}
	beginDrawState(getDrawStateSlot(), getDefaultDrawState());
	gjDrawObject(&mNodes[0]);
	endDrawState(getDrawStateSlot());

	ObjDrawState state;
	state.mFlags = 0x100;
	state.mBlend = 1;
	beginDrawState(getDrawStateSlot(), &state);
	if (mAct == 2) {
		gjDrawObject(&mNodes[1]);
	} else {
		gjDrawObject(&mNodes[2]);
	}
	endDrawState(getDrawStateSlot());
	njPopMatrixEx();
}

// 0x80121324
struct BoltVecBlock {
	f32 v[3];
};

BatteryBolt::BatteryBolt()
{
	*(BoltVecBlock*)&mFrom = *(BoltVecBlock*)&gVecZero;
	mTo.x                  = 1.0f;
	mTo.y                  = 0.0f;
	mTo.z                  = 0.0f;
	mSpread                = 0.4f;
	mWidth                 = 1.0f;
	mColor                 = 0xFFFFFFFF;
	mtxIdentity(mMatrix);
	reset();
}

// 0x801213A4
BatteryBolt::~BatteryBolt()
{
}

// 0x801213E0
void BatteryBolt::aim(const Vec3f* from, const Vec3f* to)
{
	mFrom = *from;
	mTo   = *to;
	Vec3f gap;
	vecSub(&gap, &mTo, &mFrom);
	f32 len = njScalor(&gap);
	gjScaleVector(1.0f / len, &gap, &gap);
	f32 pitch = atan(gap.y);
	int yaw   = vecAngleXZ(&gap);
	mtxSetTransV(mMatrix, &mFrom);
	mtxRotateY(mMatrix, mMatrix, yaw);
	mtxRotateX(mMatrix, mMatrix, (int)(10430.38043493439 * pitch));
	mtxScale(len, len, len, mMatrix, mMatrix);
}

// 0x801214F8
// Point 0 sits on the start and point 15 on the end. The fourteen between are shaken sideways.
void BatteryBolt::reset()
{
	f32 z         = sBatteryStep;
	f32 step      = z;
	mPoints[0].x  = 0.0f;
	mPoints[0].y  = 0.0f;
	mPoints[0].z  = 0.0f;
	mPoints[15].x = 0.0f;
	mPoints[15].y = 0.0f;
	mPoints[15].z = 1.0f;
	f32 step      = sBatteryStep;
	f32 z         = step;
	for (u16 i = 1; i < 15; i++) {
		mPoints[i].x = mSpread * (2.0f * (3.0517578e-05f * (f32)(int)rand() - 0.5f));
		mPoints[i].y = mSpread * (2.0f * (3.0517578e-05f * (f32)(int)rand() - 0.5f));
		mPoints[i].z = z;
		z += step;
	}
}

// 0x80121640
void BatteryBolt::draw()
{
	njColorBlendingMode(0, 10);
	njColorBlendingMode(1, 10);
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	njDrawLineExStart(mWidth, mColor, 1);
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
	njDrawLine3DExSetStrip(mPoints, 0x10);
	njDrawLineExEnd();
	njPopMatrixEx();
	gjGXSetBlendMode(0);
	njColorBlendingMode(0, 8);
	njColorBlendingMode(1, 6);
}
