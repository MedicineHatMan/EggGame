#include "stage/Stage.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "stage/StageObjUtil.h"
#include "io/ResBank.h"
#include "egg/ObjEgg.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "object/ObjAction.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjLauncher.h"
#include "core/Mem.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "io/NinjaFile.h"
#include "engine/TFrameBlur.h"
#include "animal/RacerMover.h"
struct DrawState;
struct DrawStateSlot;
struct NJS_OBJECT;
struct GjActor;

// 0x800E7708
void resBankCtor(ResBank* bank)
{
	bank->mArc  = NULL;
	bank->mData = NULL;
}

// 0x800E7718
void resBankLoad(ResBank* bank, const char* name)
{
	loadArc(name, &bank->mArc);
	bank->mData  = (ResBankData*)bank->mArc->mData;
	bank->mExtra = NULL;
}

// 0x800E7760
void resBankRelease(ResBank* bank)
{
	if (bank->mArc != NULL) {
		releaseResource(bank->mArc);
	}
	bank->mArc  = NULL;
	bank->mData = NULL;
}

// 0x800E77A4
NJS_OBJECT** resBankGetModels(ResBank* bank)
{
	return bank->mData->mModels;
}

// 0x800E77B0
NJS_MOTION** resBankGetMotion(ResBank* bank, int no)
{
	return bank->mData->mMotions[no];
}

// 0x800E77C4
NJS_TEXLIST** resBankGetTextures(ResBank* bank)
{
	return bank->mData->mTextures;
}

// 0x800E77D0
void** resBankGetObjects(ResBank* bank)
{
	return (void**)bank->mData->_0C;
}

// 0x800E77DC
// Loads a texture file and adds it to the texture list numbered no
void resBankAddTextures(ResBank* bank, int no, const char* name)
{
	ResBankExtra* extra = new ResBankExtra;
	extra->mNo          = no;
	extra->mNext        = NULL;
	extra->mFile        = NULL;
	if (bank->mExtra == NULL) {
		bank->mExtra = extra;
	} else {
		ResBankExtra* last = bank->mExtra;
		while (last->mNext != NULL) {
			last = last->mNext;
		}
		last->mNext = extra;
	}
	loadSizedResource(name, &extra->mFile);
	GvmLoader* loader = new GvmLoader;
	gvmLoaderSetList(loader, bank->mData->mTextures[no]);
	gvmLoaderFromMemory(loader, (unsigned char*)(extra->mFile->mData));
	gvmLoaderTakeList(loader);
	delete loader;
}

// 0x800E78B8
void resBankRemoveTextures(ResBank* bank, int no)
{
	njReleaseTexture(bank->mData->mTextures[no]);
	ResBankExtra* prev  = NULL;
	ResBankExtra* extra = bank->mExtra;
	while (extra != NULL) {
		if (extra->mNo == no) {
			break;
		}
		prev  = extra;
		extra = extra->mNext;
	}
	if (extra != NULL) {
		if (prev != NULL) {
			prev->mNext = extra->mNext;
		}
		releaseResource(extra->mFile);
		delete extra;
	}
}

// 0x800E7948
void fxBankLoadDefault(FxBankFile* file, const char* name)
{
	fxBankLoad(file, name, 5);
}

// 0x800E796C
void fxBankLoad(FxBankFile* file, const char* name, int bank)
{
	file->mArc  = NULL;
	file->mBank = bank;
	loadArc(name, &file->mArc);
	if (file->mArc != NULL) {
		fxBankBind(file->mBank & 0xFF, (u8*)file->mArc->mData);
	}
}

// Not in the DOL the linker stripped it
extern "C" void flattenGap(Vec3f* gap)
{
	gap->y = 0.0f;
}

