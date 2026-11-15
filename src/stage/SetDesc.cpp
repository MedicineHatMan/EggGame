#include "stage/SetManager.h"
#include "enemy/Enemy.h"
#include "enemy/SpawnGuard.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "Ninja/ninja.h"

SetDesc::SetDesc()
{
	mKind  = -1;
	mIndex = 0;
	mObjId = 0xFFFF;
}

SetDesc::~SetDesc()
{
}

int SetDesc::isValid()
{
	return mKind == -1;
}

// 0x8001A3FC
int SetDesc::isDataEmpty() const
{
	SetSlot* slot = getSetSlot(mKind, mIndex);
	if (slot != 0) {
		return slot->hasNoData();
	}
	return 1;
}

void SetDesc::setKind(int kind)
{
	mKind = kind;
}

int SetDesc::getKind()
{
	return mKind;
}

// 0x8001A44C
int SetDesc::isSpawned()
{
	const u16 id = mObjId;
	if (id != 0xFFFF) {
		return findObjById(id) != 0;
	}
	return 0;
}

SetDescObj::SetDescObj()
{
	mRecord = 0;
}

SetDescObj::~SetDescObj()
{
}

void SetDescObj::setRecord(SetRecord* record)
{
	mRecord = record;
}

static void clearRecord(SetRecord* record)
{
	record->mId     = 0;
	record->mPos.x  = 0.0f;
	record->mPos.y  = 0.0f;
	record->mPos.z  = 0.0f;
	record->mRot[0] = 0;
	record->mRot[1] = 0;
	record->mRot[2] = 0;
	for (int i = 0; i < 4; i++) {
		record->mInts[i]   = 0;
		record->mFloats[i] = 0.0f;
	}
	record->_3C = 0;
	record->_3E = 0;
}

void SetDescObj::clear()
{
	if (mRecord != 0) {
		clearRecord(mRecord);
	}
}

void SetDescObj::getPos(Vec3f* out)
{
	SetRecord* record = mRecord;
	if (record != 0) {
		out->x = record->mPos.x;
		out->y = record->mPos.y;
		out->z = record->mPos.z;
	}
}

void SetDescObj::getRot(SetRot* out)
{
	SetRecord* record = mRecord;
	if (record != 0) {
		out->set(record->mRot[0], record->mRot[1], record->mRot[2]);
	}
}

int SetDescObj::getId()
{
	if (mRecord != 0) {
		return mRecord->mId;
	}
	return 0;
}

int SetDescObj::isEmpty()
{
	if (mRecord != 0) {
		return mRecord->mId == 0;
	}
	return 1;
}

void SetDescObj::prepare()
{
	const SetClass* cls = findObjClass(getId());
	if (cls != 0 && cls->mPrepare != 0) {
		cls->mPrepare(this);
	}
}

void* SetDescObj::spawnNow(TObject* parent)
{
	void* obj = 0;
	if (isSpawned()) {
		return 0;
	}
	const SetClass* cls = findObjClass(getId());
	if (cls != 0 && cls->mCreate != 0) {
		obj = cls->mCreate(parent, this);
	}
	return obj;
}

void* SetDescObj::spawn(TObject* parent, Vec3f* centre, int check)
{
	void* obj = 0;
	if (isSpawned()) {
		return 0;
	}
	const SetClass* cls = findObjClass(getId());
	if (cls != 0 && cls->mCreate != 0) {
		Vec3f pos;
		getPos(&pos);
		if (centre != 0) {
			Vec3f diff;
			diff.x = pos.x;
			diff.y = pos.y;
			diff.z = pos.z;
			njSubVector(&diff, centre);
			if (njScalor2(&diff) > cls->mRangeSq) {
				return 0;
			}
		}
		if (check != 0 && spawnGuardTest(&pos) != 0) {
			return 0;
		}
		obj = cls->mCreate(parent, this);
	}
	return obj;
}

int SetDescObj::getInt(int index)
{
	if (index < 0 || index >= 4) {
		return 0;
	}
	if (mRecord != 0) {
		return mRecord->mInts[index];
	}
	return 0;
}

