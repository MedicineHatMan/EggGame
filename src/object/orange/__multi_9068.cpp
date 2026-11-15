#include "object/orange/TObjOrangeMirrorTrap.h"
#include "engine/DrawTask.h"
#include "engine/MeshColli.h"
#include "enemy/EnemyGroupWave.h"
#include "object/ObjScript.h"
#include "egg/ObjEgg.h"
#include "object/ObjArchive.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/DrawState.h"
#include "stage/TStageOrange.h"
#include "object/ObjAction.h"
#include "engine/GenTex.h"
#include "stl/math.h"
#include "object/TObjLauncher.h"
#include "engine/GroundQuery.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "core/Pad.h"
#include "object/orange/TObjOrangeMapMirror.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "stage/SetSlot.h"
#include "sound/SoundCtrl.h"
#include "core/VecMath.h"
#include "core/TMainTask.h"
struct DrawTask;
struct MeshColli;
struct MeshData;
struct SetLink;
struct MeshObject;
struct ColliShape;
struct CharaColli;
struct CamView;

// TObjOrangeMirrorTrap, 0x8012766C-0x80129F7C. Vtable 0x8025E564.
// One file with TMirrorBreaker (0x80128744-0x80129DA4, below): they share the sdata2 pool from 0x802C9FF8, and the
// static init at 0x80129E44 fills the trap's collision mesh (0x8025E534) after the breaker's code. The trap list
// gMirrorTraps (0x80282844) and its add/remove come last.

extern "C" {

// 0x802C4320
const char* gTObjOrangeMirrorTrapName = "TObjOrangeMirrorTrap";
// 0x802C4324. Defined here, ahead of the debug strings: its text follows the trap's name in .rodata.
const char* gTMirrorBreakerName = "TMirrorBreaker";
u8 sMirrorTrapFaceOrder[]       = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
}

// The usual inline sqrtf: three Newton steps on frsqrte.
static inline f32 sqrtfInline(f32 x)
{
	static const f64 _half  = .5;
	static const f64 _three = 3.0;
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		y         = (f32)(x * guess);
		return y;
	}
	return x;
}

// 0x8025E464, the trap face: a quad 120 wide and 120 tall.
Vec3f sMirrorVerts[4] = {
	{ -60.0f, 0.0f, 0.0f },
	{ -60.0f, 120.0f, 0.0f },
	{ 60.0f, 120.0f, 0.0f },
	{ 60.0f, 0.0f, 0.0f },
};

MirrorTrapList gMirrorTraps;

f32 sMirrorTriRadius = 0.5f * sqrtfInline(2.0f); // 0x802C5618

// 0x8025E494, both sides of the quad. The radii are filled by the static init.
MeshTri sMirrorTris[4] = {
	{ { 0, 2, 1 }, 0, 0x221, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.5f, 0.0f }, sMirrorTriRadius },
	{ { 0, 1, 2 }, 0, 0x221, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.5f, 0.0f }, sMirrorTriRadius },
	{ { 0, 3, 2 }, 0, 0x221, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.5f, 0.0f }, sMirrorTriRadius },
	{ { 0, 2, 3 }, 0, 0x221, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.5f, 0.0f }, sMirrorTriRadius },
};

// 0x8025E534
MeshData sMirrorMesh = { sMirrorVerts, 4, sMirrorTris, 4 };

typedef char TObjOrangeMirrorTrap_size_check[(sizeof(TObjOrangeMirrorTrap) == 0x170) ? 1 : -1];
typedef char MirrorTrapNode_size_check[(sizeof(MirrorTrapNode) == 0x10) ? 1 : -1];

static void gxFifo(u32 word)
{
	*(volatile u32*)0xCC008000 = word;
}

// 0x8012766C
extern "C" TObject* createTObjOrangeMirrorTrap(TObject* parent, SetDesc* desc)
{
	return new TObjOrangeMirrorTrap(parent, desc);
}