// 0x800E7A1C
// emitted after fxBankRelease assigning objIdIter()'s by-value result defers it by one function
// The id of the nearest egg within range of from level with it and inside spread of the way angle faces other than self
extern "C" u32 findEggAhead(const Vec3f* from, int angle, int self, int spread, f32 range)
{
	TObjEgg* egg;
	u32 found = 0xFFFF;
	Vec3f at  = *from;
	ObjIdIter iter;
	Vec3f dir;
	f32 dist;
	njSinCos(angle, &dir.x, &dir.z);
	f32 cone = njCos(spread);
	f32 best = range;
	iter     = objIdIter();
	for (iter.begin(); !iter.atEnd(); iter.next()) {
		egg = iter.getEgg();
		if (egg->canAct() != 0 && (u16)self != egg->mObjId) {
			Vec3f* pos = egg->getPos();
			Vec3f gap;
			gap.x = pos->x;
			gap.y = pos->y;
			gap.z = pos->z;
			njSubVector(&gap, &at);
			if ((f32)__fabs(gap.y) < 100.0f) {
				gap.y = 0.0f;
				dist  = njUnitVector(&gap);
				if (dist < gUnitVectorMin) {
					gap = gVecZero;
				}
				if (dist < range && njInnerProduct(&gap, &dir) > cone && dist < best) {
					found = egg->mObjId;
					best  = dist;
				}
			}
		}
	}
	return found;
}

// 0x800E79C8
void fxBankRelease(FxBankFile* file)
{
	if (file->mArc != NULL) {
		fxBankUnbind(file->mBank & 0xFF, (u8*)file->mArc->mData);
		releaseResource(file->mArc);
		file->mArc = NULL;
	}
}

