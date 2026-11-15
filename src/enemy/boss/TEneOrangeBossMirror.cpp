#include "object/ObjAction.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"
#include "engine/FxHolder.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneOrangeBossMap.h"
#include "io/ResBank.h"
#include "Ninja/ninja.h"
#include "engine/CharaColli.h"
#include "engine/MirrorRipple.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamPath.h"
#include "engine/MeshPool.h"
#include "enemy/boss/TEneOrangeBossFloor.h"
#include "enemy/ObjEnemy.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "object/ObjShadow.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "core/Mem.h"
#include "stl/math.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "stage/TEventCtrl.h"
#include "engine/GroundQuery.h"
#include "engine/WorldMesh.h"
#include "enemy/EnemyFx.h"
#include "enemy/EnemyTexFx.h"
#include "enemy/boss/TEneOrangeBossMirror_Frag.h"
#include "core/TMainTask.h"
#include "animal/RacerMover.h"
#include "core/SdkCalls.h"

const char* gTEneOrangeBossMirrorName = "TEneOrangeBossMirror";
struct MeshObject;
struct CharaColli;
struct MeshColli;

extern "C" {
}

class TEneOrangeBossMirror : public TObjAction {
public:
	enum {
		// mMode
		MODE_UP     = 0,
		MODE_LOWER  = 1, // sinking
		MODE_RAISE  = 2, // rising
		MODE_DOWN   = 3,
		// mBreak
		BREAK_WHOLE   = 0,
		BREAK_CRACK   = 1, // shaking about to break
		BREAK_BROKEN  = 2,
		BREAK_RESTORE = 3, // fading back in
		// mMirrorFlags
		MIRROR_FLAG1   = 0x1,  // a hit only rings it does not break
		MIRROR_RESTORE = 0x2,  // grow back now
		MIRROR_BUSY    = 0x4,  // cracking broken or restoring
		MIRROR_DOWN    = 0x8,
		MIRROR_WATCH   = 0x10, // watching for the player
		MIRROR_HIT     = 0x20, // the player touched it while watched
		MIRROR_BREAK   = 0x40, // break this frame
	};

	TEneOrangeBossMirror(TObject* parent, void* owner, const Vec3f* pos, int arg);
	virtual ~TEneOrangeBossMirror();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                  // _0C 0x8011DDD4 the per-frame work
	virtual void vfn_10();                                // _10 0x8011E328 hides it when out of sight
	virtual void render();                                // _1C 0x8011EAA0
	virtual Vec3f* getPos();                              // _20 0x8011EC04
	virtual int getKind();                                // _2C 0x8011EC0C
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8011D9B4

	void reset();               // 0x8011DA1C down and whole
	void clearHit();            // 0x8011DB0C
	u32 wasHit();               // 0x8011DB1C
	u16 hitBy();                // 0x8011DB28
	void watch();               // 0x8011DB30
	void unwatch();             // 0x8011DB64
	int isWhole();              // 0x8011DB98
	void setFlag1();            // 0x8011DBA8
	void clearFlag1();          // 0x8011DBB8
	void restore();             // 0x8011DBC8
	void setGrid(int grid);     // 0x8011DBD8
	void rippleAt(const Vec3f* pos); // 0x8011DBE0
	void raise();               // 0x8011DC58
	void lower();               // 0x8011DC64
	int isMode(int mode);       // 0x8011DC70
	int isStarting();           // 0x8011DC84 the move it is making has just begun
	void meshOn();              // 0x8011DCE8
	void meshRefresh();         // 0x8011DD0C
	void forceBreak();          // 0x8011DD30
	void turn(int angle);       // 0x8011DD40
	void dust();                // 0x8011DD74
	void updateMode();          // 0x8011DEF0
	void updateBreak();         // 0x8011E050

	Vec3f mPos;           // _30 y goes from -60 down to 50 up
	u8 _3C[4];
	int mAngle;           // _40
	u8 _44[4];
	MirrorPane mPane;     // _48
	void* mOwner;         // _204 0 in the ring 1 in the middle
	int mMode;            // _208
	int mPrevMode;        // _20C
	int mBreak;           // _210
	u32 mMirrorFlags;     // _214
	u16 mHitBy;           // _218
	u8 _21A[2];
	EnemyLight mLight;    // _21C
	DrawTask* mDraw;          // _25C
	TObject* mFx;         // _260 the dust while it moves
	FxHolder mGlow;       // _264
	SeHolder mRing;       // _298
	int mTimer;           // _2CC
	Vec3f mShake;         // _2D0
	f32 mFade;            // _2DC
	int mGrid;            // _2E0 below 40 the glass is drawn as a grid of that many
	MirrorRipple* mChild; // _2E4
};

typedef char TEneOrangeBossMirror_size_check[(sizeof(TEneOrangeBossMirror) == 0x2E8) ? 1 : -1];

extern "C" {

// The arena's bank and the reflection texture
ResBank sOrangeBossMapBank;           // 0x80282724
NJS_TEXNAME sMirrorScreenName;        // 0x80282718
NJS_TEXLIST sMirrorScreenList;        // 0x802C5580
TEneOrangeBossMirror** sMirrorRing;   // 0x802C5588 MIRROR_RING_COUNT of them
TEneOrangeBossMirror** sMirrorCenter; // 0x802C558C one
NJS_OBJECT** sOrangeBossMapModels;           // 0x802C5590
NJS_TEXLIST** sOrangeBossMapTextures;         // 0x802C5594
}

// The radians-to-angle factor 0x10000 / 2pi
#define RAD_TO_ANGLE 10430.38043493439

