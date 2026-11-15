#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "mode/TModeGallery.h"
#include "engine/TParticle.h"
#include "player/TPlyEffStSleep.h"
#include "object/TObjLauncher.h"
#include "engine/Color.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "io/Arc.h"
#include "object/ObjShadow.h"
#include "io/NinjaFile.h"
#include "Dolphin/OS/OSCache.h"
#include "egg/ObjEgg.h"
#include "ui/AeArc.h"
#include "core/Mem.h"
#include "engine/DrawState.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "engine/ActorMotion.h"
#include "object/TObjEggRing.h"
#include "stage/SetSlot.h"
#include "egg/TEggTextureManager.h"
#include "object/TObjKey00.h"
#include "object/TObjClosedEgg.h"
struct GjActor;
struct NJS_TEXLIST;
struct NJS_OBJECT;
struct TObjAction;
struct TEggTextureManager;

// 0x400) once that key is set the stage makes the real egg in its place
// After it come the gallery's eggs 0x801BB10C-0x801BBAC8 their archives the table of the 72 see TModeGallery.h and
// GalleryEgg which turns the one picked and runs the particles on its nodes

extern "C" {

// The shell's models and textures from gEggArc
NJS_OBJECT** gClosedEggModels;   // 0x802C5B10
NJS_TEXLIST** gClosedEggTexList; // 0x802C5B14 the arc's "texlists" export slot

// While GalleryEgg::update runs the nodes counted so far and the egg
int gGalleryNodeCount;          // 0x802C5B18
GalleryEgg* gGalleryEggCurrent; // 0x802C5B1C
}

// Each gallery egg's archive and what it holds
TArc* gGalleryEggArcs[GALLERY_EGG_COUNT];              // 0x80286908
GalleryEggAsset* gGalleryEggAssets[GALLERY_EGG_COUNT]; // 0x80286A28
// Where each of gGalleryFxNodes was this frame
f32 gGalleryFxMtx[5][12]; // 0x80286B48

typedef char TObjClosedEgg_size_check[(sizeof(TObjClosedEgg) == 0xC0) ? 1 : -1];

/** getRadiusV's radius An array so the read isn't folded into the literal pool the DOL has it ahead of the file's
 * literals apart from the 7.5f the other functions share */
extern const f32 gClosedEggRadius[1];
const f32 gClosedEggRadius[1] = { 7.5f };

// 0x801BAB2C
extern "C" void initClosedEgg()
{
	if (gEggArc != 0) {
		gClosedEggModels  = (NJS_OBJECT**)findArcExport(gEggArc, "models");
		gClosedEggTexList = (NJS_TEXLIST**)findArcExport(gEggArc, "texlists");
	}
}

// 0x801BAB74
extern "C" void termClosedEgg()
{
	gClosedEggModels  = 0;
	gClosedEggTexList = 0;
}

// 0x801BAB84
// Once its key is set the egg itself
extern "C" TObject* createTObjClosedEgg(TObject* parent, SetDesc* desc)
{
	if (!desc->isValid() && keyBitIsSet((u16)(desc->getInt(1) + 0x400))) {
		return (TObjAction*)createEgg(parent, desc);
	}
	return new TObjClosedEgg(parent, desc);
}

const char* gTObjClosedEggName = "TObjClosedEgg";

// 0x801BAC24
TObjClosedEgg::TObjClosedEgg(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	TEggTextureManager* bank;

	_00   = (u32)gTObjClosedEggName;
	mSize = 0xC0;
	setObjId(this, desc->mObjId);
	bank = getTEggTextureManager();
	bank->acquire(0x38);
	bank   = getTEggTextureManager();
	mModel = bank->getImage(0x38);
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
}

// 0x801BACEC
TObjClosedEgg::~TObjClosedEgg()
{
	if (mModel != 0) {
		getTEggTextureManager()->release(0x38);
	}
	removeDrawTask(mDraw);
	notifySetLinkDeath(mLink.get(), this);
	unregisterId();
}

