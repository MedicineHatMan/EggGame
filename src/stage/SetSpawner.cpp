#include "stage/SetManager.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjPlayer.h"
#include "stage/TEventCtrl.h"
#include "stage/SetSpawner.h"
#include "core/TMainTask.h"
#include "engine/DrawTask.h"
struct TObjAction;
struct TObjPlayer;

// The object that keeps stage objects spawning around a centre point a slice of each placement file per frame
// setSpawnerMode to setSpawnerOff switch where the centre point comes from

enum SetCentreMode {
	SET_CENTRE_PLAYER = 0,
	SET_CENTRE_CAMERA = 1,
	SET_CENTRE_NONE   = 2, // spawn everything whatever its range
	SET_CENTRE_OFF    = 3, // no spawning
};

extern "C" void createSetSpawner()
{
	if (gSetGenerator == 0) {
		gSetGenerator = new (gObjHeap) TSetGenerator(gTL_02);
	}
}

extern "C" int setSpawnerMode(int mode)
{
	if (gSetGenerator != 0) {
		int old              = gSetGenerator->mMode;
		gSetGenerator->mMode = mode;
		return old;
	}
	return 0;
}

extern "C" int setSpawnerCamera()
{
	return setSpawnerMode(SET_CENTRE_CAMERA);
}

extern "C" void setSpawnerNone()
{
	setSpawnerMode(SET_CENTRE_NONE);
}

extern "C" void setSpawnerOff()
{
	setSpawnerMode(SET_CENTRE_OFF);
}

TSetGenerator::TSetGenerator(TObject* parent)
    : TObject(parent)
{
	_00         = (u32)gTSetGeneratorName;
	mSize       = sizeof(TSetGenerator);
	mCursors[0] = 0;
	mCursors[1] = 0;
	mCursors[2] = 0;
	mMode       = SET_CENTRE_PLAYER;
	mFullScans  = 5;
}

TSetGenerator::~TSetGenerator()
{
	if (gSetGenerator == this) {
		gSetGenerator = 0;
	}
}

void TSetGenerator::draw()
{
	Vec3f pos;
	Vec3f* centre;
	int count;
	int capacity;

	if (mMode == SET_CENTRE_OFF) {
		return;
	}

	switch (mMode) {
	case SET_CENTRE_PLAYER: {
		TObjAction* player = (TObjAction*)findPlayer(0);
		if (player != 0) {
			pos = *player->getPos();
		} else {
			pos.x = 0.0f;
			pos.y = 0.0f;
			pos.z = 0.0f;
		}
		centre = &pos;
		break;
	}
	case SET_CENTRE_CAMERA:
		camGetPos(0, &pos);
		centre = &pos;
		setRangeCenter(centre);
		break;
	case SET_CENTRE_NONE:
		centre = 0;
		break;
	}

	tickSetManager();
	if (eventIsBusy() != 0) {
		mFullScans = 6;
	}

	if (mFullScans > 0) {
		getSetObjCounts(&count, &capacity);
		mCursors[0] = scanSetObj(centre, 0, count);
		getSetEnemyCounts(&count, &capacity);
		mCursors[1] = scanSetEnemy(centre, 0, count);
		getSetDesignCounts(&count, &capacity);
		mCursors[2] = scanSetDesign(centre, 0, count);
	} else {
		mCursors[0] = scanSetObj(centre, mCursors[0], 30);
		mCursors[1] = scanSetEnemy(centre, mCursors[1], 30);
		mCursors[2] = scanSetDesign(centre, mCursors[2], 30);
	}

	mFullScans--;
	if (mFullScans < 0) {
		mFullScans = 0;
	}
}
