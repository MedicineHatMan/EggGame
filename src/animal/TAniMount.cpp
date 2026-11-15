#include "animal/AniMount.h"
#include "camera/TCameraCtrl.h"
#include "player/Player.h"
#include "engine/GroundQuery.h"
#include "stage/TGameState.h"
#include "core/Pad.h"
#include "sound/Sound3DPlay.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
struct ObjMsg;

struct AniMountMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mFrom; // _04
	u16 mSelf; // _06
	int _08;   // _08
	int _0C;   // _0C
};

struct AniMountRequest {
	u16 mType; // _00
	u8 _02[2];
	u16 mFrom; // _04
};

// 0x801C5674
TAniMount::TAniMount(TObject* parent, int kind)
    : TEggCreature(parent, 0)
    , mLoaded(0)
    , mState(-1)
    , mKind(kind)
{
	aniAssetAttach(kind, &mAsset);
}

// 0x801C56F0
TAniMount::~TAniMount()
{
	if (mAsset.mLink.mList != 0) {
		aniAssetDetach(&mAsset);
	}
}

// 0x801C5764
void TAniMount::setPose(const Vec3f* pos, int rot)
{
	mStartPos = *pos;
	mStartRot = rot;
}

// 0x801C5784
void TAniMount::setState(int state)
{
	if (mState != -1) {
		runState(mState, 1);
	}
	mState = state;
	if (state != -1) {
		runState(state, 0);
	}
}

// 0x801C57F8
void TAniMount::vfn_78()
{
	onDismount();
	const s8 player = mPlayer;
	camSetTarget(player, findObjById(mHolderId));
	mHolderId = 0xFFFF;
}

// 0x801C5858
int TAniMount::mountMsg(ObjMsg* msg)
{
	if (canMount() == 0) {
		return 0;
	}
	mHolderId = ((AniMountRequest*)msg)->mFrom;
	mPlayer   = findObjById(mHolderId)->mPlayer;
	camSetTarget((s8)mPlayer, this);
	if (flagTest(&mMountFlags, 1) == 0) {
		flagWordSetBit(&mMountFlags, 1);
		AniMountMsg notify;
		notify._08   = 0;
		notify._0C   = 0;
		notify.mType = 0xA1;
		notify.mFrom = mHolderId;
		notify.mSelf = mObjId;
		notify._0C   = 1;
		sendGameMsg((ObjMsg*)&notify);
	}
	onMount();
	return 1;
}

// 0x801C5938
int TAniMount::dismountMsg(ObjMsg* msg)
{
	if (((AniMountRequest*)msg)->mFrom != mHolderId) {
		return 0;
	}
	if (canDismount() == 0) {
		return 0;
	}
	onDismount();
	const s8 player = mPlayer;
	camSetTarget(player, findObjById(mHolderId));
	mHolderId = 0xFFFF;
	return 1;
}

// 0x801C59D8
void TAniMount::draw()
{
	switch (mLoaded) {
	case 0:
		if (mAsset.mLink.mList != 0) {
			onLoaded();
			mLoaded = 1;
		}
		break;
	case 1: {
		TObjAction* rider = findObjById(mHolderId);
		if (rider != 0) {
			f32 camera[12];
			mtxSetRotY(camera, camGetYaw((s8)rider->mPlayer));
			mInput.read(rider->mObjId, camera);
		}
		if (flagTest(&mMountFlags, 0) == 0) {
			preUpdate();
		}
		if (flagTest(&mMountFlags, 0) == 0) {
			runState(mState, 2);
		}
		if (flagTest(&mMountFlags, 0) == 0) {
			runState(mState, 3);
		}
		if (flagTest(&mMountFlags, 0) == 0) {
			postUpdate();
		}
		break;
	}
	case 2:
		break;
	}
}

// 0x801C5B30
void TAniMount::vfn_7C()
{
	die();
}

// 0x801C5B50
int TAniMount::vfn_88()
{
	return 1;
}

// 0x801C5B58
void TAniMount::rumble(s16 id)
{
	int rider = mHolderId;
	if (rider < 0 || rider >= 4) {
		return;
	}
	padVibStart(rider, 0, 0x64, 0, id);
}

// 0x801C5BA4
extern "C" void aniMountSound(void*, int id, const Vec3f* pos)
{
	sePlay3D(id, (Vec3f*)pos);
}

// 0x801C5BCC
void TAniMount::die()
{
	Vec3f pos;
	pos = *getPos();
	f32 matrix[12];
	mtxSetTransV(matrix, &pos);
	fxSpawnAt(1, 9, 6, matrix);
	TObjAction* rider = findObjById(mHolderId);
	if (rider != 0) {
		camSetTarget((s8)mPlayer, rider);
	}
	mFlags |= 1;
	flagWordSetBit(&mMountFlags, 0);
}

// 0x801C5C74
void TAniMount::runState(int, int)
{
}

// 0x801C5C78
void TAniMount::onDismount()
{
}

// 0x801C5C7C
void TAniMount::onMount()
{
}

// 0x801C5C80
int TAniMount::canMount() const
{
	return 0;
}

// 0x801C5C88
int TAniMount::canDismount()
{
	return 0;
}

// 0x801C5C90
void TAniMount::postUpdate()
{
}

// 0x801C5C94
void TAniMount::preUpdate()
{
}

// 0x801C5C98
void TAniMount::onLoaded()
{
}
