extern "C" void initSetManager()
{
	sSetManagerObj = 0;
	createSetManager();
	spawnGuardInit();
}

extern "C" void freeSetManager()
{
	spawnGuardFree();
	if (sSetManagerObj != 0) {
		delete sSetManagerObj;
		sSetManagerObj = 0;
	}
}

extern "C" void createSetManager()
{
	if (sSetManagerObj == 0) {
		sSetManagerObj = new SetManager;
	}
}

// 0x80016BF0
#pragma push
#pragma opt_propagation off
SetManager::SetManager()
{
	int kind = SET_KIND_OBJ;

	mObjLists[0]   = new SetListObj(SET_KIND_OBJ, sSetListSizes[0]);
	mLists[kind]   = mObjLists[0];
	mParents[kind] = gTL_05;

	kind           = SET_KIND_ENEMY;
	mEnemy         = new SetListEnemy(SET_KIND_ENEMY, sSetListSizes[0]);
	mLists[kind]   = mEnemy;
	mParents[kind] = gTL_04;

	kind           = SET_KIND_DESIGN;
	mObjLists[1]   = new SetListObj(SET_KIND_DESIGN, sSetListSizes[1]);
	mLists[kind]   = mObjLists[1];
	mParents[kind] = gTL_05;

	mTick = 0;
}
#pragma pop

// 0x80016CE8
SetManager::~SetManager()
{
	for (int i = 0; i < 1; i++) {
		if ((&mEnemy)[i] != 0) {
			delete (&mEnemy)[i];
			(&mEnemy)[i] = 0;
		}
	}
	for (int i = 1; i >= 0; i--) {
		if (mObjLists[i] != 0) {
			delete mObjLists[i];
			mObjLists[i] = 0;
		}
	}
	for (int i = 0; i < SET_KIND_COUNT; i++) {
		mLists[i] = 0;
	}
}

extern "C" int hasSetManager()
{
	return sSetManagerObj != 0;
}

extern "C" void tickSetManager()
{
	// Tests the function's address instead of calling it so this is always true
	if (hasSetManager) {
		sSetManagerObj->tick();
	}
}

void SetManager::tick()
{
	mTick++;
	mTick &= 0xFFFF;
}

extern "C" void loadSetFile(int kind, const char* name)
{
	if (hasSetManager()) {
		if (kind == SET_KIND_ENEMY) {
			sSetManagerObj->load(SET_KIND_ENEMY, name);
			enemyGroupBlockAll();
		} else {
			sSetManagerObj->load(kind, name);
		}
	}
}

extern "C" void loadSetObjFile(const char* name)
{
	loadSetFile(SET_KIND_OBJ, name);
}

extern "C" void loadSetEnemyFile(const char* name)
{
	loadSetFile(SET_KIND_ENEMY, name);
}

extern "C" void loadSetDesignFile(const char* name)
{
	loadSetFile(SET_KIND_DESIGN, name);
}

void SetManager::load(int kind, const char* name)
{
	if (name != 0) {
		bool valid = false;
		if (kind >= 0 && kind < SET_KIND_COUNT) {
			valid = true;
		}
		if (valid) {
			mLists[kind]->load(name);
			mLists[kind]->prepareAll();
		}
	}
}

extern "C" SetEnemyRecord* getSetEnemyRecord(int index)
{
	if (!hasSetManager()) {
		return 0;
	}
	return sSetManagerObj->getEnemyRecord(index);
}

SetEnemyRecord* SetManager::getEnemyRecord(int index)
{
	return mEnemy->getRecord(index);
}

extern "C" SetSlot* getSetSlot(int kind, int index)
{
	if (!hasSetManager()) {
		return 0;
	}
	return sSetManagerObj->getSlot(kind, index);
}

extern "C" SetSlot* getSetEnemySlot(int index)
{
	if (!hasSetManager()) {
		return 0;
	}
	return sSetManagerObj->getSlot(SET_KIND_ENEMY, index);
}

SetSlot* SetManager::getSlot(int kind, int index)
{
	bool valid = false;
	if (kind >= 0 && kind < SET_KIND_COUNT) {
		valid = true;
	}
	if (valid) {
		return mLists[kind]->getSlot(index);
	}
	return 0;
}

extern "C" void getSetCounts(int kind, int* outCount, int* outCapacity)
{
	if (hasSetManager()) {
		sSetManagerObj->getCounts(kind, outCount, outCapacity);
	}
}

extern "C" void getSetObjCounts(int* outCount, int* outCapacity)
{
	getSetCounts(SET_KIND_OBJ, outCount, outCapacity);
}

extern "C" void getSetEnemyCounts(int* outCount, int* outCapacity)
{
	getSetCounts(SET_KIND_ENEMY, outCount, outCapacity);
}