// 0x801276C0
// One rolled byte per kind. suitMatches compares set int 3 with the byte for this trap's kind.
void rollMirrorTrapAnswers()
{
	sMirrorTrapFaceOrder[0] = (u8)(int)(3.0517578e-05f * (f32)rand() + 0.5f);
	sMirrorTrapFaceOrder[1] = (u8)(int)(2.0f * (3.0517578e-05f * (f32)rand()) + 0.5f);
	sMirrorTrapFaceOrder[2] = (u8)(int)(3.0f * (3.0517578e-05f * (f32)rand()) + 0.5f);
}

// 0x801277A8
TObjOrangeMirrorTrap::TObjOrangeMirrorTrap(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	mNode.mNext  = 0;
	mNode.mPrev  = 0;
	mNode.mList  = 0;
	_00          = (u32)gTObjOrangeMirrorTrapName;
	mSize        = 0x170;
	mNode.mOwner = this;
	gMirrorTraps.add(&mNode);
	mModelA   = getGeObjModels()[10];
	mModelB   = getGeObjModels()[9];
	mTexA     = getGeObjTextures()[11];
	mTexB     = getGeObjTextures()[10];
	mTexC     = getGeObjTextures()[3];
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape    = colliCreate(1, 1);
	colliSetOwner(mShape, 0xFFFF);
	colliSetMasks((CharaColli*)mShape, 4, 0xFF, 0xFF, 0xD7);
	mMesh[0] = meshColliCreate();
	meshColliSetData((MeshObject*)mMesh[0], &sMirrorMesh);
	mMesh[1] = meshColliCreate();
	meshColliSetData((MeshObject*)mMesh[1], (MeshData*)gGeObjCommon->mSet.mMeshes[1]);
	mMesh[2] = meshColliCreate();
	meshColliSetData((MeshObject*)mMesh[2], (MeshData*)gGeObjCommon->mSet.mMeshes[0]);
	initFromDesc(desc);
	vfn_74();
}

// 0x80127928
TObjOrangeMirrorTrap::~TObjOrangeMirrorTrap()
{
	meshColliFree(mMesh[2]);
	meshColliFree(mMesh[1]);
	meshColliFree(mMesh[0]);
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
	gMirrorTraps.remove(&mNode);
}

// 0x801279C8
void TObjOrangeMirrorTrap::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mKind   = CLAMP_RANGE(desc->getInt(0), 0, 2);
	mGroup  = desc->getInt(1);
	mExtent = sMirrorTrapSixty;
	mAnswer = desc->getInt(3);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxTranslate((float*)mBody, (const float*)mMatrix, 0.0f, 0.0f, -5.0f);
	mtxTranslate((float*)mFace, (const float*)mMatrix, 0.0f, 0.0f, -20.0f);
	mtxTranslate((float*)mMark, (const float*)mMatrix, 0.0f, 0.0f, 2.5f);

	u8 shape[0x30];
	colliShapeMakeBox((ColliShape*)shape, &mPos, mRot.y, mExtent, mExtent, 10.0f, 1);
	*(u32*)(shape + 0x20) |= 0x10;
	colliCopyShapes(mShape, (const ColliShape*)shape);

	Vec3f half;
	half.x = sMirrorTrapOne;
	half.y = sMirrorTrapOne;
	half.z = sMirrorTrapOne;
	gjScaleVector(1.2f * mExtent, &half, &half);
	Vec3f hi = half;
	Vec3f lo;
	lo.x = -half.x;
	lo.y = -half.y;
	lo.z = -half.z;
	f32 placed[12];
	mtxSetTrans((float*)placed, mPos.x, mPos.y + mExtent, mPos.z);
	mtxMultVec(placed, &hi, &hi);
	mtxMultVec(placed, &lo, &lo);
	f32 box[6];
	boxAroundLine(box, &lo, &hi);
	for (int i = 0; i < 3; i++) {
		meshColliSetBox((MeshObject*)mMesh[i], (const f32*)box);
	}
	meshColliSetMatrix((MeshColli*)mMesh[0], (const f32*)mBody);
	meshColliSetMatrix((MeshColli*)mMesh[1], (const f32*)mFace);
	meshColliSetMatrix((MeshColli*)mMesh[2], (const f32*)mMark);
}

