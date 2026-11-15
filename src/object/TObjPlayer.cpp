#include "player/Player.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "engine/Light.h"
#include "ui/AeArc.h"
#include "sound/SoundCtrl.h"
#include "io/Arc.h"
#include "object/TObjPlayer.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "core/Mem.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "io/ResManager.h"
#include "core/SdkCalls.h"
#include "Ninja/ninja.h"
struct ObjMsg;
struct NJS_TEXLIST;

struct RestartPoint {
	u16 mId;    // _00
	u16 mDir;   // _02
	Vec3f mPos; // _04
};

struct RestartList {
	RestartList();  // 0x80012C6C
	~RestartList(); // 0x80012C80

	RestartPoint* mData; // _00
	int mCount;          // _04
	int mCapacity;       // _08
};

extern "C" {
const char* gTObjPlayerName = "TObjPlayer";
}

// The data every player shares this unit's .bss from 0x80279990 and a flag word in .sbss
TArc* gPlayerArcs[5];         // 0x80279990 each character's archive
Vec3f sPlayerScreenPos;       // 0x802799A4 the goal
HitVec sPlayerStartPos[4];    // 0x802799B0
int sPlayerStartAngle[4];     // 0x802799E0
static RestartList sRestarts; // 0x802799FC
static void* sParams[4];      // 0x80279A08
FlagWord gPlayerFlags;        // 0x802C4BE0

// 0x80012C6C
RestartList::RestartList()
{
	mData     = NULL;
	mCount    = 0;
	mCapacity = 0;
}

// 0x80012C80
RestartList::~RestartList()
{
	if (mData != NULL && mData != NULL) {
		::operator delete[]((u8*)mData - 0x10);
	}
}

// 0x80012CDC
void restartListClear(RestartList* list)
{
	if (list->mData != NULL) {
		if (list->mData != NULL) {
			::operator delete[]((u8*)list->mData - 0x10);
		}
		list->mData = NULL;
	}
	list->mCapacity = 0;
	list->mCount    = 0;
}

// 0x80012D30
void restartListCopy(RestartList* list, RestartPoint* to, const RestartPoint* from, int count)
{
	for (int i = 0; i < count; i++) {
		to[i].mId  = from[i].mId;
		to[i].mDir = from[i].mDir;
		to[i].mPos = from[i].mPos;
	}
}

// 0x80012D7C
void restartListGrow(RestartList* list, int count)
{
	void* mem       = ::operator new[]((count << 4) + 0x10);
	list->mData     = (RestartPoint*)__construct_new_array(mem, (void*)restartPointCtor, NULL, 0x10, count);
	list->mCapacity = count;
}

// 0x80012DD4
void restartPointCtor()
{
}

// 0x80012DD8
void restartListReserve(RestartList* list, int count)
{
	if (count > list->mCapacity) {
		RestartPoint* old = list->mData;
		restartListGrow(list, count);
		if (old != NULL) {
			restartListCopy(list, list->mData, old, list->mCount);
			if (old != NULL) {
				::operator delete[]((u8*)old - 0x10);
			}
		}
	}
}

// 0x80012E40
void restartListAdd(RestartList* list, const RestartPoint* point)
{
	if (list->mCount + 1 > list->mCapacity) {
		restartListReserve(list, list->mCount + list->mCount + 2);
	}
	RestartPoint* to = &list->mData[list->mCount];
	to->mId          = point->mId;
	to->mDir         = point->mDir;
	to->mPos         = point->mPos;
	list->mCount++;
}

// 0x80012ED0
TObjPlayer::TObjPlayer(TObject* parent, u16 id, u8 charId)
    : TObjAction(parent)
{
	mCharId = charId;
	_34     = 0;
	_00     = (u32)gTObjPlayerName;
	mSize   = 0x3C;
	setObjId(this, (u16)id);
	mStatus = new LightSet;
}

// 0x80012F58
TObjPlayer::~TObjPlayer()
{
	releaseObjId(this);
	delete mStatus;
}

// 0x80012FDC
void playerLoadShared()
{
	sDrawViewPlayerId = 0;
	clearFlagWord(&gPlayerFlags.mBits);
	loadSizedResource("ge_playerparam.bin", &sPlayerParamFile);
	sParams[0]        = sPlayerParamFile->mData;
	sPlayerParamsTail = (u8*)sPlayerParamFile->mData + 0xF8;
	loadArc("ge_player.arc", &gPlayerArc);
	if (gPlayerArc != NULL) {
		void** texlists = (void**)findArcExport(gPlayerArc, "texlists");
		void** textures = (void**)findArcExport(gPlayerArc, "textures");
		numberTextures((NJS_TEXLIST*)*texlists);
		bindGvmTextures((u8*)*textures, (NJS_TEXLIST*)*texlists);
		sPlayerTexLists2 = texlists;
	}
	plyLoadShared();
	restartListReserve(&sRestarts, 0x20);
	for (int i = 0; i < 5; i++) {
		gPlayerArcs[i] = NULL;
	}
	gBattleGroups[0] = 8;
	gBattleGroups[1] = 8;
	gBattleGroups[2] = 8;
	gBattleGroups[3] = 8;
}

