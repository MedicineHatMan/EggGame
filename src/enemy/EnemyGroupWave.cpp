#include "stage/Stage.h"
#include "enemy/EnemyGroupWave.h"
#include "enemy/EnemyGroup.h"
#include "object/TObjKey00.h"
#include "core/TMainTask.h"
#include "enemy/ObjEnemy.h"
#include "stage/SetManager.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/boss/EneBoss.h"
#include "egg/EggCreature.h"
#include "engine/Colli.h"
#include "stage/SetSlot.h"
#include "enemy/TEneGroupCollision.h"
#include "enemy/TEneAm01.h"
#include "Ninja/ninja.h"

class TEneGroupStep : public TObject {
public:
	TEneGroupStep(TObject* parent, s16 group, s16 step, int count, int* list);
	virtual ~TEneGroupStep();

	virtual void draw(); // _0C

	int is(s16 group, s16 step);

	TEneGroupStep* mPrev; // _20
	TEneGroupStep* mNext; // _24
	s16 mGroup;           // _28
	s16 mStep;            // _2A
	int mCount;           // _2C slots still alive
	int* mList;           // _30 the slots spawned
};

typedef char TEneGroupStep_size_check[(sizeof(TEneGroupStep) == 0x34) ? 1 : -1];

class TEneGroupWave : public TObject {
public:
	enum {
		WAVE_WAIT  = 0, // until the step is beaten
		WAVE_NEXT  = 1, // a pause then the next step
		WAVE_AGAIN = 2, // a pause then the group from the start
	};

	TEneGroupWave(TObject* parent, s16 group, s16 step, int keyBit, int restart);
	virtual ~TEneGroupWave();

	virtual void draw(); // _0C

	int is(s16 group);

	TEneGroupWave* mPrev; // _20
	TEneGroupWave* mNext; // _24
	s16 mGroup;           // _28
	s16 mStep;            // _2A
	int mKeyBit;          // _2C set once the last step is beaten 0 = none
	int mRestart;         // _30
	int mState;           // _34
	int mTimer;           // _38
};

typedef char TEneGroupWave_size_check[(sizeof(TEneGroupWave) == 0x3C) ? 1 : -1];

TEneGroupStep* sGroupSteps; // 0x802C4EC8
TEneGroupWave* sGroupWaves; // 0x802C4ECC

// 0x80057BB8
extern "C" void enemyGroupStepsEnd()
{
	TEneGroupStep* step = sGroupSteps;
	while (step != 0) {
		TEneGroupStep* next = step->mNext;
		step->mFlags |= 1;
		step = next;
	}
}

// 0x80057BE0
extern "C" int enemyGroupStartStep(s16 group, s16 step)
{
	int result;
	int count;
	if (enemyGroupStepActive(group, step) == 0) {
		int* list = enemyGroupCollect(group, step, &count, 0);
		if (list != 0) {
			enemyGroupSpawn(list, count);
			new TEneGroupStep(gTL_02, group, step, count, list);
			result = 1;
		} else {
			result = 2;
		}
	} else {
		result = 0;
	}
	return result;
}

// 0x80057C7C
extern "C" int enemyGroupTakeStep(s16 group, s16 step)
{
	int result;
	int count;
	if (enemyGroupStepActive(group, step) == 0) {
		int* list = enemyGroupCollect(group, step, &count, 1);
		if (list != 0) {
			enemyGroupSpawn(list, count);
			new TEneGroupStep(gTL_02, group, step, count, list);
			result = 1;
		} else {
			result = 2;
		}
	} else {
		result = 0;
	}
	return result;
}

// 0x80057D18
extern "C" int enemyGroupStepActive(s16 group, s16 step)
{
	for (TEneGroupStep* s = sGroupSteps; s != 0; s = s->mNext) {
		if (s->is(group, step) != 0) {
			return 1;
		}
	}
	return 0;
}

// 0x80057D7C
TEneGroupStep::TEneGroupStep(TObject* parent, s16 group, s16 step, int count, int* list)
    : TObject(parent)
{
	if (sGroupSteps == 0) {
		sGroupSteps = this;
		mPrev       = 0;
		mNext       = 0;
	} else {
		TEneGroupStep* tail = sGroupSteps;
		while (tail->mNext != 0) {
			tail = tail->mNext;
		}
		tail->mNext = this;
		mPrev       = tail;
		mNext       = 0;
	}
	mGroup = group;
	mStep  = step;
	mCount = count;
	mList  = list;
}

// 0x80057E20
TEneGroupStep::~TEneGroupStep()
{
	if (mPrev != 0) {
		mPrev->mNext = mNext;
	}
	if (mNext != 0) {
		mNext->mPrev = mPrev;
	}
	if (sGroupSteps == this) {
		sGroupSteps = mNext;
	}
	if (mList != 0) {
		delete[] mList;
		mList = 0;
	}
}

// 0x80057ED4
int TEneGroupStep::is(s16 group, s16 step)
{
	u8 is = 0;
	if (mGroup == group && mStep == step) {
		is = 1;
	}
	return is;
}