// 0x8011BE94
extern "C" void initOrangeBossMap()
{
	NJS_TEXINFO info;
	int i;

	resBankLoad(&sOrangeBossMapBank, "ar_ene_orange_boss_map.arc");
	resBankAddTextures(&sOrangeBossMapBank, 0, "ene_orange_boss_map.gvm");
	sOrangeBossMapModels   = resBankGetModels(&sOrangeBossMapBank);
	sOrangeBossMapTextures = resBankGetTextures(&sOrangeBossMapBank);
	sMirrorScreenList.textures  = &sMirrorScreenName;
	sMirrorScreenList.nbTexture = 1;
	njSetTextureInfo(&info, 0, 4, 320, 240);
	njSetTextureName(&sMirrorScreenName, (void*)&info, (void*)0x101C, (u32)0x08800000);
	njLoadTexture(&sMirrorScreenList);
	sMirrorRing = new TEneOrangeBossMirror*[MIRROR_RING_COUNT];
	for (i = 0; i < MIRROR_RING_COUNT; i++) {
		sMirrorRing[i] = 0;
	}
	sMirrorCenter    = new TEneOrangeBossMirror*[1];
	sMirrorCenter[0] = 0;
	orangeFloorClear();
}

// Flags a mirror to be deleted
static inline void mirrorKill(TEneOrangeBossMirror* mirror)
{
	mirror->mFlags |= 1;
}

// 0x8011BF90
extern "C" void shutdownOrangeBossMap()
{
	int i;

	orangeFloorEnd();
	for (i = 0; i >= 0; i--) {
		if (sMirrorCenter[i] != 0) {
			mirrorKill(sMirrorCenter[i]);
			sMirrorCenter[i] = 0;
		}
	}
	if (sMirrorCenter != 0) {
		delete[] sMirrorCenter;
		sMirrorCenter = 0;
	}
	for (i = MIRROR_RING_COUNT - 1; i >= 0; i--) {
		if (sMirrorRing[i] != 0) {
			mirrorKill(sMirrorRing[i]);
			sMirrorRing[i] = 0;
		}
	}
	if (sMirrorRing != 0) {
		delete[] sMirrorRing;
		sMirrorRing = 0;
	}
	njReleaseTexture(&sMirrorScreenList);
	resBankRemoveTextures(&sOrangeBossMapBank, 0);
	resBankRelease(&sOrangeBossMapBank);
	sOrangeBossMapModels   = 0;
	sOrangeBossMapTextures = 0;
}

// 0x8011C074
extern "C" NJS_TEXLIST** orangeBossMapTextures()
{
	return (NJS_TEXLIST**)sOrangeBossMapTextures;
}

// 0x8011C07C
extern "C" NJS_OBJECT** orangeBossMapModels()
{
	return (NJS_OBJECT**)sOrangeBossMapModels;
}

// 0x8011C084
extern "C" NJS_MOTION** orangeBossMapMotion(int index)
{
	return resBankGetMotion(&sOrangeBossMapBank, index);
}

// 0x8011C0B0
extern "C" void mirrorCopyScreen()
{
	GXSetTexCopySrc(0, 0, 320, 240);
	GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_FALSE);
	GXCopyTex(sMirrorScreenList.textures->texaddr->hdr->info.image, GX_TRUE);
}

// 0x8011C10C
extern "C" void mirrorsHide()
{
	int i;

	if (sMirrorRing != 0) {
		for (i = 0; i < MIRROR_RING_COUNT; i++) {
			if (sMirrorRing[i] != 0) {
				eventHideObj((u8*)sMirrorRing[i]);
			}
		}
	}
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		eventHideObj((u8*)sMirrorCenter[0]);
	}
}

// 0x8011C184
// The ring 220 out from the middle every eighth of a turn each facing in
extern "C" void mirrorsCreate()
{
	Vec3f pos;
	int angle;
	int i;

	for (i = 0; i < MIRROR_RING_COUNT; i++) {
		angle = (i << 16) / MIRROR_RING_COUNT;
		njSinCos(angle, &pos.x, &pos.z);
		pos.x *= 220.0f;
		pos.y = 50.0f;
		pos.z *= 220.0f;
		sMirrorRing[i] = createMirrorRing(gTL_04, &pos, angle + 0x8000);
	}
	pos.x = 0.0f;
	pos.y = 50.0f;
	pos.z = 0.0f;
	sMirrorCenter[0] = createMirrorCenter(gTL_04, &pos, 0);
	orangeFloorStart();
}

// 0x8011C270
extern "C" TEneOrangeBossMirror* mirrorAt(int slot)
{
	if (sMirrorRing != 0 && slot >= 0 && slot < MIRROR_RING_COUNT) {
		return sMirrorRing[slot];
	}
	return 0;
}

// 0x8011C2A0
extern "C" void mirrorForget(TEneOrangeBossMirror* mirror)
{
	int i;

	if (sMirrorRing != 0) {
		for (i = 0; i < MIRROR_RING_COUNT; i++) {
			if (sMirrorRing[i] == mirror) {
				sMirrorRing[i] = 0;
			}
		}
	}
	if (sMirrorCenter != 0) {
		for (i = 0; i < 1; i++) {
			if (sMirrorCenter[i] == mirror) {
				sMirrorCenter[i] = 0;
			}
		}
	}
}

// 0x8011C2FC
extern "C" int mirrorSlotOf(const Vec3f* dir)
{
	int angle = (int)(RAD_TO_ANGLE * (f32)atan2(dir->x, dir->z));
	angle = (angle + 0x1000) & 0xFFFF;
	return angle / 0x2000;
}

// 0x8011C34C
extern "C" int mirrorSlotOpposite(const Vec3f* dir)
{
	return (mirrorSlotOf(dir) + 4) % MIRROR_RING_COUNT;
}

// 0x8011C384
extern "C" int mirrorSlotAngle(int slot)
{
	return slot << 13;
}