// 0x8025E458, the name of each kind, with where it stands.
const char* sMirrorTrapIdNames[] = {
	"A(X=1593.5)",
	"B(X=1985.0)",
	"C(X=1863.0)",
};

// 0x80127C5C
void TObjOrangeMirrorTrap::debugPrint(void*)
{
	njPrint(0xA0013, ":ID=%s", sMirrorTrapIdNames[mKind]);
	njPrintC(0xA0014, ":ENEMY-GROUP ID");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, ":LOCAL ID");
	njPrint(0xA0017, "+%.1f:SIZE", 60.0);
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x80127D28
void TObjOrangeMirrorTrap::vfn_74()
{
	mFaceScale = 100.0f;
	mtxTranslate((float*)mFace, (const float*)mMatrix, 0.0f, 100.0f, -20.0f);
	setLinkFree((SetLink*)&mLink);
	meshColliRefresh((MeshObject*)mMesh[1]);
	if (suitMatches() != 0) {
		meshColliRefresh((MeshObject*)mMesh[2]);
	}
	if ((u32)mLink.get()->getData() == 1) {
		meshColliRefresh((MeshObject*)mMesh[0]);
		colliDisable(mShape);
		mAct = 4;
	} else {
		meshClearState400((MeshObject*)mMesh[0]);
		mAct = 0;
	}
}

// 0x80127DD0
int TObjOrangeMirrorTrap::suitMatches()
{
	return (int)sMirrorTrapFaceOrder[mKind] == mAnswer;
}

// 0x80127DF0
void TObjOrangeMirrorTrap::shatter()
{
	Vec3f dir;
	dir.x = mExtent;
	dir.y = mExtent;
	dir.z = 0.0f;
	vecRotateY(&dir, &dir, mRot.y);
	EggHits hits;
	Vec3f at;
	at.set(mPos.x, 10.0f + mPos.y, mPos.z);
	if (meshColliProbe((EggHit*)&hits, &at, 0) != 0) {
		createTMirrorBreaker(gTL_05, &mPos, hits.mHits[0].mPos.y, &dir, 1);
	} else {
		createTMirrorBreaker(gTL_05, &mPos, mPos.y, &dir, 1);
	}
}

// 0x80127EC4
void TObjOrangeMirrorTrap::failOthers()
{
	MirrorTrapNode* node;
	node = gMirrorTraps.mHead;
	while (0 != node) {
		if (node->mOwner != this) {
			if (mKind == node->mOwner->mKind) {
				node->mOwner->failOne();
			}
		}
		node = node->mNext;
	}
}

// 0x80127F28
void TObjOrangeMirrorTrap::clearOthers()
{
	MirrorTrapNode* node;
	node = gMirrorTraps.mHead;
	while (0 != node) {
		if (node->mOwner != this) {
			if (mKind == node->mOwner->mKind) {
				node->mOwner->clearOne();
			}
		}
		node = node->mNext;
	}
}

// 0x80127F8C
void TObjOrangeMirrorTrap::failOne()
{
	if (mAct != 4) {
		colliDisable(mShape);
		meshClearState400((MeshObject*)mMesh[1]);
		mAct = 1;
	}
}

// 0x80127FD8
void TObjOrangeMirrorTrap::clearOne()
{
	if (mAct != 4) {
		shatter();
		meshColliRefresh((MeshObject*)mMesh[0]);
		colliDisable(mShape);
		mLink.get()->setData((void*)1);
		mAct = 4;
	}
}