// 0x80057F04
void TEneGroupStep::draw()
{
	enemyGroupPrune(mList, &mCount);
	if (mCount == 0) {
		mFlags |= 1;
	}
}

// 0x80057F50
extern "C" void enemyGroupWavesEnd()
{
	TEneGroupWave* wave;
	TEneGroupWave* next;
	for (wave = sGroupWaves; wave != 0; wave = next) {
		next = wave->mNext;
		wave->mFlags |= 1;
	}
}

// 0x80057F78
extern "C" int enemyGroupStartWave(s16 group, int keyBit, int restart)
{
	s16 step;
	if (enemyGroupWaveActive(group) == 0 && enemyGroupStartFirst(group, &step) != 0) {
		new TEneGroupWave(gTL_04, group, step, keyBit, restart);
		return 1;
	}
	return 0;
}

// 0x80058000
// Lets out the group's first step that has anyone in it
extern "C" int enemyGroupStartFirst(s16 group, s16* out)
{
	s16 step;
	s16 next;
	if (enemyGroupFirstStep(group, &step) != 0) {
		int more = 1;
		while (more) {
			if (enemyGroupStartStep(group, step) != 2) {
				*out = step;
				return 1;
			}
			if (enemyGroupNextStep(group, step, &next) != 0) {
				step = next;
			} else {
				more = 0;
			}
		}
	}
	return 0;
}

// 0x8005809C
extern "C" int enemyGroupWaveActive(s16 group)
{
	for (TEneGroupWave* w = sGroupWaves; w != 0; w = w->mNext) {
		if (w->is(group) != 0) {
			return 1;
		}
	}
	return 0;
}

// 0x800580F8
int TEneGroupWave::is(s16 group)
{
	return mGroup == group;
}

// 0x80058110
TEneGroupWave::TEneGroupWave(TObject* parent, s16 group, s16 step, int keyBit, int restart)
    : TObject(parent)
{
	if (sGroupWaves == 0) {
		sGroupWaves = this;
		mPrev       = 0;
		mNext       = 0;
	} else {
		TEneGroupWave* tail = sGroupWaves;
		while (tail->mNext != 0) {
			tail = tail->mNext;
		}
		tail->mNext = this;
		mPrev       = tail;
		mNext       = 0;
	}
	mGroup   = group;
	mStep    = step;
	mKeyBit  = keyBit;
	mRestart = restart;
	mState   = WAVE_WAIT;
	mTimer   = 0;
}

// 0x800581C0
TEneGroupWave::~TEneGroupWave()
{
	if (mPrev != 0) {
		mPrev->mNext = mNext;
	}
	if (mNext != 0) {
		mNext->mPrev = mPrev;
	}
	if (sGroupWaves == this) {
		sGroupWaves = mNext;
	}
}

// 0x8005825C
void TEneGroupWave::draw()
{
	s16 next;
	switch (mState) {
	case WAVE_WAIT:
		if (enemyGroupStepActive(mGroup, mStep) == 0) {
			mTimer = 120;
			mState = WAVE_NEXT;
		}
		break;
	case WAVE_NEXT:
		mTimer--;
		if (mTimer <= 0) {
			while (enemyGroupNextStep(mGroup, mStep, &next) != 0) {
				mStep = next;
				if (enemyGroupStartStep(mGroup, mStep) != 2) {
					mState = WAVE_WAIT;
					break;
				}
			}
			if (mState != WAVE_WAIT) {
				if (mKeyBit != 0) {
					setKeyBit(mKeyBit);
				}
				if (mRestart != 0) {
					mState = WAVE_AGAIN;
					mTimer = 120;
				} else {
					mFlags |= 1;
				}
			}
		}
		break;
	case WAVE_AGAIN:
		mTimer--;
		if (mTimer <= 0) {
			enemyGroupShow(mGroup);
			if (enemyGroupStartFirst(mGroup, &mStep) != 0) {
				mState = WAVE_WAIT;
			} else {
				mFlags |= 1;
			}
		}
		break;
	}
}

struct SetLink;

