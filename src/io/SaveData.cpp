#include "io/SaveData.h"
#include "Ninja/ninja.h"
#include "core/SdkCalls.h"

extern "C" {
SaveData* gSaveData;
int gSaveSlot;
}

// 0x80198F08
extern "C" u32 crc32(const u8* data, u32 size)
{
	u32 crc = 0xFFFFFFFF;
	for (u32 i = 0; i < size; i++, data++) {
		crc ^= *data;
		for (int j = 8; j != 0; j--) {
			if (crc & 1) {
				crc = (crc >> 1) ^ 0xEDB88320;
			} else {
				crc >>= 1;
			}
		}
	}
	return crc ^ 0xFFFFFFFF;
}

// 0x80198F58
void SaveSlot::clear()
{
	memset(this, 0, sizeof(SaveSlot));
}

// 0x80198F80
extern "C" int keyBitPos(int bit, int* byte, u8* mask)
{
	if (bit < 0 || bit >= 0x400) {
		return 0;
	}
	*byte = bit / 8;
	*mask = 0x80 >> (bit % 8);
	return 1;
}

// 0x80198FCC
void SaveData::clear()
{
	memset(this, 0, sizeof(SaveData));
	mCrc = calcCrc();
}

// 0x8019900C
// The bus clock word at 0x800000F8 over 4 is the timer rate
void SaveData::stampTime()
{
	mTime = OSGetTime() / (*(u32*)0x800000F8 / 4);
}

// 0x80199050
u32 SaveData::calcCrc()
{
	return crc32((const u8*)this, 0x724);
}

// 0x80199074
void SaveData::updateCrc()
{
	mCrc = calcCrc();
}

// 0x801990A4
int SaveData::checkCrc()
{
	int crc = calcCrc();
	return crc == mCrc;
}

// 0x801990E0
extern "C" void saveCreate()
{
	gSaveData = new SaveData;
	saveReset();
}

// 0x8019910C
extern "C" void saveReset()
{
	if (gSaveData != 0) {
		gSaveData->clear();
	}
	gSaveSlot = 0;
}

// 0x80199140
extern "C" void saveRead(const SaveData* src)
{
	if (gSaveData != 0 && src != 0) {
		memcpy(gSaveData, src, sizeof(SaveData));
	}
}

// 0x80199180
extern "C" void saveWrite(SaveData* dst)
{
	if (gSaveData != 0 && dst != 0) {
		memcpy(dst, gSaveData, sizeof(SaveData));
	}
}

// 0x801991B8
extern "C" void saveSeal()
{
	if (gSaveData != 0) {
		gSaveData->stampTime();
		gSaveData->updateCrc();
	}
}

// 0x801991EC
extern "C" int saveTimeIs(u32 time)
{
	if (gSaveData == 0) {
		return 0;
	}
	if (time == 0) {
		return 1;
	}
	return gSaveData->mTime == time;
}

// 0x80199224
extern "C" SaveSlot* saveGetSlot(int slot)
{
	if (gSaveData == 0) {
		return 0;
	}
	if (slot < 0 || slot >= 3) {
		return 0;
	}
	return &gSaveData->mSlots[slot];
}

// 0x8019925C
extern "C" SaveRecord* saveGetRecord(int slot, int stage)
{
	SaveSlot* save = saveGetSlot(slot);
	if (save != 0 && stage >= 0 && stage < 0x38) {
		return &save->mRecords[stage];
	}
	return 0;
}

// 0x801992B4
extern "C" void saveSelectSlot(int slot)
{
	SaveSlot* save = saveGetSlot(slot);
	if (save != 0) {
		gSaveSlot   = slot;
		save->mUsed = 1;
	}
}

// 0x801992F4
extern "C" void saveCopySlot(int from, int to)
{
	if (from != to) {
		SaveSlot* src = saveGetSlot(from);
		SaveSlot* dst = saveGetSlot(to);
		if (src != 0 && dst != 0) {
			memmove(dst, src, sizeof(SaveSlot));
		}
	}
}