// 0x80128034
void TObjOrangeMirrorTrap::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 700.0f)) {
		return;
	}
	switch (mAct) {
	case 0:
		stateIdle();
		break;
	case 1:
		stateGroup();
		break;
	case 2:
		stateWaitGroup();
		break;
	case 3:
		stateRecover();
		break;
	case 4:
		stateBroken();
		break;
	}
}

// 0x801280E8
void TObjOrangeMirrorTrap::stateIdle()
{
	for (void* contact = *(void**)((u8*)mShape + 0x80); contact != 0; contact = *(void**)contact) {
		if (colliContactHasBit8((ColliContact*)contact) == 0) {
			continue;
		}
		if (suitMatches() != 0) {
			mapMirrorDropSphere(mKind);
			clearOthers();
			clearOne();
		} else {
			clearOne();
			failOthers();
			sePlay(0x24B);
		}
	}
	grow();
}

// 0x80128184
void TObjOrangeMirrorTrap::stateGroup()
{
	int state = enemyGroupTakeStep((s16)mGroup, 0);
	switch (state) {
	case 2:
		colliEnable(mShape);
		meshColliRefresh((MeshObject*)mMesh[1]);
		mAct = 0;
		break;
	case 0:
	case 1:
		mAct = 2;
		break;
	}
	shrink();
}

// 0x80128200
void TObjOrangeMirrorTrap::stateWaitGroup()
{
	if (enemyGroupStepActive((s16)mGroup, 0) == 0) {
		mAct = 3;
	}
	shrink();
}

// 0x80128250
void TObjOrangeMirrorTrap::stateRecover()
{
	colliEnable(mShape);
	meshColliRefresh((MeshObject*)mMesh[1]);
	mAct = 0;
}

// 0x80128290
void TObjOrangeMirrorTrap::stateBroken()
{
	grow();
}

// 0x801282B0
void TObjOrangeMirrorTrap::shrink()
{
	if (mFaceScale > 0.0f) {
		mFaceScale -= 2.0f;
		if (mFaceScale < 0.0f) {
			mFaceScale = 0.0f;
		}
		mtxTranslate((float*)mFace, (const float*)mMatrix, 0.0f, mFaceScale, -20.0f);
	}
}

// 0x80128314
void TObjOrangeMirrorTrap::grow()
{
	if (mFaceScale < 100.0f) {
		mFaceScale += sMirrorTrapOne;
		if (mFaceScale > 100.0f) {
			mFaceScale = 100.0f;
		}
		mtxTranslate((float*)mFace, (const float*)mMatrix, 0.0f, mFaceScale, -20.0f);
	}
}

// 0x80128378
void TObjOrangeMirrorTrap::vfn_10()
{
	Vec3f at = mPos;
	at.y += mExtent;
	if (cameraIsVisible(700.0f, 1.5f * mExtent, &at, 0) != 0) {
		mDrawTask->mFlags &= 0xFFFE;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8012840C
void TObjOrangeMirrorTrap::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mFace);
	njSetTexture(mTexA);
	drawObjectDefault(mModelA);
	njPopMatrixEx();
	if (mirrorIsReflecting() == 0 && suitMatches() == 0) {
		njPushMatrixEx();
		njMultiMatrix(0, mMark);
		njSetTexture(mTexB);
		drawObjectDefault(mModelB);
		njPopMatrixEx();
	}
	if (mAct != 4) {
		drawMark();
	}
}

