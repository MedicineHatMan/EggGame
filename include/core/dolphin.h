#ifndef _DOLPHIN_H
#define _DOLPHIN_H

#include "core/types.h"
#include "io/nt_arc.h"
#include "Dolphin/dvd.h"
#include "Dolphin/os.h"
#include "Dolphin/GX/GXTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// Only SDK functions whose bodies have been checked against the DOL belong here

void DVDInit(void);
s32 DVDGetDriveStatus(void);

#define PAD_BUTTON_LEFT  0x0001
#define PAD_BUTTON_RIGHT 0x0002
#define PAD_BUTTON_DOWN  0x0004
#define PAD_BUTTON_UP    0x0008
#define PAD_BUTTON_A     0x0100
#define PAD_BUTTON_B 0x0200

void PADControlAllMotors(const u32* commands);

void DCStoreRange(void* addr, u32 nBytes);

int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, u32 n);
char* strstr(const char* str, const char* find);

#ifdef __cplusplus
}
#endif

#endif