// 0x800E7C0C
void modelPostTevAdd(u32 stage)
{
	int s = stage & 0xFF;
	GXSetTevColorIn(s, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
	GXSetTevColorOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(s, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(s, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x800E7CB0
void modelPostTexGenNormal(u32 gen)
{
	GXSetTexCoordGen2(gen & 0xFF, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX5, GX_TRUE, GX_PTTEXMTX19);
}

// 0x800E7CE8
struct FxModelMatrix {
	f32 m[12];
};

TObjFxModel::TObjFxModel(TObject* parent, void* model, void* textures, const f32* matrix)
    : TObject(parent)
{
	mActor    = NULL;
	mTextures = NULL;
	mState[0] = 0;
	_4C       = 0;
	mModel    = (NJS_OBJECT*)model;
	mActor    = NULL;
	if (*(u32*)model & 0x2000) {
		mActor = memAlloc(gjActorSize((NJS_OBJECT*)model));
		gjCreateActor((NJS_OBJECT*)model, (u8*)mActor);
		njPushMatrixEx();
		njUnitMatrix(NULL);
		gjBuildActorVertices((GjActor*)mActor);
		njPopMatrixEx();
	}
	mTextures                = textures;
	*(FxModelMatrix*)mMatrix = *(const FxModelMatrix*)matrix;
	mState[0]                = 0x200;
	mState[5]                = -1;
	mTask                    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
}

// 0x800E7DE0
TObjFxModel::~TObjFxModel()
{
	if (mActor != NULL) {
		memFree(mActor);
		mActor = NULL;
	}
	removeDrawTask(mTask);
}

// 0x800E7E60
void TObjFxModel::draw()
{
	if (fadeAmbient((DrawState*)mState, 0, 8) != 0) {
		mFlags |= 1;
	}
}

// 0x800E7EAC
void TObjFxModel::vfn_10()
{
}

// 0x800E7EB0
void TObjFxModel::render()
{
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrixEx();
	njMultiMatrix(NULL, mMatrix);
	beginDrawState(slot, (const DrawState*)mState);
	if (mTextures != NULL) {
		njSetTexture(mTextures);
	}
	if (mActor != NULL) {
		gjDrawActor((GjActor*)mActor);
	} else if (mModel != NULL) {
		gjDrawObject(mModel);
	}
	endDrawState(slot);
	njPopMatrixEx();
}

// 0x800E7F3C
void fxModelMotion(f32 frame, TObjFxModel* model, void* motion)
{
	if (model->mActor != NULL) {
		gjPushUnitMatrix();
		gjMotionActor(frame, (GjActor*)model->mActor, motion);
		njPopMatrixEx();
	}
}

// ---- The animals' texture swap one slot of a texture list borrows a texture of the shared animal list for a while

/** The shared animal model at gAnimalCommonModel */
struct AniCommonTex {
	u8 _00[0x10];
	TexList** mTextures; // _10
};

extern "C" {
extern const TexVtx sResMarkQuad[4] = {
	{ -2.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0xFFFFFFFF },
	{ 2.0f, 4.0f, 0.0f, 1.0f, 0.0f, 0xFFFFFFFF },
	{ -2.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0xFFFFFFFF },
	{ 2.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xFFFFFFFF },
};

TexList* sTexSwapList; // 0x802C5308
int sTexSwapSlot;      // 0x802C530C
void* sTexSwapSaved;   // 0x802C5310
}

// 0x800E7F98
extern "C" void aniTexSwapReset()
{
	sTexSwapList  = 0;
	sTexSwapSlot  = 0;
	sTexSwapSaved = 0;
}

// 0x800E7FAC
// Gives back what was in the slot
extern "C" void* aniTexSwap(TexList* list, int slot, int from)
{
	TexName* name = &list->textures[slot];
	void* old     = name->texaddr;
	name->texaddr = ((AniCommonTex*)gAnimalCommonModel)->mTextures[0]->textures[from].texaddr;
	return old;
}

// 0x800E7FE0
// One swap at a time
extern "C" void aniTexSwapBegin(TexList* list, int slot, int from)
{
	if (sTexSwapList == 0) {
		sTexSwapList  = list;
		sTexSwapSlot  = slot;
		sTexSwapSaved = aniTexSwap(list, slot, from);
	}
}

// 0x800E8018
extern "C" void aniTexSwapEnd()
{
	if (sTexSwapList == 0) {
		return;
	}
	sTexSwapList->textures[sTexSwapSlot].texaddr = sTexSwapSaved;
	sTexSwapList                                 = 0;
}

/**
 * The mark over an animal that greets the player: texture 2 of the shared animal list on a quad that faces the
 * camera, drifting up for 60 frames and picking a new sideways drift every 10. The name is ours.
 *
 * @size 0x40
 */
class TAniGreetMark : public TObject {
public:
	TAniGreetMark(TObject* parent, const Vec3f* pos);
	virtual ~TAniGreetMark();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	Vec3f mPos;          // _20
	Vec3f mVel;          // _2C
	int mLife;           // _38
	DrawTask* mDrawTask; // _3C
};

typedef char TAniGreetMark_size_check[(sizeof(TAniGreetMark) == 0x40) ? 1 : -1];

// 0x800E8048
extern "C" TAniGreetMark* createTAniGreetMark(TObject* parent, const Vec3f* pos)
{
	return new TAniGreetMark(parent, pos);
}

// 0x800E809C
TAniGreetMark::TAniGreetMark(TObject* parent, const Vec3f* pos)
    : TObject(parent)
    , mPos(*pos)
{
	mVel.x    = 0.0f;
	mVel.y    = 0.2f;
	mVel.z    = 0.0f;
	mLife     = 60;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
}

// 0x800E812C
TAniGreetMark::~TAniGreetMark()
{
	removeDrawTask(mDrawTask);
}

// 0x800E8194
void TAniGreetMark::draw()
{
	if (mLife == 0) {
		mFlags |= 1;
	} else {
		mLife--;
	}
	if (mLife % 10 == 0) {
		mVel.x = 0.4f * (3.0517578e-5f * rand()) + -0.2f;
		mVel.z = 0.4f * (3.0517578e-5f * rand()) + -0.2f;
	}
	njAddVector(&mPos, &mVel);
}

// 0x800E8274
void TAniGreetMark::vfn_10()
{
}

// 0x800E8278
void TAniGreetMark::render()
{
	DrawStateSlot* slot = getDrawStateSlot();
	int rot             = camGetDrawYaw() + 0x8000;
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY((u16)rot);
	beginDrawState(slot, (const DrawState*)getDefaultDrawState());
	njSetTexture(((AniCommonTex*)gAnimalCommonModel)->mTextures[0]);
	njSetTextureNum(2);
	njDrawTexture3DEx(sResMarkQuad, 4, 1);
	endDrawState(slot);
	njPopMatrixEx();
}