// 0x801284AC
void TObjOrangeMirrorTrap::drawMark()
{
	if (mirrorIsReflecting() != 0) {
		return;
	}
	pushGXCullMode(0);
	f32 view[12];
	mtxConcat(view, (f32*)gjGetMatrix(), mBody);
	GXLoadPosMtxImm(view, 0);
	GXLoadNrmMtxImm(view, 0);
	GXSetCurrentMtx(0);
	GXSetNumTevStages(1);
	GXSetNumChans(1);
	GXSetNumTexGens(2);
	GXSetNumIndStages(0);
	gjGXSetBlendMode(0);
	mirrorSetTexture();
	gjLoadTextureNum(0, 0, 0, 0);
	genTexLoadWrap(4, 1, 0);

	u8* cam = (u8*)camGetView(0);
	f32 fov = 0.75f * (360.0f * ((f32) * (s32*)(cam + 0x44) / 65536.0f));
	f32 light[12];
	C_MTXLightPerspective((float*)light, fov, *(f32*)(cam + 0x4C), 0.5f, -0.5f, 0.5f, 0.5f);
	f32 lit[12];
	mtxConcat(lit, light, view);
	GXLoadTexMtxImm(lit, 0x1E, 0);
	GXSetTexCoordGen2(0, 0, 0, 0x1E, 0, 0x7D);
	GXSetChanCtrl(4, 0, 0, 0, 0, 0, 2);
	GXSetChanMatColor(4, *(GXColor*)&sOrangeMirrorTrapClearColor);
	GXSetTevOrder(0, 0, 0, 4);
	GXSetTevOp(0, 0);
	GXClearVtxDesc();
	GXSetVtxDesc(9, 1);
	GXSetVtxDesc(0xD, 1);
	GXSetVtxAttrFmt(0, 9, 1, 4, 0);
	GXSetVtxAttrFmt(0, 0xD, 1, 4, 0);
	GXBegin(0x80, 0, 4);
	GX_FIFO_F32 = sMirrorTrapMinusSixty;
	GX_FIFO_F32 = sMirrorTrapOneTwenty;
	GX_FIFO_F32 = sMirrorTrapZero;
	GX_FIFO_F32 = sMirrorTrapZero;
	GX_FIFO_F32 = sMirrorTrapZero;
	GX_FIFO_F32 = sMirrorTrapMinusSixty;
	GX_FIFO_F32 = sMirrorTrapZero;
	GX_FIFO_F32 = sMirrorTrapZero;
	GX_FIFO_F32 = sMirrorTrapZero;
	GX_FIFO_F32 = sMirrorTrapOne;
	GX_FIFO_F32 = sMirrorTrapSixty;
	GX_FIFO_F32 = sMirrorTrapZero;
	GX_FIFO_F32 = sMirrorTrapZero;
	GX_FIFO_F32 = sMirrorTrapOne;
	GX_FIFO_F32 = sMirrorTrapOne;
	GX_FIFO_F32 = sMirrorTrapSixty;
	GX_FIFO_F32 = sMirrorTrapOneTwenty;
	GX_FIFO_F32 = sMirrorTrapZero;
	GX_FIFO_F32 = sMirrorTrapOne;
	GX_FIFO_F32 = sMirrorTrapZero;
	GXSetTevDirect(0);
	GXSetNumTexGens(0);
	GXSetNumChans(0);
	GXSetNumIndStages(0);
	popGXCullMode();
}

// ---- TMirrorBreaker. 0x80128744-0x80129DA4. Vtable 0x8025E544.
// A mirror breaking: 50 big, 100 middle and 300 small triangles spread over the box given, each with its own speed,
// falling and bouncing on a floor. The first two kinds carry the mirror's texture. It lasts 300 frames and fades over
// the last 60. Sound 0x24A plays at the centre.

/**
 * @size 0x44
 */
struct MirrorFrag {
	MirrorFrag();

	Vec3f mPos;       // _00
	Vec3f mVel;       // _0C
	Vec3f mCorner[3]; // _18
	f32 mU;           // _3C
	f32 mV;           // _40
};

typedef char MirrorFrag_size_check[(sizeof(MirrorFrag) == 0x44) ? 1 : -1];

/**
 * @size 0x3C
 */
class TMirrorBreaker : public TObject {
public:
	TMirrorBreaker(TObject* parent, const Vec3f* pos, const Vec3f* size, int raised, f32 floor);
	virtual ~TMirrorBreaker();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();