// 0x801BAD98
void TObjClosedEgg::initFromDesc(SetDesc* desc)
{
	Vec3f pos;
	SetRot rot;

	desc->getPos(&pos);
	desc->getRot(&rot);
	mEggType = desc->getInt(0);
	mKey     = desc->getInt(1);
	shadowSetGround(&mShadow, &pos, 0);
	mShadow._30 = 15.0f;
	mShadow._34 = 15.0f;
	shadowEnable(&mShadow);
	pos.y += 7.5f;
	mPos.x = pos.x;
	mPos.y = pos.y;
	mPos.z = pos.z;
	mRotY  = rot.y;
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mRotY);
}

// 0x801BAE9C
void TObjClosedEgg::debugPrint(void*)
{
	njPrint(0xA0013, ":EGG TYPE ", mEggType);
	njPrint(0xA0014, ":SWITCH ", mKey);
}

// 0x801BAEFC
// Goes once out of range
void TObjClosedEgg::draw()
{
	if (!debugEditActive() && !isInRange(this, &mPos, 500.0f)) {
		mFlags |= 1;
	}
}

// 0x801BAF54
void TObjClosedEgg::vfn_10()
{
	if (!cameraIsVisible(1000.0f, 7.5f, &mPos, 0)) {
		mDraw->mFlags |= 1;
	} else {
		mDraw->mFlags &= ~1;
	}
}

// 0x801BAFBC
// The shell with the egg's own image or the arc's first texture on it
void TObjClosedEgg::render()
{
	f32 mtx[12];
	NJS_TEXNAME tex;
	NJS_TEXLIST texList;
	DrawStateSlot* slot;

	mtxFromView(mtx, mMtx);
	if (gClosedEggModels == 0) {
		mtxScale(7.5f, 7.5f, 7.5f, mtx, mtx);
		debugWireSphere(mtx, &gColorBlack);
		return;
	}
	slot = getDrawStateSlot();
	ObjDrawState state;
	mtxScale(1.5f, 1.5f, 1.5f, mtx, mtx);
	state.mFlags |= 0x101;
	state.mMaterial   = 0xFFFFFF80;
	state.mBlend      = 0;
	texList.textures  = &tex;
	texList.nbTexture = 1;
	if (mModel == 0) {
		tex.texaddr = (*gClosedEggTexList)->textures[0].texaddr;
	} else {
		tex.texaddr = (GjTexMem*)mModel;
	}
	njSetTexture(&texList);
	njPushMatrix(mtx);
	shadowAlphaBegin();
	njSetTexture(&texList);
	beginDrawState(slot, &state);
	gjDrawObject(gClosedEggModels[1]);
	endDrawState(slot);
	shadowAlphaEnd();
	njPopMatrixEx();
}

// 0x801BB0EC
int TObjClosedEgg::getKind()
{
	return 3;
}

// 0x801BB0F4
Vec3f* TObjClosedEgg::getPos()
{
	return &mPos;
}

// 0x801BB0FC
SetLink* TObjClosedEgg::getLink()
{
	return (SetLink*)&mLink;
}

// 0x801BB104
f32 TObjClosedEgg::getRadiusV()
{
	return gClosedEggRadius[0];
}

// 0x801BB10C
void loadGalleryEggArcs()
{
	int i;

	for (i = 0; i < GALLERY_EGG_COUNT; i++) {
		loadArc(gGalleryEggTable[i].mArc, &gGalleryEggArcs[i]);
		gGalleryEggAssets[i] = (GalleryEggAsset*)gGalleryEggArcs[i]->mData;
		bindGvmTextures((u8*)gGalleryEggAssets[i]->mGvm, gGalleryEggAssets[i]->mTexList);
	}
}

// 0x801BB198
void releaseGalleryEggArcs()
{
	int i;

	for (i = GALLERY_EGG_COUNT - 1; i >= 0; i--) {
		releaseTextures(gGalleryEggAssets[i]->mTexList);
		releaseResource(gGalleryEggArcs[i]);
		gGalleryEggArcs[i]   = 0;
		gGalleryEggAssets[i] = 0;
	}
}

// 0x801BB200
s16 galleryEggFoundBit(s8 egg)
{
	return gGalleryEggTable[egg].mFoundBit;
}

// 0x801BB218
s16 galleryEggModel(s8 egg)
{
	return gGalleryEggTable[egg].mModel;
}

// 0x801BB234
GalleryEgg::GalleryEgg()
{
	setEgg(0);
}