extern "C" void getSetDesignCounts(int* outCount, int* outCapacity)
{
	getSetCounts(SET_KIND_DESIGN, outCount, outCapacity);
}

static inline int listCount(SetList* list)
{
	return list->mCount;
}
static inline int listCapacity(SetList* list)
{
	return list->mCapacity;
}

// 0x80017188
void SetManager::getCounts(int kind, int* outCount, int* outCapacity)
{
	bool valid = false;
	if (kind >= 0 && kind < SET_KIND_COUNT) {
		valid = true;
	}
	if (!valid) {
		return;
	}
	*outCount    = listCount(mLists[kind]);
	*outCapacity = listCapacity(mLists[kind]);
}

extern "C" TObject* setSetObjParent(TObject* parent)
{
	if (!hasSetManager()) {
		return 0;
	}
	return sSetManagerObj->setParent(SET_KIND_OBJ, parent);
}

extern "C" TObject* setSetEnemyParent(TObject* parent)
{
	if (!hasSetManager()) {
		return 0;
	}
	return sSetManagerObj->setParent(SET_KIND_ENEMY, parent);
}

extern "C" TObject* setSetDesignParent(TObject* parent)
{
	if (!hasSetManager()) {
		return 0;
	}
	return sSetManagerObj->setParent(SET_KIND_DESIGN, parent);
}

TObject* SetManager::setParent(int kind, TObject* parent)
{
	TObject* old = 0;
	bool valid   = false;
	if (kind >= 0 && kind < SET_KIND_COUNT) {
		valid = true;
	}
	if (valid) {
		old            = mParents[kind];
		mParents[kind] = parent;
	}
	return old;
}

extern "C" void spawnSetObjsNow(void* arg)
{
	if (hasSetManager()) {
		sSetManagerObj->spawnNowObj(arg);
	}
}

void SetManager::spawnNowObj(void* arg)
{
	mEnemy->spawnNow(mParents[SET_KIND_ENEMY], (int)arg);
}

extern "C" int scanSet(int kind, Vec3f* centre, int start, int count, int check)
{
	if (!hasSetManager()) {
		return 0;
	}
	return sSetManagerObj->scan(kind, centre, start, count, check);
}

extern "C" int scanSetObj(Vec3f* centre, int start, int count)
{
	return scanSet(SET_KIND_OBJ, centre, start, count, 1);
}

extern "C" int scanSetEnemy(Vec3f* centre, int start, int count)
{
	return scanSet(SET_KIND_ENEMY, centre, start, count, 0);
}

extern "C" int scanSetDesign(Vec3f* centre, int start, int count)
{
	return scanSet(SET_KIND_DESIGN, centre, start, count, 0);
}

// 0x80017480
int SetManager::scan(int kind, Vec3f* centre, int start, int count, int check)
{
	int total;
	int pos;
	SetList* list;
	SetSlot* slot;
	int i;
	pos   = start;
	list  = mLists[kind];
	total = listCount(list);
	for (i = 0; i < count; i++) {
		if (pos >= total) {
			pos = 0;
		}
		slot = list->getSlot(pos);
		if (slot->canSpawn(mTick)) {
			int checkOverlap = 0;
			if (check != 0 && slot->hasNoData() == 0) {
				checkOverlap = 1;
			}
			list->spawn(mParents[kind], pos, centre, (u8)checkOverlap);
		}
		pos++;
	}
	return pos;
}

extern "C" u32 spawnSetById(int id)
{
	u32 result = 0xFFFF;
	if (hasSetManager()) {
		result = sSetManagerObj->spawnById(SET_KIND_OBJ, SET_KIND_OBJ, id);
		if ((result & 0xFFFF) == 0xFFFF) {
			result = sSetManagerObj->spawnById(SET_KIND_ENEMY, SET_KIND_DESIGN, id);
		}
	}
	return result;
}

// 0x800175D0
u32 SetManager::spawnById(int kindA, int kindB, int id)
{
	SetDescObj desc;
	int count;
	SetListObj* list;
	SetSlot* slot;
	int result;
	int i  = 0;
	list   = mObjLists[kindA];
	result = 0xFFFF;
	count  = list->mCount;

	for (; i < count; i++) {
		slot = list->getSlot(i);
		list->getDesc(i, &desc);
		if (id == desc.get3C()) {
			if (slot->canSpawn(mTick)) {
				list->spawnNow(mParents[kindB], i);
			}
			result = slot->mObjId;
			break;
		}
	}
	return result;
}