// 0x8011C38C
extern "C" int mirrorSlotDir(int slot, Vec3f* dir)
{
	int angle = mirrorSlotAngle(slot);
	njSinCos(angle, &dir->x, &dir->z);
	dir->y = 0.0f;
	return angle;
}

// 0x8011C3D4
extern "C" void mirrorSlotPos(int slot, Vec3f* pos, int* angle)
{
	int a = mirrorSlotAngle(slot);
	njSinCos(a, &pos->x, &pos->z);
	pos->x = 220.0f * pos->x;
	pos->y = 50.0f;
	pos->z = 220.0f * pos->z;
	*angle = a + 0x8000;
}

// 0x8011C444
extern "C" int mirrorIsClear(int slot)
{
	TEneOrangeBossMirror* mirror = mirrorAt(slot);
	if (mirror != 0) {
		return mirror->isWhole();
	}
	return 0;
}

// 0x8011C478
// Which mirror lies along dir if one is within the given width of it
// 0x802C9D00
// in radians how far past the slot's centre a mirror still counts as facing it
static const f32 sFacingSlack = 0.22727273f;

static inline int radToAngle(const f32& rad)
{
	return (int)(RAD_TO_ANGLE * rad);
}

extern "C" int mirrorSlotFacing(f32 width, const Vec3f* dir, int* slot)
{
	Vec3f flat;
	f32 len;
	int angle;
	int found;

	flat.x = dir->x;
	flat.y = 0.0f;
	flat.z = dir->z;
	len    = njUnitVector(&flat);
	if (len < gUnitVectorMin) {
		flat = gVecZero;
	}
	angle = (int)(RAD_TO_ANGLE * (f32)atan2(flat.x, flat.z));
	found = (u16)(angle + 0x1000);
	found /= 0x2000;
	int diff = angleDiff(found << 13, angle);
	if (diff <= radToAngle(sFacingSlack) + radToAngle(width / len)) {
		*slot = found;
		return 1;
	}
	return 0;
}

// 0x8011C594
extern "C" void mirrorListRaise(TEneOrangeBossMirror** list, int count)
{
	int i;

	if (list != 0) {
		for (i = 0; i < count; i++) {
			if (list[i] != 0) {
				list[i]->raise();
			}
		}
	}
}

// 0x8011C5F0
extern "C" void mirrorListLower(TEneOrangeBossMirror** list, int count)
{
	int i;

	if (list != 0) {
		for (i = 0; i < count; i++) {
			if (list[i] != 0) {
				list[i]->lower();
			}
		}
	}
}

// 0x8011C64C
extern "C" int mirrorListAllInMode(TEneOrangeBossMirror** list, int count, int mode)
{
	int i;

	if (list != 0) {
		for (i = 0; i < count; i++) {
			if (list[i] != 0 && list[i]->isMode(mode) == 0) {
				return 0;
			}
		}
		return 1;
	}
	return 0;
}

// 0x8011C6CC
extern "C" void mirrorsReset()
{
	int i;

	if (sMirrorRing != 0) {
		for (i = 0; i < MIRROR_RING_COUNT; i++) {
			if (sMirrorRing[i] != 0) {
				sMirrorRing[i]->reset();
			}
		}
	}
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		sMirrorCenter[0]->reset();
	}
}

// 0x8011C744
extern "C" void mirrorCenterClearFlag20()
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		sMirrorCenter[0]->clearHit();
	}
}

// 0x8011C77C
extern "C" u32 mirrorCenterFlag20()
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		return sMirrorCenter[0]->wasHit();
	}
	return 0;
}

// 0x8011C7BC
extern "C" u16 mirrorCenterId()
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		return sMirrorCenter[0]->hitBy();
	}
	return 0xFFFF;
}

// 0x8011C800
extern "C" void mirrorRingSetFlag1()
{
	int i;

	if (sMirrorRing != 0) {
		for (i = 0; i < MIRROR_RING_COUNT; i++) {
			if (sMirrorRing[i] != 0) {
				sMirrorRing[i]->setFlag1();
			}
		}
	}
}

// 0x8011C85C
extern "C" void mirrorRingClearFlag1()
{
	int i;

	if (sMirrorRing != 0) {
		for (i = 0; i < MIRROR_RING_COUNT; i++) {
			if (sMirrorRing[i] != 0) {
				sMirrorRing[i]->clearFlag1();
			}
		}
	}
}

// 0x8011C8B8
extern "C" void mirrorRingSetFlag2()
{
	int i;

	if (sMirrorRing != 0) {
		for (i = 0; i < MIRROR_RING_COUNT; i++) {
			if (sMirrorRing[i] != 0) {
				sMirrorRing[i]->restore();
			}
		}
	}
}

// 0x8011C914
extern "C" void mirrorRingSetTarget(int target)
{
	int i;

	if (sMirrorRing != 0) {
		for (i = 0; i < MIRROR_RING_COUNT; i++) {
			if (sMirrorRing[i] != 0) {
				sMirrorRing[i]->setGrid(target);
			}
		}
	}
}

// 0x8011C978
extern "C" void mirrorAimAt(int slot, const Vec3f* pos)
{
	TEneOrangeBossMirror* mirror = mirrorAt(slot);
	if (mirror != 0) {
		mirror->rippleAt(pos);
	}
}

// 0x8011C9B4
extern "C" void mirrorRingRaise()
{
	mirrorListRaise(sMirrorRing, MIRROR_RING_COUNT);
	mirrorRingMoveSe();
}

// 0x8011C9E0
extern "C" void mirrorRingLower()
{
	mirrorListLower(sMirrorRing, MIRROR_RING_COUNT);
	mirrorRingMoveSe();
}

// 0x8011CA0C
extern "C" int mirrorRingAllDown()
{
	return mirrorListAllInMode(sMirrorRing, MIRROR_RING_COUNT, 0);
}

