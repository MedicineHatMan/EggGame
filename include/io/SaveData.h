#ifndef _SAVEDATA_H
#define _SAVEDATA_H

#include "core/types.h"

/**
 * @size 0x8
 */
struct SaveRecord {
	int mTime;   // _00 milliseconds
	int mPoints; // _04
};

/**
 * One of the three save files. The names are ours.
 * @size 0x260
 */
struct SaveSlot {
	void clear(); // 0x80198F58

	u8 mKeyBits[0x80];         // _000 0x400 bits bit 0 is 0x80 of byte 0
	SaveRecord mRecords[0x38]; // _080
	u8 mLives;                 // _240
	u8 mSoundMode;             // _241 1 mono
	u8 mRumbleOff;             // _242 0 rumble on 1 off
	u8 _243;
	u8 mUsed;                  // _244
	u8 _245[0x1B];
};

/**
 * What goes on the memory card: three slots, the save time and a CRC over both. The names are ours.
 * @size 0x728
 */
struct SaveData {
	void clear();     // 0x80198FCC
	void stampTime(); // 0x8019900C
	u32 calcCrc();    // 0x80199050
	void updateCrc(); // 0x80199074
	int checkCrc();   // 0x801990A4

	SaveSlot mSlots[3]; // _000
	int mTime;          // _720 seconds
	u32 mCrc;           // _724
};

extern "C" {
u32 crc32(const u8* data, u32 size);         // 0x80198F08
int keyBitPos(int bit, int* byte, u8* mask); // 0x80198F80

void saveCreate();                              // 0x801990E0
void saveReset();                               // 0x8019910C
void saveRead(const SaveData* src);             // 0x80199140
void saveWrite(SaveData* dst);                  // 0x80199180
void saveSeal();                                // 0x801991B8
int saveTimeIs(u32 time);                        // 0x801991EC
SaveSlot* saveGetSlot(int slot);                // 0x80199224
SaveRecord* saveGetRecord(int slot, int stage); // 0x8019925C
void saveSelectSlot(int slot);                  // 0x801992B4
void saveCopySlot(int from, int to);            // 0x801992F4
void saveClearSlot(int slot);                   // 0x80199354
int saveSlotLives(int slot);                     // 0x80199380
int saveSlotUsed(int slot);                      // 0x801993B4
int saveGetKeyBit(int bit);                     // 0x801993F4
void saveSetKeyBit(int bit, int on);            // 0x8019945C
int saveGetTime(int stage);                     // 0x801994E4
int saveGetPoints(int stage);                   // 0x80199520
void saveSetTime(int stage, int time);          // 0x8019955C
void saveSetPoints(int stage, int points);      // 0x801995A0
int saveGetLives();                              // 0x801995E4
void saveSetLives(u8 lives);                    // 0x80199608
int saveGetSoundMode();                          // 0x80199644
void saveSetSoundMode(int mode);                // 0x8019967C
int saveGetRumbleOff();                          // 0x801996B8
void saveSetRumbleOff(int off);                 // 0x801996F0

extern SaveData* gSaveData; // the live save
extern int gSaveSlot;       // the slot being played
}

#endif