// 0x800130D4
void playerFreeShared()
{
	plyUnloadShared();
	releaseTextures((NJS_TEXLIST*)*sPlayerTexLists2);
	releaseResource(gPlayerArc);
	releaseResource(sPlayerParamFile);
	restartListClear(&sRestarts);
	for (int i = 0; i < 5; i++) {
		playerFreeArc(i);
	}
}

// 0x8001313C
void playerLoadArc(int no, const char* name)
{
	TArc* arc;
	loadArc(name, &arc);
	if (arc != NULL) {
		gPlayerArcs[no] = arc;
		arc->linkFrom(gPlayerArc);
		if (no == 4) {
			arc->linkFrom(gPlayerArcs[0]);
		}
		void** texlists = (void**)findArcExport(arc, "texlists");
		if (texlists != NULL) {
			numberTextures((NJS_TEXLIST*)*texlists);
			void** textures = (void**)findArcExport(arc, "textures");
			bindGvmTextures((u8*)*textures, (NJS_TEXLIST*)*texlists);
		}
	}
}

// 0x802523E0
// The archives and voice banks of the four characters
const char* sPlayerArchives[5] = {
	"ge_player1.arc", "ge_player2.arc", "ge_player3.arc", "ge_player4.arc", "ge_player1n.arc",
};
const char* sPlayerArchives2[5] = {
	"ge_player1l.arc", "ge_player2l.arc", "ge_player3l.arc", "ge_player4l.arc", "ge_player1n.arc",
};
const char* sPlayerSoundBanks[5] = {
	"voice1p.mpb", "voice2p.mpb", "voice3p.mpb", "voice4p.mpb", "voice1p.mpb",
};

// 0x800131EC
void playerEnsureArc(int no)
{
	if (gPlayerArcs[no] == NULL) {
		playerLoadArc(no, sPlayerArchives[no]);
	}
}

// 0x80013230
void playerEnsureBattleArc(int no)
{
	if (gPlayerArcs[no] == NULL) {
		playerLoadArc(no, sPlayerArchives2[no]);
	}
}

// 0x80013274
void playerFreeArc(int no)
{
	TArc* arc = gPlayerArcs[no];
	if (arc != NULL) {
		gPlayerArcs[no] = NULL;
		njReleaseTexture(*(NJS_TEXLIST**)findArcExport(arc, "texlists"));
		releaseResource(arc);
	}
}

// 0x800132D8
void playerLoadSoundBank(int no, int bank)
{
	soundLoadBank(bank, sPlayerSoundBanks[no]);
}

// 0x8001330C
void* playerCreateBattle(TObject* parent, int no, u8 charId)
{
	return new TChrPlayer(parent, no & 0xFFFF, charId, 1);
}

// 0x8001336C
void* playerCreate(TObject* parent, int no, u8 charId, int active)
{
	return new TChrPlayer(parent, no & 0xFFFF, charId, active);
}

// 0x800133D0
void playerSetStart(int no, const Vec3f* pos, int dir)
{
	sPlayerStartPos[no & 0xFFFF]   = *pos;
	sPlayerStartAngle[no & 0xFFFF] = dir;
}

// 0x80013410
TObjPlayer* findPlayer(int id)
{
	TObjAction* obj = (TObjAction*)findObjById(id & 0xFFFF);
	if (obj == NULL) {
		return NULL;
	}
	if (obj->getKind() != 0) {
		return NULL;
	}
	return (TObjPlayer*)obj;
}

// 0x80013470
void playerSetDrawView(int value)
{
	sDrawViewPlayerId = value;
}

// 0x80013478
int playerGetDrawView()
{
	return sDrawViewPlayerId;
}

// 0x80013480
int playerGetGoal(Vec3f* out)
{
	if (gPlayerGoalVisible != 0) {
		if (out != NULL) {
			*out = sPlayerScreenPos;
		}
		return 1;
	}
	return 0;
}

// 0x800134C0
void playerAddLight(int id, LightBase* light)
{
	findPlayer(id)->mStatus->add(light);
}

// 0x800134F8
void playerRemoveLight(int id, LightBase* light)
{
	TObjPlayer* player = findPlayer(id);
	if (player != NULL) {
		player->mStatus->remove(light);
	}
}