// 0x8011CA38
extern "C" int mirrorRingAllUp()
{
	return mirrorListAllInMode(sMirrorRing, MIRROR_RING_COUNT, 3);
}

// 0x8011CA64
extern "C" void mirrorRingMoveSe()
{
	if (sMirrorRing != 0 && sMirrorRing[0] != 0 && sMirrorRing[0]->isStarting() != 0) {
		seStart(0x151);
	}
}

// 0x8011CAAC
extern "C" void mirrorCenterSetFlag2()
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		sMirrorCenter[0]->restore();
	}
}

// 0x8011CAE4
extern "C" void mirrorCenterRaise(int se)
{
	mirrorListRaise(sMirrorCenter, 1);
	if (se != 0) {
		mirrorCenterMoveSe();
	}
}

// 0x8011CB24
extern "C" void mirrorCenterLower(int se)
{
	mirrorListLower(sMirrorCenter, 1);
	if (se != 0) {
		mirrorCenterMoveSe();
	}
}

// 0x8011CB64
extern "C" int mirrorCenterDown()
{
	return mirrorListAllInMode(sMirrorCenter, 1, 0);
}

// 0x8011CB90
extern "C" int mirrorCenterUp()
{
	return mirrorListAllInMode(sMirrorCenter, 1, 3);
}

// 0x8011CBBC
extern "C" void mirrorCenterTurn(int angle)
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		sMirrorCenter[0]->turn(angle);
	}
}

// 0x8011CBF8
extern "C" void mirrorCenterSolid()
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		sMirrorCenter[0]->unwatch();
	}
}

// 0x8011CC30
extern "C" void mirrorCenterHollow()
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		sMirrorCenter[0]->watch();
	}
}

// 0x8011CC68
extern "C" void mirrorCenterAimAt(const Vec3f* pos)
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		sMirrorCenter[0]->rippleAt(pos);
	}
}

// 0x8011CCA4
extern "C" void mirrorCenterMoveSe()
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0 && sMirrorCenter[0]->isStarting() != 0) {
		seStartAtFar(0x151, &sMirrorCenter[0]->mPos);
	}
}

// 0x8011CCF8
extern "C" void mirrorCenterMesh(int on)
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		if (on != 0) {
			sMirrorCenter[0]->meshOn();
		} else {
			sMirrorCenter[0]->meshRefresh();
		}
	}
}

// 0x8011CD48
extern "C" void mirrorCenterSetFlag40()
{
	if (sMirrorCenter != 0 && sMirrorCenter[0] != 0) {
		sMirrorCenter[0]->forceBreak();
	}
}

// 0x8011CD80
MirrorPane::MirrorPane()
{
	mCollider = 0;
	mDepth    = 0.0f;
	mMesh     = 0;
}

// 0x8011CD98
MirrorPane::~MirrorPane()
{
	colliRetire((CharaColli*)mCollider);
	mCollider = 0;
	meshColliFree(mMesh);
}

// 0x8011CDF4
void MirrorPane::init(u16 objId, Vec3f* pos, int angle)
{
	initMesh(pos, angle);
	initCollider(objId, pos, angle);
	meshClearState400(mMesh);
}

// 0x8011CE50
void MirrorPane::setPose(Vec3f* pos, int angle)
{
	buildMesh(pos, angle);
	placeCollider(pos, angle);
}

// 0x8011CE94
// One face of the mesh its corners normal and bounding sphere
static void makeTri(f32 nx, f32 nz, MeshTri* tri, Vec3f* verts, int a, int b, int c)
{
	Vec3f sum;
	Vec3f vb;
	Vec3f vc;
	Vec3f va;

	tri->mVerts[0] = a;
	tri->mVerts[1] = b;
	tri->mVerts[2] = c;
	tri->mMask     = 0x201;
	tri->mNormal.x = nx;
	tri->mNormal.y = 0.0f;
	tri->mNormal.z = nz;
	sum = verts[a];
	vb.set(verts[b].x, verts[b].y, verts[b].z);
	njAddVector(&sum, &vb);
	vc.set(verts[c].x, verts[c].y, verts[c].z);
	njAddVector(&sum, &vc);
	gjScaleVector(0.33333334f, &sum, &sum);
	tri->mCenter = sum;
	va.set(verts[a].x, verts[a].y, verts[a].z);
	njSubVector(&sum, &va);
	tri->mRadius = njScalor(&sum);
}

// 0x8011CFB8
void MirrorPane::initMesh(Vec3f* pos, int angle)
{
	f32 mtx[12];

	mData.mVerts    = mVerts;
	mData._04       = 8;
	mData.mTris     = mTris;
	mData.mTriCount = 8;
	mtxIdentity(mtx);
	mMesh = (MeshObject*)meshColliCreate();
	meshColliSetData(mMesh, &mData);
	meshSetState200(mMesh, 1);
	meshColliSetMatrix(mMesh, mtx);
	buildMesh(pos, angle);
}