// 0x800176C8
void SetManager::onDeath(int kind, void* obj, SetLink* link)
{
	if (link->mKind != -1) {
		s16 index     = link->mIndex;
		SetSlot* slot = mLists[kind]->getSlot(index);
		if (kind == SET_KIND_ENEMY) {
			SetEnemyRecord* record = getSetEnemyRecord(index);
			if (record != 0 && isEnemyGrouped(record->_1C) != 0 && link->mMode == 2) {
				setLinkDone(link);
			}
		}
		if (slot != 0) {
			switch (link->mMode) {
			case 0:
				slot->setDeadFree(obj);
				break;
			case 1:
				slot->setDeadDone(obj);
				break;
			case 2:
				slot->setDeadTimed(obj, (u16)(mTick + link->mTimer));
				break;
			case 3:
				slot->setDeadForever(obj);
				break;
			}
		}
		link->reset();
	}
}

extern "C" void onSetDeath(int kind, void* obj, SetLink* link)
{
	if (hasSetManager()) {
		sSetManagerObj->onDeath(kind, obj, link);
	}
}

extern "C" void resetSets()
{
	if (hasSetManager()) {
		sSetManagerObj->resetKind(SET_KIND_OBJ);
		sSetManagerObj->resetKind(SET_KIND_ENEMY);
		sSetManagerObj->resetKind(SET_KIND_DESIGN);
	}
}

void SetManager::resetKind(int kind)
{
	bool valid = false;
	if (kind >= 0 && kind < SET_KIND_COUNT) {
		valid = true;
	}
	if (valid) {
		mLists[kind]->resetAll();
		if (kind == SET_KIND_ENEMY) {
			enemyGroupBlockAll();
		}
	}
}

extern "C" void clearSets()
{
	if (hasSetManager()) {
		sSetManagerObj->clearKind(SET_KIND_OBJ);
		sSetManagerObj->clearKind(SET_KIND_ENEMY);
		sSetManagerObj->clearKind(SET_KIND_DESIGN);
	}
}

void SetManager::clearKind(int kind)
{
	bool valid = false;
	if (kind >= 0 && kind < SET_KIND_COUNT) {
		valid = true;
	}
	if (valid) {
		mLists[kind]->clearAll();
		if (kind == SET_KIND_ENEMY) {
			enemyGroupBlockAll();
		}
	}
}

extern "C" void attachSetLink(SetLink* link)
{
	if (hasSetManager()) {
		sSetManagerObj->attachLink(link);
	}
}

void SetManager::attachLink(SetLink* link)
{
	int kind = link->mKind;
	if (kind >= 0 && kind < SET_KIND_COUNT) {
		mLists[kind]->attachLink(link);
	}
}

SetListEnemy::SetListEnemy(int kind, int capacity)
    : SetList(kind, capacity)
{
	mCount   = 0;
	mRecords = 0;
}

SetListEnemy::~SetListEnemy()
{
	mRecords = 0;
}

void* SetListEnemy::spawnNow(TObject* parent, int index)
{
	SetDescEnemy desc;
	void* obj     = 0;
	SetSlot* slot = getDesc(index, &desc);
	if (slot != 0) {
		obj = desc.spawnNow(parent);
		if (obj != 0) {
			slot->setSpawned(obj);
		}
	}
	return obj;
}

SetEnemyRecord* SetListEnemy::getRecord(int index)
{
	if (isValid(index)) {
		return &mRecords[index];
	}
	return 0;
}

SetListObj::SetListObj(int kind, int capacity)
    : SetList(kind, capacity)
{
	mCount   = 0;
	mRecords = 0;
}

SetListObj::~SetListObj()
{
	mRecords = 0;
}

SetSlot* SetListObj::getDesc(int index, SetDesc* desc)
{
	SetSlot* slot = 0;
	if (isValid(index)) {
		slot = &mSlots[index];
		((SetDescObj*)desc)->setRecord(&mRecords[index]);
		desc->setKind(mKind);
		desc->mIndex = index;
		desc->mObjId = slot->mObjId;
	}
	return slot;
}

void* SetListObj::spawnNow(TObject* parent, int index)
{
	SetDescObj desc;
	void* obj     = 0;
	SetSlot* slot = getDesc(index, &desc);
	if (slot != 0) {
		obj = desc.spawnNow(parent);
		if (obj != 0) {
			slot->setSpawned(obj);
		}
	}
	return obj;
}

void SetListEnemy::resetAll()
{
	for (int i = 0; i < mCount; i++) {
		mSlots[i].reset();
	}
}

void SetListEnemy::clearAll()
{
	for (int i = 0; i < mCount; i++) {
		mSlots[i].clear();
	}
}

void SetListEnemy::load(const char* name)
{
	if (loadSizedResource(name, &mFile)) {
		mRecords = (SetEnemyRecord*)mFile->mData;
		countRecords();
	}
}

