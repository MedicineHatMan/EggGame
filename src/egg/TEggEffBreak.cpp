#include "egg/TEggEffBreak.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "object/ObjAction.h"
#include "egg/ObjEgg.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "engine/Viewport.h"
#include "core/VecMath.h"
#include "core/TMainTask.h"
#include "egg/TEggTextureManager.h"
struct GjModel;
struct NJS_QUATERNION;
struct TEggTextureManager;

// 0x800401A0
// ObjEgg calls this when an egg breaks
extern "C" void createTEggEffBreak(f32 radius, const Vec3f* pos, const EggRot* rot, int id)
{
	new TEggEffBreak(radius, gTL_06, pos, rot, id);
}

struct BreakNode {
	u8 _00[4];
	void* mModel;      // _04
	Vec3f mPos;        // _08
	u8 _14[0x18];
	BreakNode* mChild; // _2C
	BreakNode* mNext;  // _30
};

typedef char TEggEffBreak_size_check[(sizeof(TEggEffBreak) == 0x70) ? 1 : -1];
typedef char EggBreakPiece_size_check[(sizeof(EggBreakPiece) == 0x68) ? 1 : -1];

const char* gTEggEffBreakName = "TEggEffBreak";

// 0x80040208
TEggEffBreak::TEggEffBreak(f32 radius, TObject* parent, const Vec3f* pos, const EggRot* rot, int id)
    : TObject(parent)
{
	TEggTextureManager* mgr;
	void** exp;
	BreakNode* node;
	int count;
	EggBreakPiece* piece;
	int i;
	f32 scale;
	f32 mtx[12];
	Vec3f down;

	mId     = id;
	mRadius = radius;
	_00     = (u32)gTEggEffBreakName;
	mSize   = 0x70;
	mLife   = 0xB4;
	mTask   = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFrame  = 0;
	mgr     = getTEggTextureManager();
	if (mgr->acquire(id) == 0) {
		mAltTex = 0;
	} else {
		mAltTex = mgr->getImage(id);
	}
	exp     = (void**)findArcExport(gEggArc, "models");
	mModel  = exp[2];
	exp     = (void**)findArcExport(gEggArc, "texlists");
	mTexList = (NJS_TEXLIST*)exp[2];
	count   = 0;
	if (mModel != 0) {
		node = ((BreakNode*)mModel)->mChild;
		while (node != 0) {
			count++;
			node = node->mNext;
		}
	}
	mCount  = (u16)count;
	mPieces = new EggBreakPiece[count];
	if (count != 0) {
		scale = 0.2f * mRadius;
		mtxSetTransV(mtx, pos);
		mtxConcatQuat((f32*)mtx, (const f32*)mtx, (const f32*)rot);
		mtxScale(scale, scale, scale, mtx, mtx);
		piece = mPieces;
		node  = ((BreakNode*)mModel)->mChild;
		for (i = 0; i < count; i++, piece++) {
			piece->mModel = node->mModel;
			mtxMultVec(mtx, &node->mPos, &piece->mPos);
			mtxMultVecSR(mtx, &node->mPos, &down);
			vecNormalize(&down, &down);
			vecScale(250.0f, &piece->mVel, &down);
			piece->mRot = *rot;
			down.x      = 0.0f;
			down.y      = -1.0f;
			down.z      = 0.0f;
			njOuterProduct(&piece->mVel, &down, &down);
			vecScale(1.0f / mRadius, &piece->mSpin, &down);
			node = node->mNext;
			mtxSetTransV(piece->mMtx, &piece->mPos);
			mtxConcatQuat((f32*)piece->mMtx, (const f32*)piece->mMtx, (const f32*)&piece->mRot);
			mtxScale(scale, scale, scale, piece->mMtx, piece->mMtx);
		}
	}
}

// 0x800404E0
EggBreakPiece::EggBreakPiece() { }

// 0x800404E4
TEggEffBreak::~TEggEffBreak()
{
	TEggTextureManager* mgr;

	if (mAltTex != 0) {
		mgr = getTEggTextureManager();
		mgr->release(mId);
	}
	delete[] mPieces;
	removeDrawTask(mTask);
}

// 0x80040578
static inline EggRot eggRotMul(EggRot rot, const EggRot* by)
{
	njMultiQuaternion((const NJS_QUATERNION*)&rot, (const NJS_QUATERNION*)by, (NJS_QUATERNION*)&rot);
	return rot;
}

static inline EggRot eggRotScale(EggRot rot, f32 scale)
{
	quatScale((Quat*)&rot, scale);
	return rot;
}

void TEggEffBreak::draw()
{
	EggBreakPiece* piece;
	int i;
	f32 scale;
	Vec3f down;
	Vec3f step;

	piece  = mPieces;
	scale  = 0.2f * mRadius;
	down.x = 0.0f;
	down.y = -400.0f;
	down.z = down.x;
	for (i = 0; i < mCount; i++, piece++) {
		if (mFrame > 5) {
			vecScaleAdd(0.016666668f, &piece->mVel, &down, &piece->mVel);
			vecScaleAdd(0.016666668f, &piece->mPos, &piece->mVel, &piece->mPos);
			vecScale(0.016666668f, &step, &piece->mSpin);
			EggRot made;
			made.mW    = 0.0f;
			made.mAxis.v[0] = step.x;
			made.mAxis.v[1] = step.y;
			made.mAxis.v[2] = step.z;
			EggRot half = eggRotScale(eggRotMul(made, &piece->mRot), 0.5f);
			quatAdd((Quat*)&piece->mRot, (const Quat*)&half);
			quatNormalize((Quat*)&piece->mRot);
			vecScale(0.97f, &piece->mVel, &piece->mVel);
		}
		mtxSetTransV(piece->mMtx, &piece->mPos);
		mtxConcatQuat((f32*)piece->mMtx, (const f32*)piece->mMtx, (const f32*)&piece->mRot);
		mtxScale(scale, scale, scale, piece->mMtx, piece->mMtx);
	}
	if (--mLife == 0) {
		mFlags |= 1;
	}
	mFrame += 1;
}

// 0x80040754
void TEggEffBreak::render()
{
	struct TexBlock {
		void* mNames;
		u32 _04;
		u32 _08;
		u32 _0C;
		void* mImage;
		u32 _14;
		u32 _18;
		u32 _1C;
	};
	TexBlock tex;
	NJS_TEXNAME* entry;
	EggBreakPiece* piece;
	int i;

	if (mFrame > 0x1E && (mFrame & 1) == 0) {
		return;
	}
	tex.mNames = &tex._08;
	if (mAltTex == 0) {
		tex.mImage = mTexList->textures->texaddr;
	} else {
		tex.mImage = mAltTex;
	}
	entry   = mTexList->textures;
	tex._14 = (u32)entry[1].filename;
	tex._18 = entry[1].attr;
	tex._1C = (u32)entry[1].texaddr;
	((void (*)(int, void*))pushCullMode)(0, entry);
	njSetTexture(&tex);
	piece = mPieces;
	for (i = 0; i < mCount; i++, piece++) {
		njPushMatrixEx();
		njMultiMatrix(0, piece->mMtx);
		gjDrawModel((GjModel*)piece->mModel);
		njPopMatrixEx();
	}
	popCullMode();
}