// 0x80199354
extern "C" void saveClearSlot(int slot)
{
	SaveSlot* save = saveGetSlot(slot);
	if (save != 0) {
		save->clear();
	}
}

// 0x80199380
extern "C" int saveSlotLives(int slot)
{
	SaveSlot* save = saveGetSlot(slot);
	if (save != 0) {
		return save->mLives;
	}
	return 0;
}

// 0x801993B4
extern "C" int saveSlotUsed(int slot)
{
	SaveSlot* save = saveGetSlot(slot);
	if (save != 0) {
		return save->mUsed == 1;
	}
	return 0;
}

// 0x801993F4
extern "C" int saveGetKeyBit(int bit)
{
	SaveSlot* save = saveGetSlot(gSaveSlot);
	int byte;
	u8 mask;
	if (save != 0 && keyBitPos(bit, &byte, &mask) != 0) {
		return mask & save->mKeyBits[byte];
	}
	return 0;
}

// 0x8019945C
extern "C" void saveSetKeyBit(int bit, int on)
{
	SaveSlot* save = saveGetSlot(gSaveSlot);
	int byte;
	u8 mask;
	if (save != 0 && keyBitPos(bit, &byte, &mask) != 0) {
		if (on != 0) {
			save->mKeyBits[byte] |= mask;
		} else {
			save->mKeyBits[byte] &= ~mask;
		}
	}
}

// 0x801994E4
extern "C" int saveGetTime(int stage)
{
	SaveRecord* record = saveGetRecord(gSaveSlot, stage);
	if (record != 0) {
		return record->mTime;
	}
	return 0;
}

// 0x80199520
extern "C" int saveGetPoints(int stage)
{
	SaveRecord* record = saveGetRecord(gSaveSlot, stage);
	if (record != 0) {
		return record->mPoints;
	}
	return 0;
}

// 0x8019955C
extern "C" void saveSetTime(int stage, int time)
{
	SaveRecord* record = saveGetRecord(gSaveSlot, stage);
	if (record != 0) {
		record->mTime = time;
	}
}

// 0x801995A0
extern "C" void saveSetPoints(int stage, int points)
{
	SaveRecord* record = saveGetRecord(gSaveSlot, stage);
	if (record != 0) {
		record->mPoints = points;
	}
}

// 0x801995E4
extern "C" int saveGetLives()
{
	return saveSlotLives(gSaveSlot);
}

// 0x80199608
extern "C" void saveSetLives(u8 lives)
{
	SaveSlot* save = saveGetSlot(gSaveSlot);
	if (save != 0) {
		save->mLives = lives;
	}
}

// 0x80199644
extern "C" int saveGetSoundMode()
{
	SaveSlot* save = saveGetSlot(gSaveSlot);
	if (save != 0) {
		return save->mSoundMode;
	}
	return 0;
}

// 0x8019967C
extern "C" void saveSetSoundMode(int mode)
{
	SaveSlot* save = saveGetSlot(gSaveSlot);
	if (save != 0) {
		save->mSoundMode = mode;
	}
}

// 0x801996B8
extern "C" int saveGetRumbleOff()
{
	SaveSlot* save = saveGetSlot(gSaveSlot);
	if (save != 0) {
		return save->mRumbleOff;
	}
	return 0;
}

// 0x801996F0
extern "C" void saveSetRumbleOff(int off)
{
	SaveSlot* save = saveGetSlot(gSaveSlot);
	if (save != 0) {
		if (off < 0 || off >= 2) {
			off = 0;
		}
		save->mRumbleOff = off;
	}
}

typedef char SaveSlot_size_check[(sizeof(SaveSlot) == 0x260) ? 1 : -1];
typedef char SaveData_size_check[(sizeof(SaveData) == 0x728) ? 1 : -1];