	void* mTask;        // _20
	MirrorFrag* mBig;   // _24, 50 of them
	MirrorFrag* mMid;   // _28, 100
	MirrorFrag* mSmall; // _2C, 300
	f32 mFloor;         // _30
	int mTime;          // _34
	s16 mAlpha;         // _38
};

typedef char TMirrorBreaker_size_check[(sizeof(TMirrorBreaker) == 0x3C) ? 1 : -1];

static inline f32 mirrorRand()
{
	return 3.0517578125e-05f * (f32)rand();
}
static inline f32 mirrorSigned()
{
	return 2.0f * (mirrorRand() - 0.5f);
}

#define MIRROR_RAND()   mirrorRand()
#define MIRROR_SIGNED() (2.0f * (MIRROR_RAND() - 0.5f))
#define MIRROR_CORNER(frags, i, j, corner)                                         \
	{                                                                              \
		f32 z = MIRROR_SIGNED();                                                   \
		f32 y = MIRROR_SIGNED();                                                   \
		frags[i].mCorner[j].set(corner * MIRROR_SIGNED(), corner * y, corner * z); \
	}
#define MIRROR_FILL(frags, count, speed, corner)                        \
	for (int i = 0; i < count; i++) {                                   \
		frags[i].mPos.x = size->x * MIRROR_SIGNED() + pos->x;           \
		frags[i].mPos.y = raise + (size->y * MIRROR_SIGNED() + pos->y); \
		frags[i].mPos.z = size->z * MIRROR_SIGNED() + pos->z;           \
		frags[i].mVel.x = speed * MIRROR_RAND() + -0.5f * speed;        \
		frags[i].mVel.y = speed * MIRROR_RAND() + -0.5f * speed;        \
		frags[i].mVel.z = speed * MIRROR_RAND() + -0.5f * speed;        \
		MIRROR_CORNER(frags, i, 0, corner)                              \
		MIRROR_CORNER(frags, i, 1, corner)                              \
		MIRROR_CORNER(frags, i, 2, corner)                              \
		frags[i].mU = 0.5f * MIRROR_RAND();                             \
		frags[i].mV = 0.5f * MIRROR_RAND();                             \
	}

static inline void mirrorFill(MirrorFrag* frags, int count, const Vec3f* pos, const Vec3f* size, f32 raise, f32 speed, f32 corner)
{
	for (int i = 0; i < count; i++) {
		MirrorFrag* frag = &frags[i];
		frag->mPos.x     = size->x * mirrorSigned() + pos->x;
		frag->mPos.y     = raise + (size->y * mirrorSigned() + pos->y);
		frag->mPos.z     = size->z * mirrorSigned() + pos->z;
		frag->mVel.x     = speed * mirrorRand() + -0.5f * speed;
		frag->mVel.y     = speed * mirrorRand() + -0.5f * speed;
		frag->mVel.z     = speed * mirrorRand() + -0.5f * speed;
		for (int j = 0; j < 3; j++) {
			f32 z              = mirrorSigned();
			f32 y              = mirrorSigned();
			frag->mCorner[j].x = corner * mirrorSigned();
			frag->mCorner[j].y = corner * y;
			frag->mCorner[j].z = corner * z;
		}
		frag->mU = 0.5f * mirrorRand();
		frag->mV = 0.5f * mirrorRand();
	}
}

static inline void mirrorStep(MirrorFrag* frags, int count, f32 floor, f32 grip, f32 bounce)
{
	for (int i = 0; i < count; i++) {
		MirrorFrag* frag = &frags[i];
		frag->mVel.y     = 0.98f * (-0.1f + frag->mVel.y);
		vecAdd(&frag->mPos, &frag->mPos, &frag->mVel);
		if (frag->mPos.y < floor) {
			frag->mPos.y = floor;
			frag->mVel.x *= grip;
			frag->mVel.y = bounce * -frag->mVel.y;
			frag->mVel.z *= grip;
		}
	}
}