// 0x8011D048
// A slab 100 wide 100 high and 10 deep turned and moved into place
#pragma opt_common_subs off
void MirrorPane::buildMesh(Vec3f* pos, int angle)
{
	f32 s;
	f32 c;
	Vec3f* v;
	int i;
	Vec3f max;
	Vec3f min;
	f32 box[6];

	njSinCos(angle, &s, &c);
	mVerts[0].x = -50.0f;
	mVerts[0].y = 100.0f;
	mVerts[0].z = 5.0f;
	mVerts[1].x = 50.0f;
	mVerts[1].y = 100.0f;
	mVerts[1].z = 5.0f;
	mVerts[2].x = -50.0f;
	mVerts[2].y = 0.0f;
	mVerts[2].z = 5.0f;
	mVerts[3].x = 50.0f;
	mVerts[3].y = 0.0f;
	mVerts[3].z = 5.0f;
	mVerts[4].x = 50.0f;
	mVerts[4].y = 100.0f;
	mVerts[4].z = -5.0f;
	mVerts[5].x = -50.0f;
	mVerts[5].y = 100.0f;
	mVerts[5].z = -5.0f;
	mVerts[6].x = 50.0f;
	mVerts[6].y = 0.0f;
	mVerts[6].z = -5.0f;
	mVerts[7].x = -50.0f;
	mVerts[7].y = 0.0f;
	mVerts[7].z = -5.0f;
	gjPushUnitMatrix();
	njTranslateEx(pos);
	gjRotateY(angle);
	v = mVerts;
	for (i = 0; i < 8; i++) {
		njCalcPointEx(v, v);
		v++;
	}
	njPopMatrixEx();
	makeTri(s, c, &mTris[0], mVerts, 0, 2, 1);
	makeTri(s, c, &mTris[1], mVerts, 1, 2, 3);
	makeTri(-s, -c, &mTris[2], mVerts, 4, 6, 5);
	makeTri(-s, -c, &mTris[3], mVerts, 5, 6, 7);
	makeTri(-c, s, &mTris[4], mVerts, 5, 7, 0);
	makeTri(-c, s, &mTris[5], mVerts, 0, 7, 2);
	makeTri(c, -s, &mTris[6], mVerts, 1, 3, 4);
	makeTri(c, -s, &mTris[7], mVerts, 4, 3, 6);
	max = mVerts[0];
	min = mVerts[6];
	v   = mVerts;
	for (i = 0; i < 8; i++) {
		f32 x = v->x;
		if (max.x < x) {
			max.x = x;
		} else if (min.x > x) {
			min.x = x;
		}
		f32 z = v->z;
		if (max.z < z) {
			max.z = z;
		} else if (min.z > z) {
			min.z = z;
		}
		v++;
	}
	boxAroundLine(box, &max, &min);
	meshColliSetBox(mMesh, box);
}
#pragma opt_common_subs reset

// 0x8011D30C
void MirrorPane::initCollider(u16 objId, Vec3f* pos, int angle)
{
	mCollider = colliCreate(1, 1);
	colliShapeMakeBox(colliGetShape(mCollider, 0), pos, angle, 50.0f, 50.0f, 5.0f, 1);
	colliSetOwner(mCollider, (u16)objId);
	colliSetMasks(mCollider, 2, 0xFF, 0, 0x12);
	colliShapeSetTag(mCollider, 0, 0);
	placeCollider(pos, angle);
}

// 0x8011D3BC
void MirrorPane::placeCollider(const Vec3f* pos, int angle)
{
	Vec3f at;
	f32 s;
	f32 c;
	ColliShape* shape;

	at   = *pos;
	at.y += 50.0f;
	njSinCos(angle, &s, &c);
	at.x = s * mDepth + at.x;
	at.z = c * mDepth + at.z;
	shape = colliGetShape(mCollider, 0);
	colliShapeSetPos(shape, &at);
	colliShapeSetBoxRot(shape, angle);
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
}

// 0x8011D474
void MirrorPane::clearShapeBit8()
{
	colliShapeSetBit8(mCollider, 0);
}

// 0x8011D49C
void MirrorPane::clearContacts()
{
	colliShapeClearBit8(mCollider, 0);
}

// 0x8011D4C4
int MirrorPane::isShapeBit8()
{
	return colliGetShapes(mCollider)->mFlags & 8;
}

// 0x8011D4F0
void MirrorPane::enable()
{
	colliEnable(mCollider);
}

// 0x8011D514
void MirrorPane::disable()
{
	colliDisable(mCollider);
}

// 0x8011D538
void MirrorPane::meshOn()
{
	meshClearState400(mMesh);
}

// 0x8011D55C
void MirrorPane::meshRefresh()
{
	meshColliRefresh(mMesh);
}

// 0x8011D580
int MirrorPane::checkHit(TObjAction* by)
{
	ColliContact* contact;

	for (contact = ((CharaColli*)mCollider)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactHasBit8(contact) != 0) {
			HitInfo info;
			TObjAction* other = findObjById((u16)colliContactOwner(contact));
			if (other != 0 && other->onHit(by, colliContactTag(contact), (int*)&info) != 0 && info.mDamage > 0) {
				ColliShape* shape = colliGetShape(mCollider, 0);
				Vec3f d;
				d = contact->mPos;
				Vec3f at;
				at.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
				f32 s;
				f32 c;
				njSubVector(&d, &at);
				njSinCos(shape->mRotY, &s, &c);
				if (d.x * s + d.z * c > 0.0f) {
					return 1;
				}
			}
		}
	}
	return 0;
}

// 0x8011D6BC
void MirrorPane::setDepth(f32 depth)
{
	mDepth = depth;
}

// 0x8011D6C4
extern "C" TEneOrangeBossMirror* createMirrorRing(TObject* parent, const Vec3f* pos, int angle)
{
	return new TEneOrangeBossMirror(parent, (void*)0, pos, angle);
}

// 0x8011D724
extern "C" TEneOrangeBossMirror* createMirrorCenter(TObject* parent, const Vec3f* pos, int angle)
{
	return new TEneOrangeBossMirror(parent, (void*)1, pos, angle);
}

