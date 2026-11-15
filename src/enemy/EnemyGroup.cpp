#include "stage/SetManager.h"
#include "enemy/EnemyGroupWave.h"

extern "C" int enemyGroupGetSet(SetEnemyRecord** record, SetSlot** slot)
{
	*record = getSetEnemyRecord(0);
	*slot   = getSetEnemySlot(0);
	u8 ok   = 0;
	if (*record != 0 && *slot != 0) {
		ok = 1;
	}
	return ok;
}

// 0x80058B9C
extern "C" int enemyGroupIs(SetEnemyRecord* record, s16 group, s16 step)
{
	u8 is = 0;
	if (record->_1C == group && record->_1E == step) {
		is = 1;
	}
	return is;
}

// 0x80058BCC
extern "C" int isEnemyGrouped(s16 group)
{
	return group > 0;
}

// 0x80058BE0
// The group's lowest step
extern "C" int enemyGroupFirstStep(s16 group, s16* out)
{
	SetEnemyRecord* records = getSetEnemyRecord(0);
	if (records == 0) {
		return 0;
	}
	int count;
	int i;
	s16 step;
	getSetEnemyCounts(&count, &i);
	i = 0;
	for (i = 0; i < count; i++) {
		if (group == records[i]._1C) {
			step = records[i]._1E;
			break;
		}
	}
	if (i >= count) {
		return 0;
	}
	for (; i < count; i++) {
		if (group == records[i]._1C && records[i]._1E < step) {
			step = records[i]._1E;
		}
	}
	*out = step;
	return 1;
}

// 0x80058CE0
// The group's next step after after
extern "C" int enemyGroupNextStep(s16 group, s16 after, s16* out)
{
	SetEnemyRecord* records = getSetEnemyRecord(0);
	if (records == 0) {
		return 0;
	}
	int count;
	int i;
	s16 step;
	getSetEnemyCounts(&count, &i);
	i = 0;
	for (i = 0; i < count; i++) {
		if (group == records[i]._1C) {
			step = records[i]._1E;
			if (step > after) {
				break;
			}
		}
	}
	if (i >= count) {
		return 0;
	}
	for (; i < count; i++) {
		if (group == records[i]._1C && records[i]._1E > after && records[i]._1E < step) {
			step = records[i]._1E;
		}
	}
	*out = step;
	return 1;
}

// 0x80058DF4
// Blocks every grouped record at the start of the stage
extern "C" void enemyGroupBlockAll()
{
	int count;
	int i;
	SetEnemyRecord* record;
	SetSlot* slot;
	if (enemyGroupGetSet(&record, &slot) == 0) {
		return;
	}
	getSetEnemyCounts(&count, &i);
	for (i = 0; i < count; i++) {
		if (isEnemyGrouped(record->_1C) != 0) {
			slot->setBlocked();
		}
		record++;
		slot++;
	}
	enemyGroupWavesEnd();
	enemyGroupStepsEnd();
}

// 0x80058E94
// Spawns the listed slots that are still blocked
extern "C" void enemyGroupSpawn(int* list, int count)
{
	SetSlot* slots = getSetEnemySlot(0);
	if (slots == 0) {
		return;
	}
	int index;
	for (int i = 0; i < count; i++) {
		index = list[i];
		if (slots[index].takeBlocked() != 0) {
			spawnSetObjsNow((void*)index);
		}
	}
}

// 0x80058F0C
extern "C" void enemyGroupShow(s16 group)
{
	int count;
	int i;
	SetEnemyRecord* record;
	SetSlot* slot;
	if (enemyGroupGetSet(&record, &slot) == 0) {
		return;
	}
	getSetEnemyCounts(&count, &i);
	for (i = 0; i < count; i++) {
		if (group == record->_1C) {
			slot->takeHidden();
		}
		record++;
		slot++;
	}
}

// 0x80058FB0
// The group is beaten for good
extern "C" void enemyGroupFinish(s16 group)
{
	int count;
	int i;
	SetEnemyRecord* record;
	SetSlot* slot;
	if (enemyGroupGetSet(&record, &slot) == 0) {
		return;
	}
	getSetEnemyCounts(&count, &i);
	for (i = 0; i < count; i++) {
		if (group == record->_1C) {
			slot->setFinished();
		}
		record++;
		slot++;
	}
}

// 0x80059054
// The slots of one step of a group in a new array
extern "C" int* enemyGroupCollect(s16 group, s16 step, int* outCount, int take)
{
	int* list = 0;
	*outCount = 0;
	int count;
	int i;
	SetEnemyRecord* records;
	SetSlot* slots;
	if (enemyGroupGetSet(&records, &slots) != 0) {
		int first              = -1;
		int last               = -1;
		SetEnemyRecord* record = records;
		SetSlot* slot          = slots;
		int shown;
		int found;
		found = 0;
		shown = 0;
		getSetEnemyCounts(&count, &i);
		for (i = 0; i < count; i++) {
			int match;
			if (enemyGroupIs(record, group, step) != 0) {
				found++;
				if (slot->checkHidden() == 0) {
					shown++;
				}
				match = 1;
			} else {
				match = 0;
			}
			if (match) {
				first = i;
				last  = i;
				break;
			}
			record++;
			slot++;
		}
		if (first != -1) {
			for (i = first + 1, record++, slot++; i < count; i++, record++, slot++) {
				int match;
				if (enemyGroupIs(record, group, step) != 0) {
					found++;
					if (slot->checkHidden() == 0) {
						shown++;
					}
					match = 1;
				} else {
					match = 0;
				}
				if (match) {
					last = i;
				}
			}
		}
		if (found == 0) {
			*outCount = -1;
			return 0;
		}
		if (shown == 0) {
			if (take != 0) {
				SetEnemyRecord* rec = records + first;
				SetSlot* sl         = slots + first;
				*outCount           = found;
				list                = new int[found];
				int n               = 0;
				for (i = first; i <= last; i++) {
					if (enemyGroupIs(rec, group, step) != 0) {
						sl->takeAny();
						list[n++] = i;
					}
					rec++;
					sl++;
				}
			} else {
				*outCount = 0;
				return 0;
			}
		} else {
			SetEnemyRecord* rec = records + first;
			SetSlot* sl         = slots + first;
			*outCount           = shown;
			list                = new int[shown];
			int n               = 0;
			for (i = first; i <= last; i++) {
				if (enemyGroupIs(rec, group, step) != 0 && sl->checkHidden() == 0) {
					list[n++] = i;
				}
				rec++;
				sl++;
			}
		}
	}
	return list;
}

// 0x80059320
// Drops the hidden slots from a list filling each gap from the end
extern "C" void enemyGroupPrune(int* list, int* count)
{
	SetSlot* slots = getSetEnemySlot(0);
	if (slots == 0) {
		return;
	}
	for (int i = 0; i < *count; i++) {
		if (slots[list[i]].checkHidden() != 0) {
			(*count)--;
			list[i] = list[*count];
			i--;
		}
	}
}