void SetListEnemy::prepareAll()
{
	SetDescEnemy desc;
	for (int i = 0; i < mCount; i++) {
		desc.setRecord(&mRecords[i]);
		desc.setKind(mKind);
		desc.mIndex = i;
		desc.prepare();
	}
}

SetSlot* SetListEnemy::getDesc(int index, SetDesc* desc)
{
	SetSlot* slot = 0;
	if (isValid(index)) {
		slot = &mSlots[index];
		((SetDescEnemy*)desc)->setRecord(&mRecords[index]);
		desc->setKind(mKind);
		desc->mIndex = index;
		desc->mObjId = slot->mObjId;
	}
	return slot;
}

void* SetListEnemy::spawn(TObject* parent, int index, Vec3f* centre, int check)
{
	SetDescEnemy desc;
	void* obj     = 0;
	SetSlot* slot = getDesc(index, &desc);
	if (slot != 0) {
		obj = desc.spawn(parent, centre, check);
		if (obj != 0) {
			slot->setSpawned(obj);
		}
	}
	return obj;
}

void SetListObj::resetAll()
{
	for (int i = 0; i < mCount; i++) {
		mSlots[i].reset();
	}
}

void SetListObj::clearAll()
{
	for (int i = 0; i < mCount; i++) {
		mSlots[i].clear();
	}
}

void SetListObj::load(const char* name)
{
	if (loadSizedResource(name, &mFile)) {
		mRecords = (SetRecord*)mFile->mData;
		countRecords();
	}
}

void SetListObj::prepareAll()
{
	SetDescObj desc;
	for (int i = 0; i < mCount; i++) {
		desc.setRecord(&mRecords[i]);
		desc.setKind(mKind);
		desc.mIndex = i;
		desc.prepare();
	}
}

void* SetListObj::spawn(TObject* parent, int index, Vec3f* centre, int check)
{
	SetDescObj desc;
	void* obj     = 0;
	SetSlot* slot = getDesc(index, &desc);
	if (slot != 0) {
		obj = desc.spawn(parent, centre, check);
		if (obj != 0) {
			slot->setSpawned(obj);
		}
	}
	return obj;
}

// SetRecord as the copy in SetListObj::remove sees it the four ints and four floats move as one 32-byte block
struct SetRecordCopy {
	int mId;
	Vec3f mPos;
	int mRot[3];
	u32 mParams[8];
	s16 _3C;
	s16 _3E;
};

// 0x80018204
void SetListObj::remove(int index)
{
	SetDescObj desc;
	if (!isValid(index)) {
		return;
	}
	desc.setRecord(&mRecords[index]);
	desc.clear();
	mSlots[index].reset();
	mCount--;
	for (int i = index; i < mCount; i++) {
		*(SetRecordCopy*)&mRecords[i] = *(SetRecordCopy*)&mRecords[i + 1];
		mSlots[i]                     = mSlots[i + 1];
		if (findObjById(mSlots[i].mObjId) != 0) {
			mSlots[i].setLinkIndex(i);
		}
	}
	desc.setRecord(&mRecords[mCount]);
	desc.clear();
	mSlots[mCount].init();
}

// SetEnemyRecord as SetListEnemy::remove copies it with the same 32-byte block of ints and floats
struct SetEnemyRecordCopy {
	int mId;
	Vec3f mPos;
	int mRot[3];
	s16 _1C;
	s16 _1E;
	int _20;
	int _24[3];
	u32 mParams[8];
};

// 0x800183EC
void SetListEnemy::remove(int index)
{
	SetDescEnemy desc;
	if (!isValid(index)) {
		return;
	}
	desc.setRecord(&mRecords[index]);
	desc.clear();
	mSlots[index].reset();
	mCount--;
	for (int i = index; i < mCount; i++) {
		*(SetEnemyRecordCopy*)&mRecords[i] = *(SetEnemyRecordCopy*)&mRecords[i + 1];
		mSlots[i]                          = mSlots[i + 1];
		if (findObjById(mSlots[i].mObjId) != 0) {
			mSlots[i].setLinkIndex(i);
		}
	}
	desc.setRecord(&mRecords[mCount]);
	desc.clear();
	mSlots[mCount].init();
}

// 0x800185F4
void SetListEnemy::countRecords()
{
	int count = 0;
	SetDescEnemy desc;
	for (int i = 0; i < mCapacity; i++) {
		desc.setRecord(&mRecords[i]);
		if (desc.isEmpty()) {
			break;
		}
		count++;
	}
	mCount = count;
}

// 0x8001867C
void SetListObj::countRecords()
{
	int count = 0;
	SetDescObj desc;
	for (int i = 0; i < mCapacity; i++) {
		desc.setRecord(&mRecords[i]);
		if (desc.isEmpty()) {
			break;
		}
		count++;
	}
	mCount = count;
}