// 0x8011D784
TEneOrangeBossMirror::TEneOrangeBossMirror(TObject* parent, void* owner, const Vec3f* pos, int arg)
    : TObjAction(parent)
{
	_00   = (u32)gTEneOrangeBossMirrorName;
	mSize = 0x2E8;
	setObjId(this, 0xFFFF);
	mPos  = *pos;
	mAngle = arg;
	mPane.init(mObjId, &mPos, arg);
	mOwner = owner;
	mHitBy = 0xFFFF;
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mLight.reset();
	mLight.setFadeDist(-180.0f);
	mFx = 0;
	fxHolderStop(&mGlow);
	seHolderStop(&mRing);
	mRing.setRange(700.0f);
	mChild = new MirrorRipple;
	reset();
}

// 0x8011D8C4
TEneOrangeBossMirror::~TEneOrangeBossMirror()
{
	if (mChild != 0) {
		delete mChild;
		mChild = 0;
	}
	seHolderStop(&mRing);
	fxHolderStop(&mGlow);
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
	mirrorForget(this);
	removeDrawTask(mDraw);
	releaseObjId(this);
}

#define MIRROR_FIFO_F32 (*(volatile f32*)0xCC008000)
#define MIRROR_FIFO_U32 (*(volatile u32*)0xCC008000)

// 0x8011D9B4
// Notes who touched it while it is watched
int TEneOrangeBossMirror::onHit(const TObjAction* other, int arg, int* result)
{
	if ((mMirrorFlags & MIRROR_WATCH) && isPlayerOrKind4(other)) {
		mMirrorFlags |= MIRROR_HIT;
		mHitBy = other->mObjId;
		return 0;
	}
	return 0;
}

// 0x8011DA1C
void TEneOrangeBossMirror::reset()
{
	mMirrorFlags = 0;
	mBreak       = BREAK_WHOLE;
	if ((int)mOwner != 0) {
		mPane.setDepth(5.0f);
		setFlag1();
	}
	mMode     = MODE_DOWN;
	mPrevMode = mMode;
	mPos.y    = -60.0f;
	mMirrorFlags |= MIRROR_BUSY | MIRROR_DOWN;
	mTimer   = 0;
	mGrid    = 40;
	mShake.x = 0.0f;
	mShake.y = 0.0f;
	mShake.z = 0.0f;
	mFade    = 1.0f;
	mPane.setPose(&mPos, mAngle);
	unwatch();
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
	fxHolderStop(&mGlow);
	if (mChild != 0) {
		mChild->reset();
	}
}

// 0x8011DB0C
void TEneOrangeBossMirror::clearHit()
{
	mMirrorFlags &= ~MIRROR_HIT;
}

// 0x8011DB1C
u32 TEneOrangeBossMirror::wasHit()
{
	return mMirrorFlags & MIRROR_HIT;
}

// 0x8011DB28
u16 TEneOrangeBossMirror::hitBy()
{
	return mHitBy;
}

// 0x8011DB30
void TEneOrangeBossMirror::watch()
{
	mMirrorFlags |= MIRROR_WATCH;
	mPane.clearShapeBit8();
}

// 0x8011DB64
void TEneOrangeBossMirror::unwatch()
{
	mMirrorFlags &= ~MIRROR_WATCH;
	mPane.clearContacts();
}

// 0x8011DB98
int TEneOrangeBossMirror::isWhole()
{
	return mBreak == BREAK_WHOLE;
}

// 0x8011DBA8
void TEneOrangeBossMirror::setFlag1()
{
	mMirrorFlags |= MIRROR_FLAG1;
}

// 0x8011DBB8
void TEneOrangeBossMirror::clearFlag1()
{
	mMirrorFlags &= ~MIRROR_FLAG1;
}

// 0x8011DBC8
void TEneOrangeBossMirror::restore()
{
	mMirrorFlags |= MIRROR_RESTORE;
}

// 0x8011DBD8
void TEneOrangeBossMirror::setGrid(int grid)
{
	mGrid = grid;
}

// 0x8011DBE0
void TEneOrangeBossMirror::rippleAt(const Vec3f* pos)
{
	Vec3f d;
	d.x = pos->x;
	d.y = pos->y;
	d.z = pos->z;

	njSubVector(&d, &mPos);
	vecRotateY(&d, &d, -mAngle);
	if (mChild != 0) {
		mChild->touch(d.x, d.y);
	}
}

// 0x8011DC58
void TEneOrangeBossMirror::raise()
{
	mMode = MODE_RAISE;
}

// 0x8011DC64
void TEneOrangeBossMirror::lower()
{
	mMode = MODE_LOWER;
}

// 0x8011DC70
int TEneOrangeBossMirror::isMode(int mode)
{
	return mMode == mode;
}

// 0x8011DC84
int TEneOrangeBossMirror::isStarting()
{
	if (mMode == MODE_RAISE) {
		return mPrevMode == MODE_DOWN || mPrevMode == MODE_LOWER;
	}
	if (mMode == MODE_LOWER) {
		return mPrevMode == MODE_UP || mPrevMode == MODE_RAISE;
	}
	return 0;
}

// 0x8011DCE8
void TEneOrangeBossMirror::meshOn()
{
	mPane.meshOn();
}

// 0x8011DD0C
void TEneOrangeBossMirror::meshRefresh()
{
	mPane.meshRefresh();
}

// 0x8011DD30
void TEneOrangeBossMirror::forceBreak()
{
	mMirrorFlags |= MIRROR_BREAK;
}

// 0x8011DD40
void TEneOrangeBossMirror::turn(int angle)
{
	mAngle = angle;
	mPane.setPose(&mPos, angle);
}

// 0x8011DD74
void TEneOrangeBossMirror::dust()
{
	Vec3f at;

	at.x = mPos.x;
	at.y = mPos.y;
	at.z = mPos.z;
	at.y = 50.0f;
	particleKeepAtRot((void**)&mFx, &at, 0, mAngle + 0x4000, 1, 0x51);
}