// 0x80128744
extern "C" void createTMirrorBreaker(TObject* parent, const Vec3f* pos, f32 floor, const Vec3f* size, int raised)
{
	new TMirrorBreaker(parent, pos, size, raised, floor);
}

// 0x801287B0
TMirrorBreaker::TMirrorBreaker(TObject* parent, const Vec3f* pos, const Vec3f* size, int raised, f32 floor)
    : TObject(parent)
{
	mFloor = floor;
	mAlpha = 0xFF;
	_00    = (u32)gTMirrorBreakerName;
	mSize  = 0x3C;
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);

	f32 raise = (raised != 0) ? size->y : 0.0f;
	mBig      = new MirrorFrag[0x32];
	mMid      = new MirrorFrag[0x64];
	mSmall    = new MirrorFrag[0x12C];
	MIRROR_FILL(mBig, 0x32, 0.6f, 12.0f)
	MIRROR_FILL(mMid, 0x64, 0.8f, 4.0f)
	MIRROR_FILL(mSmall, 0x12C, 1.0f, 2.0f)
	mTime = 0x12C;

	Vec3f at = *pos;
	at.y     = pos->y + raise;
	sePlay3D(0x24A, &at);
	padVibStart(0, (int)this, 0xFF, 0, 0x23);
}

// 0x801293D8
MirrorFrag::MirrorFrag()
{
}

// 0x801293DC
TMirrorBreaker::~TMirrorBreaker()
{
	delete[] mSmall;
	delete[] mMid;
	delete[] mBig;
	removeDrawTask((DrawTask*)mTask);
}

// 0x80129480
#define MIRROR_STEP(frags, count, grip, bounce)                 \
	for (int i = 0; i < count; i++) {                           \
		frags[i].mVel.y = 0.98f * (-0.1f + frags[i].mVel.y);    \
		vecAdd(&frags[i].mPos, &frags[i].mPos, &frags[i].mVel); \
		if (frags[i].mPos.y < mFloor) {                         \
			frags[i].mPos.y = mFloor;                           \
			frags[i].mVel.x = grip * frags[i].mVel.x;           \
			frags[i].mVel.y = bounce * -frags[i].mVel.y;        \
			frags[i].mVel.z = grip * frags[i].mVel.z;           \
		}                                                       \
	}

void TMirrorBreaker::draw()
{
	MIRROR_STEP(mBig, 0x32, 0.92f, 0.4f)
	MIRROR_STEP(mMid, 0x64, 0.93f, 0.5f)
	MIRROR_STEP(mSmall, 0x12C, 0.94f, 0.6f)
	if (mTime < 0x3C) {
		mAlpha = (int)((f32)mAlpha - 4.25f);
		if (mAlpha < 0) {
			mAlpha = 0;
		}
	}
	if (mTime == 0) {
		mFlags |= 1;
	}
	mTime--;
}