f32 SetDescObj::getFloat(int index)
{
	if (index < 0 || index >= 4) {
		return 0.0f;
	}
	if (mRecord != 0) {
		return mRecord->mFloats[index];
	}
	return 0.0f;
}

int SetDescObj::get3E()
{
	if (mRecord != 0) {
		return mRecord->_3E;
	}
	return 0;
}

int SetDescObj::get3C()
{
	if (mRecord != 0) {
		return mRecord->_3C;
	}
	return 0;
}

SetDescEnemy::SetDescEnemy()
{
	mRecord = 0;
}

SetDescEnemy::~SetDescEnemy()
{
}

void SetDescEnemy::setRecord(SetEnemyRecord* record)
{
	mRecord = record;
}

static void clearEnemyRecord(SetEnemyRecord* record)
{
	record->mId     = 0;
	record->mPos.x  = 0.0f;
	record->mPos.y  = 0.0f;
	record->mPos.z  = 0.0f;
	record->mRot[0] = 0;
	record->mRot[1] = 0;
	record->mRot[2] = 0;
	record->_1C     = 0;
	record->_1E     = 0;
	record->_20     = 0;
	for (int i = 0; i < 3; i++) {
		record->_24[i] = 0;
	}
	for (int i = 0; i < 4; i++) {
		record->mInts[i]   = 0;
		record->mFloats[i] = 0.0f;
	}
}

void SetDescEnemy::clear()
{
	if (mRecord != 0) {
		clearEnemyRecord(mRecord);
	}
}

void SetDescEnemy::getPos(Vec3f* out)
{
	SetEnemyRecord* record = mRecord;
	if (record != 0) {
		out->x = record->mPos.x;
		out->y = record->mPos.y;
		out->z = record->mPos.z;
	}
}

void SetDescEnemy::getRot(SetRot* out)
{
	SetEnemyRecord* record = mRecord;
	if (record != 0) {
		out->set(record->mRot[0], record->mRot[1], record->mRot[2]);
	}
}

int SetDescEnemy::getId()
{
	if (mRecord != 0) {
		return mRecord->mId;
	}
	return 0;
}

int SetDescEnemy::isEmpty()
{
	if (mRecord != 0) {
		return mRecord->mId == 0;
	}
	return 1;
}

void SetDescEnemy::prepare()
{
	const SetClass* cls = lookupEnemyClass(getId());
	if (cls != 0 && cls->mPrepare != 0) {
		cls->mPrepare(this);
	}
}

void* SetDescEnemy::spawnNow(TObject* parent)
{
	void* obj = 0;
	if (isSpawned()) {
		return 0;
	}
	const SetClass* cls = lookupEnemyClass(getId());
	if (cls != 0 && cls->mCreate != 0) {
		obj = cls->mCreate(parent, this);
	}
	return obj;
}

void* SetDescEnemy::spawn(TObject* parent, Vec3f* centre, int check)
{
	void* obj = 0;
	if (isSpawned()) {
		return 0;
	}
	const SetClass* cls = lookupEnemyClass(getId());
	if (cls != 0 && cls->mCreate != 0) {
		if (centre != 0) {
			Vec3f pos;
			getPos(&pos);
			Vec3f diff;
			diff.x = pos.x;
			diff.y = pos.y;
			diff.z = pos.z;
			njSubVector(&diff, centre);
			if (njScalor2(&diff) > cls->mRangeSq) {
				return 0;
			}
		}
		obj = cls->mCreate(parent, this);
	}
	return obj;
}

int SetDescEnemy::getInt(int index)
{
	if (index < 0 || index >= 4) {
		return 0;
	}
	if (mRecord != 0) {
		return mRecord->mInts[index];
	}
	return 0;
}

f32 SetDescEnemy::getFloat(int index)
{
	if (index < 0 || index >= 4) {
		return 0.0f;
	}
	if (mRecord != 0) {
		return mRecord->mFloats[index];
	}
	return 0.0f;
}