// 0x8011DDD4
void TEneOrangeBossMirror::draw()
{
	updateMode();
	updateBreak();
	if (mMirrorFlags & MIRROR_BUSY || mMirrorFlags & MIRROR_DOWN) {
		mPane.disable();
	} else {
		mPane.enable();
	}
	if (!isMode(MODE_RAISE) && !isMode(MODE_LOWER)) {
		if (mPane.isShapeBit8()) {
			Vec3f at;
			njSinCos(mAngle, &at.x, &at.z);
			gjScaleVector(30.0f, &at, &at);
			at.y = 50.0f;
			njAddVector(&at, &mPos);
			mGlow.start(5, 0xB, &at, 0, mAngle);
			seHolderPlay(&mRing, 0x14E, &at);
		} else {
			fxHolderStop(&mGlow);
			seHolderStop(&mRing);
		}
	} else {
		fxHolderStop(&mGlow);
		seHolderStop(&mRing);
	}
}

// 0x8011DEF0
void TEneOrangeBossMirror::updateMode()
{
	mMirrorFlags &= ~MIRROR_DOWN;
	switch (mMode) {
	case MODE_UP:
		mPos.y = 50.0f;
		break;
	case MODE_RAISE:
		mPos.y += 2.0f;
		if (mPos.y >= 50.0f) {
			mPos.y = 50.0f;
			mMode  = MODE_UP;
			if (mFx != 0) {
				mFx->mFlags |= 1;
				mFx = 0;
			}
		} else {
			dust();
		}
		mPane.setPose(&mPos, mAngle);
		break;
	case MODE_LOWER:
		mPos.y -= 2.0f;
		if (mPos.y <= -60.0f) {
			mPos.y = -60.0f;
			mMode  = MODE_DOWN;
			if (mFx != 0) {
				mFx->mFlags |= 1;
				mFx = 0;
			}
		} else {
			dust();
		}
		mPane.setPose(&mPos, mAngle);
		break;
	case MODE_DOWN:
		mMirrorFlags |= MIRROR_DOWN;
		restore();
		mPos.y = -60.0f;
		break;
	}
	mPrevMode = mMode;
}

// -1 to 1 Its constants are pooled ahead of the caller's
static inline f32 randSigned()
{
	return 2.0f * ((f32)rand() * (1.0f / 32768.0f) - 0.5f);
}

// 0x8011E050
void TEneOrangeBossMirror::updateBreak()
{
	if (mMirrorFlags & MIRROR_RESTORE) {
		mMirrorFlags &= ~MIRROR_RESTORE;
		mBreak = BREAK_RESTORE;
	}
	switch (mBreak) {
	case BREAK_WHOLE: {
		int hit = 0;
		mMirrorFlags &= ~MIRROR_BUSY;
		if (mMirrorFlags & MIRROR_BREAK) {
			hit = 2;
		} else if (mPane.checkHit(this)) {
			if (mMirrorFlags & MIRROR_FLAG1) {
				hit = 1;
			} else {
				hit = 2;
			}
		}
		if (hit == 1) {
			seStartAt(0x154, &mPos);
		} else if (hit == 2) {
			mBreak = BREAK_CRACK;
			mMirrorFlags |= MIRROR_BUSY;
			mTimer = 10;
		}
		break;
	}
	case BREAK_CRACK:
		mMirrorFlags |= MIRROR_BUSY;
		mShake.x = 10.0f * randSigned();
		mShake.y = 5.0f * randSigned();
		mShake.z = 10.0f * randSigned();
		mTimer--;
		if (mTimer <= 0) {
			mBreak = BREAK_BROKEN;
			mTimer = 240;
			seStartAt(0x153, &mPos);
			mFade = 0.0f;
			new TEneOrangeBossMirror_Frag(this, &mPos, mAngle);
		}
		break;
	case BREAK_BROKEN:
		mMirrorFlags |= MIRROR_BUSY;
		if (!(mMirrorFlags & MIRROR_FLAG1)) {
			mTimer--;
			if (mTimer <= 0) {
				mFade  = 0.0f;
				mBreak = BREAK_RESTORE;
			}
		}
		break;
	case BREAK_RESTORE:
		mMirrorFlags |= MIRROR_BUSY;
		mFade += 0.05f;
		if (mFade >= 1.0f) {
			mFade  = 1.0f;
			mBreak = BREAK_WHOLE;
			mMirrorFlags &= ~MIRROR_BUSY;
		}
		break;
	}
	mMirrorFlags &= ~MIRROR_BREAK;
	if (mChild != 0) {
		mChild->update();
	}
}

// 0x8011E328
void TEneOrangeBossMirror::vfn_10()
{
	if (!mLight.isVisible(mDraw, 3000.0f, &mPos, 200.0f, 0)) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x8011E390
// The texture matrices and blending the glass is drawn with the screen copy shifted by scroll
extern "C" void mirrorGlassSetup(const Vec3f* pos, int angle, u8 alpha, const Vec3f* scroll)
{
	f32 mtx[14];

	njSetTexture(&sMirrorScreenList);
	gjLoadTextureNum(0, 0, 0, 0);
	njPushMatrixEx();
	njTranslateEx(pos);
	gjRotateY(angle);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	texFxScreenMtx(mtx);
	if (scroll != 0) {
		mtxTranslateV(mtx, mtx, scroll);
	}
	GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX3x4);
	njPopMatrixEx();
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	if (alpha < 0xFF) {
		GXSetAlphaCompare(GX_GREATER, 0x14, GX_AOP_AND, GX_LEQUAL, 0xFF);
		GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_DISABLE);
		GXSetZCompLoc(GX_FALSE);
	} else {
		GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
		GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
		GXSetZCompLoc(GX_TRUE);
	}
}