// 0x80129754
void TMirrorBreaker::render()
{
	if (mirrorIsReflecting()) {
		return;
	}
	pushGXCullMode(0);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXSetNumTevStages(1);
	GXSetNumChans(1);
	GXSetNumTexGens(1);
	GXSetNumIndStages(0);
	GXSetZCompLoc(GX_TRUE);
	if (mAlpha == 0xFF) {
		gjGXSetBlendMode(0);
	} else {
		gjGXSetBlendMode(1);
	}
	mirrorSetTexture();
	gjLoadTextureNum(0, 0, 0, 0);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	u32 colour = (0x80808000) | (u8)mAlpha;
	GXSetTevKColor(GX_KCOLOR0, *(GXColor*)&colour);
	GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
	GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetAlphaCompare(GX_ALWAYS, 0xFF, GX_AOP_AND, GX_ALWAYS, 0xFF);

	volatile f32* fifo = (volatile f32*)0xCC008000;
	volatile u8* fifoB = (volatile u8*)0xCC008000;

#define MIRROR_VERT(FRAG, N, GREY, U, V)            \
	*fifo  = (FRAG)->mPos.x + (FRAG)->mCorner[N].x; \
	*fifo  = (FRAG)->mPos.y + (FRAG)->mCorner[N].y; \
	*fifo  = (FRAG)->mPos.z + (FRAG)->mCorner[N].z; \
	*fifoB = (GREY);                                \
	*fifoB = (GREY);                                \
	*fifoB = (GREY);                                \
	*fifo  = (U);                                   \
	*fifo  = (V)

	GXBegin(GX_TRIANGLES, GX_VTXFMT0, 0x96);
	for (int i = 0; i < 0x32; i++) {
		MirrorFrag* frag = &mBig[i];
		MIRROR_VERT(frag, 0, 0xFF, 0.0f, 0.0f);
		MIRROR_VERT(frag, 1, 0x7F, frag->mU, 0.0f);
		MIRROR_VERT(frag, 2, 0x7F, frag->mU, frag->mV);
	}
	GXBegin(GX_TRIANGLES, GX_VTXFMT0, 0x12C);
	for (int i = 0; i < 0x64; i++) {
		MirrorFrag* frag = &mMid[i];
		MIRROR_VERT(frag, 0, 0xFF, 0.0f, 0.0f);
		MIRROR_VERT(frag, 1, 0x7F, frag->mU, 0.0f);
		MIRROR_VERT(frag, 2, 0x7F, frag->mU, frag->mV);
	}
#undef MIRROR_VERT

	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	u32 white = 0xFFFFFFFF;
	GXSetChanMatColor(GX_COLOR0A0, *(GXColor*)&white);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	colour = (0x80808000) | (u8)mAlpha;
	GXSetTevKColor(GX_KCOLOR0, *(GXColor*)&colour);
	GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
	GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

	GXBegin(GX_TRIANGLES, GX_VTXFMT0, 0x384);
	for (int i = 0; i < 0x12C; i++) {
		MirrorFrag* frag = &mSmall[i];
		for (int j = 0; j < 3; j++) {
			*fifo = frag->mPos.x + frag->mCorner[j].x;
			*fifo = frag->mPos.y + frag->mCorner[j].y;
			*fifo = frag->mPos.z + frag->mCorner[j].z;
		}
	}
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumTexGens(0);
	GXSetNumChans(0);
	GXSetNumIndStages(0);
	popGXCullMode();
}

// ---- The trap list.

// 0x80129DA4
void MirrorTrapList::add(MirrorTrapNode* node)
{
	if (mHead == 0) {
		mTail = node;
		mHead = node;
	} else {
		node->mPrev  = mTail;
		mTail->mNext = node;
		mTail        = node;
	}
	node->mList = this;
	mCount++;
}

// 0x80129DE4
void MirrorTrapList::remove(MirrorTrapNode* node)
{
	if (node->mPrev != 0) {
		node->mPrev->mNext = node->mNext;
	} else {
		mHead = node->mNext;
	}
	if (node->mNext != 0) {
		node->mNext->mPrev = node->mPrev;
	} else {
		mTail = node->mPrev;
	}
	node->mNext = 0;
	node->mPrev = 0;
	node->mList = 0;
	mCount--;
}

// 0x80129E44 is the static init (gMirrorTraps, sMirrorTriRadius and sMirrorTris). The list's ctor,
// dtor and clear follow it.

// 0x80129EF4
MirrorTrapList::MirrorTrapList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x80129F08
MirrorTrapList::~MirrorTrapList()
{
	clear();
}

// 0x80129F50. Unlinks every node without freeing it.
void MirrorTrapList::clear()
{
	MirrorTrapNode* node = mHead;
	while (node != 0) {
		MirrorTrapNode* next = node->mNext;
		node->mList          = 0;
		node->mNext          = 0;
		node->mPrev          = 0;
		node                 = next;
	}
}