extern "C" {
extern const char sGroupWatcherName[]               = "TEneGroupWatcher";
extern const char sGroupComboName[]                 = "TEneGroupCombo";
const char* gTEneGroupCollisionName                 = "TEneGroupCollision";
extern const ColliShape sEneGroupCollisionShapes[1] = {
	{ { 0.0f, 20.0f, 0.0f }, 0x0, 0, 0x0, 20.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

// 0x800583CC
extern "C" TObject* createTEneGroupCollision(TObject* parent, SetDesc* desc)
{
	return new TEneGroupCollision(parent, desc);
}

// 0x80058420
TEneGroupCollision::TEneGroupCollision(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00   = (u32)gTEneGroupCollisionName;
	mSize = 0x168;
	init(desc);
	mEnemyFlags |= ENEMY_KILLED;
}

// 0x80058488
void TEneGroupCollision::init(SetDesc* desc)
{
	mCollider = colliCreate(1, 1);
	colliCopyShapes(mCollider, sEneGroupCollisionShapes);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0xFF, 0xFF, 0xFE);
	mState = 0;
	readEnemyDesc(desc);
	setLinkDone((SetLink*)&mLink);
}

// 0x80058520
// One that waited for its group says the group is done
TEneGroupCollision::~TEneGroupCollision()
{
	colliRetire(mCollider);
	mCollider = 0;
	if (mRestart == RESTART_NOT) {
		enemyGroupFinish(mGroup);
	}
}

// 0x800585A4
// Places the shape on the body
void TEneGroupCollision::syncCollider()
{
	ColliShape* shape = colliGetShape(mCollider, 0);
	ColliShape place;
	place._0C    = 0;
	place.mPos.x = 0.0f;
	place.mPos.y = 0.0f;
	place.mPos.z = 0.0f;
	place.mRotY  = 0;
	place._14    = 0;
	switch (shape->mType) {
	case SHAPE_SPHERE:
		colliPlaceShapes(mCollider, &mBody.mPos, &place);
		colliBoundsOff(mCollider);
		break;
	case SHAPE_CYLINDER:
		place.mPos.y = shape->mSizeY;
		colliPlaceShapes(mCollider, &mBody.mPos, &place);
		colliRefreshBounds(mCollider);
		break;
	case SHAPE_BOX:
		place.mPos.y = shape->mSizeY;
		colliPlaceShapesRotY(mCollider, &mBody.mPos, mBody.mRot.y, &place);
		colliRefreshBounds(mCollider);
		break;
	}
}

// 0x80058684
// Ints the group how many the shape the restart
void TEneGroupCollision::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	mGroup      = desc->getInt(0);
	mCount      = desc->getInt(1);
	mRestart    = desc->getInt(3);
	int restart = mRestart;
	if (restart < 0) {
		restart = 0;
	} else if (restart > 2) {
		restart = 2;
	}
	mRestart          = restart;
	ColliShape* shape = colliGetShape(mCollider, 0);
	int type          = desc->getInt(2);
	type              = type < 0 ? 0 : (type > 2 ? 2 : type);
	f32 a             = desc->getFloat(0);
	a                 = a < 10.0f ? 10.0f : a;
	f32 b             = desc->getFloat(1);
	b                 = b < 10.0f ? 10.0f : b;
	f32 c             = desc->getFloat(2);
	c                 = c < 10.0f ? 10.0f : c;
	switch (type) {
	case 0:
		shape->mType = SHAPE_SPHERE;
		colliShapeSetRadius(shape, a);
		break;
	case 1:
		shape->mType = SHAPE_CYLINDER;
		colliShapeSetCylRadius(shape, a);
		colliShapeSetCylHeight(shape, b);
		break;
	case 2:
		shape->mType = SHAPE_BOX;
		colliShapeSetBoxX(shape, a);
		colliShapeSetBoxY(shape, b);
		colliShapeSetBoxZ(shape, c);
		break;
	}
	syncCollider();
}

// 0x800588CC
void TEneGroupCollision::updateBody()
{
	switch (mState) {
	case 0:
		if (isInRange(this, &mBody.mPos, 500.0f) == 0) {
			setLinkFree((SetLink*)&mLink);
			mFlags |= 1;
		} else if (colliGetContactCount(mCollider) != 0) {
			colliDisable(mCollider);
			enemyGroupStartWave(mGroup, mCount, mRestart == RESTART_ON);
			if (mRestart != RESTART_NOT) {
				mFlags |= 1;
			} else {
				mState = 1;
			}
		}
		break;
	case 1:
		if (enemyGroupWaveActive(mGroup) == 0) {
			setLinkForever((SetLink*)&mLink);
			mFlags |= 1;
		}
		break;
	}
}

// 0x800589BC
void TEneGroupCollision::vfn_6C()
{
}

// 0x800589C0
void TEneGroupCollision::vfn_3C()
{
	int x    = kEnemyInfoX << 16;
	int line = kEnemyInfoLine;
	njPrintC(x | line, ":GROUP");
	njPrintC(x | (line + 1), ":SWITCH ID");
	njPrintC(x | (line + 2), ":COL TYPE");
	switch (mRestart) {
	case RESTART_OFF:
		njPrintC(x | (line + 3), ":RE-SET OFF");
		break;
	case RESTART_ON:
		njPrintC(x | (line + 3), ":RE-SET ON");
		break;
	case RESTART_NOT:
		njPrintC(x | (line + 3), ":NOT RESTART");
		break;
	}
	switch (colliGetShape(mCollider, 0)->mType) {
	case SHAPE_SPHERE:
		njPrintC(x | (line + 4), ":RAD");
		break;
	case SHAPE_CYLINDER:
		njPrintC(x | (line + 4), ":RAD");
		njPrintC(x | (line + 5), ":HEIGHT");
		break;
	case SHAPE_BOX:
		njPrintC(x | (line + 4), ":WIDTH");
		njPrintC(x | (line + 5), ":HEIGHT");
		njPrintC(x | (line + 6), ":DEPTH");
		break;
	}
}

// 0x80058B24
int TEneGroupCollision::canAct()
{
	return 0;
}

// 0x80058B2C
void TEneGroupCollision::vfn_B0()
{
}

// 0x80058B30
void TEneGroupCollision::onActionPhase(int, int)
{
}