// 0x801BB274
GalleryEgg::~GalleryEgg()
{
	freeParts();
}

// 0x801BB2BC
// Below zero none
void GalleryEgg::setEgg(int egg)
{
	int i;

	mShown  = 1;
	mActor2 = 0;
	mActor  = 0;
	mScale  = 1.0f;
	mMtx    = 0;
	mtxIdentity(mLocal);
	mFrame  = 0.0f;
	mMotion = 0;
	mSpeed  = 0.5f;
	for (i = 0; i < 5; i++) {
		mPart[i] = 0;
	}
	mFlags.clear();
	if (egg < 0) {
		mAsset = 0;
		return;
	}
	mAsset = gGalleryEggAssets[egg];
	if (mAsset != 0) {
		if (mAsset->mModels != 0) {
			NJS_OBJECT* object = mAsset->mModels[0];
			if (object->evalflags & 0x2000) {
				mActor = (GjActor*)(memAlloc(gjActorSize(object)));
				gjCreateActor(object, (u8*)mActor);
				gjPushUnitMatrix();
				gjBuildActorVertices(mActor);
				njPopMatrixEx();
				if (egg == 0x3F || egg == 0x40) {
					mActor2 = (GjActor*)(memAlloc(gjActorSize(mAsset->mModels[1])));
					gjCreateActor(mAsset->mModels[1], (u8*)mActor2);
					gjPushUnitMatrix();
					gjBuildActorVertices(mActor2);
					njPopMatrixEx();
				}
			}
		}
		if (gGalleryEggTable[egg].mNear != 0) {
			mFlags.mBits |= 1;
		}
		mScale = gGalleryEggTable[egg].mScale;
		mSpeed = gGalleryEggTable[egg].mSpeed;
	}
}

// 0x801BB458
void GalleryEgg::freeParts()
{
	int i;

	for (i = 4; i >= 0; i--) {
		if (mPart[i] != 0) {
			mPart[i]->mFlags |= 1;
			mPart[i] = 0;
		}
	}
	if (mActor2 != 0) {
		memFree(mActor2);
		mActor2 = 0;
	}
	if (mActor != 0) {
		memFree(mActor);
		mActor = 0;
	}
}

// 0x801BB4E8
void GalleryEgg::start()
{
	mtxCopy(mLocal, mMtx);
	mtxScale(mScale, mScale, mScale, mLocal, mLocal);
}

// 0x801BB534
void GalleryEgg::stop()
{
	freeParts();
	setEgg(-1);
}

static void galleryNodeCallback(NJS_OBJECT*);
static int galleryWalkNodes(NJS_OBJECT* object, int node);

// A skinned vertex its place and a step added an eighth at a time
struct GalleryVertex {
	s16 mPos[3];  // _00
	s16 mStep[3]; // _06
};

// The part of a GjActor that holds its vertices
struct GalleryActor {
	u8 _00[4];
	GalleryVertex* mVerts; // _04
	u32 mSize;             // _08 in bytes
};

// The head of an NJS_MOTION see TObjOrangeClockTower.h
struct GalleryMotion {
	void* mdata; // _00
	u32 nbFrame; // _04
};

