#include "player/TPlyDemoLastLose.h"
#include "io/Resource.h"
#include "camera/TCamera.h"

#include "player/Player.h"
#include "stage/TEventCtrl.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "sound/SoundCtrl.h"
#include "object/TObjPlayer.h"
struct TObjPlayer;

typedef char TPlyDemoLastLose_size_check[(sizeof(TPlyDemoLastLose) == 0x34) ? 1 : -1];

// 0x801CEBD8
// superBillyLoad hands over the archive
void plyDemoLastLoseBind(TArc* arc)
{
	gLastLoseExports = (PlyDemoExport*)findArcExport(arc, "event_lose");
}

const char* gTPlyDemoLastLoseName = "TPlyDemoLastLose";

// 0x801CEC04
TPlyDemoLastLose::TPlyDemoLastLose(TObject* parent)
    : TObject(parent)
{
	mEvent = 0;
	mTimer = 0;
	_00    = (u32)gTPlyDemoLastLoseName;
	mSize  = 0x34;
	mStep  = &TPlyDemoLastLose::stepIntro;
}

// 0x801CEC7C
TPlyDemoLastLose::~TPlyDemoLastLose()
{
	if (mEvent != 0) {
		mEvent->mFlags |= 1;
	}
}

// 0x801CECF4
void TPlyDemoLastLose::draw()
{
	(this->*mStep)();
}

// 0x801CED1C
void TPlyDemoLastLose::stepIntro()
{
	if (eventStartById(0, 0) == 0) {
		return;
	}
	TChrPlayer* player = (TChrPlayer*)findPlayer(0);
	Vec3f pos;
	pos    = *player->getPos();
	mEvent = createCamPath(0, 2, gLastLoseExports->mCamObject, gLastLoseExports->mCamMotion, 0, 0.0f, 0.5f);
	mEvent->attachToMtx();
	f32 mtx[12];
	mtxSetTransV(mtx, &pos);
	mtxCopy(mEvent->getMtx(), mtx);
	int motions[6]; // the motions vfn_C4 plays
	motions[0] = 0;
	motions[1] = 0;
	motions[2] = 0;
	motions[3] = 0;
	motions[4] = 0;
	motions[5] = 0;
	motions[0] = gLastLoseExports->mMotions[0];
	motions[1] = gLastLoseExports->mMotions[1];
	motions[2] = gLastLoseExports->mMotions[2];
	motions[4] = gLastLoseExports->mMotions[3];
	motions[3] = gLastLoseExports->mMotions[4];
	player->vfn_C4(motions, mtx);
	sePlayFor(0x2BD, 0);
	mStep = &TPlyDemoLastLose::stepHold;
}

// 0x801CEE68
void TPlyDemoLastLose::stepHold()
{
	TChrPlayer* player;

	if (mTimer == 0) {
		player = (TChrPlayer*)findPlayer(0);
		player->vfn_D8();
	} else if (mTimer == 0x124) {
		mStep = &TPlyDemoLastLose::stepEnd;
	}
	mTimer++;
}

// 0x801CEEEC
void TPlyDemoLastLose::stepEnd()
{
	ObjMsg msg;

	mEvent->mFlags |= 1;
	mEvent    = 0;
	msg.mType = 0x79;
	sendMsgById(0, &msg);
	eventEnd();
	mFlags |= 1;
}