// 0x8011E534
// The glass as one quad 100 wide and 100 high
extern "C" void mirrorGlassQuad(const Vec3f* pos, int angle, u8 alpha, const Vec3f* scroll)
{
	u32 color;

	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	mirrorGlassSetup(pos, angle, alpha, scroll);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
	color = alpha | 0xFFFFFF00;
	f32 left   = -50.0f;
	f32 top    = 100.0f;
	f32 bottom = 0.0f;
	f32 right  = 50.0f;
	GXPosition3f32(left, top, bottom);
	GXColor1u32(color);
	GXPosition3f32(right, top, bottom);
	GXColor1u32(color);
	GXPosition3f32(left, bottom, bottom);
	GXColor1u32(color);
	GXPosition3f32(right, bottom, bottom);
	GXColor1u32(color);
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}

// 0x8011E668
// The glass as a grid of grid x grid tiles each showing the screen a little shifted
extern "C" void mirrorGlassGrid(const Vec3f* pos, int angle, u8 alpha, int grid)
{
	u32 color;
	f32 shift;
	f32 zero;
	f32 step;
	f32 x;
	f32 x1;
	f32 y;
	f32 y0;
	f32 u1;
	f32 t0;
	f32 off;
	int i;
	int j;

	mirrorGlassSetup(pos, angle, alpha, 0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	color = alpha | 0xFFFFFF00;
	if (grid > 32) {
		shift = (f32)(grid - 32) / 8.0f;
		grid  = 32;
	} else {
		shift = 0.0f;
	}
	zero = 0.0f;
	step = 100.0f / (f32)grid;
	off  = step * shift;
	y    = step;
	for (i = 0; i < grid; i++) {
		y0 = y - step;
		t0 = y - off;
		x      = -50.0f;
		for (j = 0; j < grid; j++) {
			x1 = x + step;
			u1 = x + off;
			GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
			MIRROR_FIFO_F32 = x;
			MIRROR_FIFO_F32 = y0;
			MIRROR_FIFO_F32 = zero;
			MIRROR_FIFO_F32 = x;
			MIRROR_FIFO_F32 = t0;
			MIRROR_FIFO_F32 = zero;
			MIRROR_FIFO_U32 = color;
			MIRROR_FIFO_F32 = x;
			MIRROR_FIFO_F32 = y;
			MIRROR_FIFO_F32 = zero;
			MIRROR_FIFO_F32 = x;
			MIRROR_FIFO_F32 = y;
			MIRROR_FIFO_F32 = zero;
			MIRROR_FIFO_U32 = color;
			MIRROR_FIFO_F32 = x1;
			MIRROR_FIFO_F32 = y0;
			MIRROR_FIFO_F32 = zero;
			MIRROR_FIFO_F32 = u1;
			MIRROR_FIFO_F32 = t0;
			MIRROR_FIFO_F32 = zero;
			MIRROR_FIFO_U32 = color;
			MIRROR_FIFO_F32 = x1;
			MIRROR_FIFO_F32 = y;
			MIRROR_FIFO_F32 = zero;
			MIRROR_FIFO_F32 = u1;
			MIRROR_FIFO_F32 = y;
			MIRROR_FIFO_F32 = zero;
			MIRROR_FIFO_U32 = color;
			x = x + step;
		}
		y = y + step;
	}
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}

// 0x8011E90C
// The rippling glass 15 strips of the ripple's 16 x 16 points
extern "C" void mirrorGlassRipple(const Vec3f* pos, int angle, MirrorRipple* ripple)
{
	int row;
	int col;
	f32 zero;

	mirrorGlassSetup(pos, angle, 0xFF, 0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	zero = 0.0f;
	for (row = 0; row < 15; row++) {
		RippleVert* a;
		RippleVert* b;
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x20);
		a = ripple->mVerts[row];
		b = a + 16;
		for (col = 0; col < 16; col++) {
			GXPosition3f32(b->mX, b->mY, zero);
			GXNormal3f32(b->mU, b->mV, zero);
			GXColor1u32(0xFFFFFFFF);
			b++;
			GXPosition3f32(a->mX, a->mY, zero);
			GXNormal3f32(a->mU, a->mV, zero);
			GXColor1u32(0xFFFFFFFF);
			a++;
		}
	}
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}

// 0x8011EAA0
// The frame then the glass
void TEneOrangeBossMirror::render()
{
	Vec3f at;

	shadowAlphaBegin();
	njSetTexture(*(NJS_TEXLIST**)sOrangeBossMapTextures);
	njPushMatrixEx();
	at.x = mPos.x;
	at.y = mPos.y;
	at.z = mPos.z;
	at.y = mPos.y - 10.1f;
	njTranslateEx(&at);
	gjRotateY(mAngle);
	mLight.drawObject(sOrangeBossMapModels[3]);
	njPopMatrixEx();
	switch (mBreak) {
	case BREAK_WHOLE:
	case BREAK_CRACK:
		if (mGrid < 40) {
			mirrorGlassGrid(&mPos, mAngle, 0xFF, mGrid);
		} else if (mChild != 0 && !mChild->isCalm()) {
			mirrorGlassRipple(&mPos, mAngle, mChild);
		} else {
			if (mBreak == BREAK_CRACK) {
				mirrorGlassQuad(&mPos, mAngle, 0xFF, &mShake);
			} else {
				mirrorGlassQuad(&mPos, mAngle, 0xFF, 0);
			}
		}
		break;
	case BREAK_BROKEN:
		break;
	case BREAK_RESTORE:
		mirrorGlassQuad(&mPos, mAngle, (u8)(int)(255.0f * mFade), 0);
		break;
	}
	shadowAlphaEnd();
}

// 0x8011EC04
Vec3f* TEneOrangeBossMirror::getPos()
{
	return &mPos;
}

// 0x8011EC0C
int TEneOrangeBossMirror::getKind()
{
	return 2;
}