// 0x801BB56C
void GalleryEgg::update()
{
	int i;

	if (mAsset == 0) {
		return;
	}
	start();
	gGalleryEggCurrent = this;
	for (i = 0; i < 5; i++) {
		gGalleryFxNodes[i] = -1;
	}
	if (mAsset->mFxCount != 0) {
		if (mAsset->mFxCount > 5) {
			mAsset->mFxCount = 5;
		}
		for (i = 0; i < mAsset->mFxCount; i++) {
			gGalleryFxNodes[i] = mAsset->mFx[i * 2];
		}
	}
	if (mActor != 0) {
		if (mAsset->mMotions != 0) {
			gjPushUnitMatrix();
			gGalleryNodeCount = 0;
			gjSetMotionCallback((void (*)(NJS_OBJECT*))galleryNodeCallback);
			((void (*)(GjActor*, void*, f32))gjMotionActor)(mActor, mAsset->mMotions[mMotion], mFrame);
			gjSetMotionCallback(0);
			if (mActor2 != 0) {
				GalleryActor* actor;
				GalleryVertex* v;
				int n;

				gGalleryNodeCount = 0;
				gjSetMotionCallback((void (*)(NJS_OBJECT*))galleryNodeCallback);
				((void (*)(GjActor*, void*, f32))gjMotionActor)(mActor2, mAsset->mMotions[mMotion], mFrame);
				gjSetMotionCallback(0);
				actor = (GalleryActor*)mActor2;
				v     = actor->mVerts;
				for (n = actor->mSize / 12; n > 0; n--) {
					v->mPos[0] = v->mPos[0] + (v->mStep[0] >> 3);
					v->mPos[1] = v->mPos[1] + (v->mStep[1] >> 3);
					v->mPos[2] = v->mPos[2] + (v->mStep[2] >> 3);
					v++;
				}
				DCFlushRange(((GalleryActor*)mActor2)->mVerts, ((GalleryActor*)mActor2)->mSize);
			}
			njPopMatrixEx();
		}
	} else if (mAsset->mMotions != 0) {
		gjPushUnitMatrix();
		njStartMotionObj(mAsset->mMotions[mMotion], mFrame);
		galleryWalkNodes(mAsset->mModels[0], 0);
		njPopMatrixEx();
	}
	if (mAsset->mMotions != 0) {
		for (i = 0; i < mAsset->mFxCount; i++) {
			Vec3f at;

			mtxMultVec(gGalleryFxMtx[i], &gVecZero, &at);
			mtxMultVec(mLocal, &at, &at);
			if (mPart[i] == 0) {
				mPart[i] = particleGenByIdDefault(&at, 0, mAsset->mFx[i * 2 + 1]);
			} else {
				TParticleGenerator* part = mPart[i];
				part->mPos.x             = at.x;
				part->mPos.y             = at.y;
				part->mPos.z             = at.z;
			}
		}
	}
	gGalleryEggCurrent = 0;
	if (mAsset->mMotions != 0) {
		mFrame += mSpeed;
		if (mFrame > (f32)((GalleryMotion*)mAsset->mMotions[mMotion])->nbFrame) {
			mFrame -= (f32)((GalleryMotion*)mAsset->mMotions[mMotion])->nbFrame;
		}
	}
}

// 0x801BB8BC
void GalleryEgg::draw()
{
	if (mAsset == 0) {
		return;
	}
	if (mAsset->mTexList != 0) {
		njSetTexture(mAsset->mTexList);
	}
	if (mAsset->mModels == 0) {
		return;
	}
	pushCullMode(0x10000000);
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
	njPushMatrixEx();
	njMultiMatrix(0, mLocal);
	if (mActor != 0) {
		gjDrawActor(mActor);
		if (mActor2 != 0) {
			gjDrawActor(mActor2);
		}
	} else if (mAsset->mMotions != 0) {
		gjDrawMotion(mAsset->mModels[0], mAsset->mMotions[0], mFrame);
	} else {
		gjDrawObject(mAsset->mModels[0]);
	}
	njPopMatrixEx();
	popCullMode();
}

// 0x801BB998
// The motion's node callback keeps the matrices of the nodes the particles follow
static void galleryNodeCallback(NJS_OBJECT*)
{
	int i;

	for (i = 0; i < 5; i++) {
		if (gGalleryNodeCount == gGalleryFxNodes[i]) {
			njGetMatrix(gGalleryFxMtx[i]);
		}
	}
	gGalleryNodeCount++;
}

// 0x801BBA10
// The same for a model without an actor walking the tree
static int galleryWalkNodes(NJS_OBJECT* object, int node)
{
	int i;

	do {
		njPushMatrixEx();
		njMotionTransformEx(object);
		for (i = 0; i < 5; i++) {
			if (node == gGalleryFxNodes[i]) {
				njGetMatrix(gGalleryFxMtx[i]);
			}
		}
		node++;
		if (object->child != 0) {
			node = galleryWalkNodes(object->child, node);
		}
		njPopMatrixEx();
		object = object->sibling;
	} while (object != 0);
	return node;
}

// 0x801BBABC
void SnowFlags::clear()
{
	mBits = 0;
}

// 0x801BBAC8
extern "C" TPlyEffStSleep* createPlyEffStSleep(TObject* parent)
{
	return new TPlyEffStSleep(parent);
}