struct RestartIter {
	RestartIter(RestartPoint* p) { mPtr = p; }
	bool operator!=(const RestartIter& other) const { return mPtr != other.mPtr; }
	RestartPoint& operator*() const { return *mPtr; }
	RestartIter& operator++()
	{
		mPtr++;
		return *this;
	}

	RestartPoint* mPtr; // _00
};

static inline RestartIter restartFind(RestartIter first, RestartIter last, const u16& id)
{
	while (first != last && first.mPtr->mId != id) {
		++first;
	}
	return first;
}

// 0x80013538
// Adds a restart point or moves the one with this id
void restartPointSet(int id, const Vec3f* pos, int dir)
{
	RestartPoint* end = sRestarts.mData + sRestarts.mCount;
	RestartIter found = restartFind(RestartIter(sRestarts.mData), RestartIter(end), id);
	if (found.mPtr != end) {
		RestartPoint* it = found.mPtr;
		it->mDir         = dir;
		it->mPos         = *pos;
	} else {
		RestartPoint point;
		point.mId  = id;
		point.mDir = dir;
		point.mPos = *pos;
		restartListAdd(&sRestarts, &point);
	}
}

// 0x8001360C
// Makes a restart point the player's start
void restartPointUse(int no, int id)
{
	RestartPoint* end = sRestarts.mData + sRestarts.mCount;
	RestartPoint* it  = restartFind(RestartIter(sRestarts.mData), RestartIter(end), id).mPtr;
	if (it != end) {
		sPlayerStartPos[no & 0xFFFF]   = it->mPos;
		sPlayerStartAngle[no & 0xFFFF] = it->mDir;
	}
}

// 0x800136B8
MobileParams* playerGetParams(int player)
{
	return (MobileParams*)sParams[0];
}

// 0x800136C8
void* playerGetParamsTail()
{
	return sPlayerParamsTail;
}

// 0x800136D0
int evLoadPlayer(u8* no)
{
	if (*no < 1 || *no > 5) {
		return 1;
	}
	playerEnsureArc(*no - 1);
	return 1;
}

// 0x80013714
int evTakeoffSuit()
{
	u16 msg = 0x85;
	sendMsgById(0, (ObjMsg*)&msg);
	return 1;
}

// 0x80013748
int TObjPlayer::vfn_6C()
{
	return 0;
}

// 0x80013750
void TObjPlayer::vfn_70(int arg)
{
}

// 0x80013754
void TObjPlayer::vfn_74(int arg)
{
}

// 0x80013758
int TObjPlayer::vfn_78()
{
	return 0;
}

// 0x80013760
void TObjPlayer::vfn_7C()
{
}

// 0x80013764
void TObjPlayer::vfn_80()
{
}

// 0x80013768
int TObjPlayer::vfn_84()
{
	return 0;
}

// 0x80013770
int TObjPlayer::vfn_88()
{
	return 0;
}

// 0x80013778
int TObjPlayer::vfn_8C()
{
	return 0;
}

// 0x80013780
void TObjPlayer::vfn_90(ItemSpec* spec)
{
}

// 0x80013784
void TObjPlayer::vfn_94(ItemSpec* spec)
{
}

// 0x80013788
void TObjPlayer::vfn_98()
{
}

// 0x8001378C
void TObjPlayer::vfn_9C(int arg0, int arg1)
{
}

// 0x80013790
void TObjPlayer::vfn_A8(int joint, f32* mtx)
{
}

// 0x80013794
void TObjPlayer::vfn_AC(int joint, f32* mtx) const
{
}

// 0x80013798
void TObjPlayer::vfn_B0(const int* motions)
{
}

// 0x8001379C
void TObjPlayer::vfn_B4(int on)
{
}

// 0x800137A0
void TObjPlayer::resetCamera()
{
}

// 0x800137A4
void TObjPlayer::vfn_BC(const int* motions, f32* mtx)
{
}

// 0x800137A8
void TObjPlayer::vfn_C0(const Vec3f* pos, const EggRot* rot)
{
}

// 0x800137AC
void TObjPlayer::vfn_C4(const int* motions, f32* mtx)
{
}

// 0x800137B0
void TObjPlayer::vfn_C8(const int* motions, f32* mtx)
{
}

// 0x800137B4
void TObjPlayer::vfn_CC()
{
}

// 0x800137B8
void TObjPlayer::vfn_D0()
{
}

// 0x800137BC
int TObjPlayer::vfn_D4()
{
	return 0;
}

// 0x800137C4
int TObjPlayer::vfn_D8()
{
	return 0;
}

// 0x800137CC
int TObjPlayer::getKind()
{
	return 0;
}

// 0x800137D4
void clearFlagWord(u32* word)
{
	*word = 0;
}
