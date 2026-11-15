#ifndef _IO_CARDMANAGER_H_
#define _IO_CARDMANAGER_H_

#include "core/types.h"

extern "C" {
void cardCreate();
int cardGetState();
void cardInit();
int cardIsWriting();
void cardLoadSave();
void cardShutdown();
void cardUpdate();
}

extern "C" {
extern u8 gCardImages[0x2000]; // the banner and icon images
}

#endif
