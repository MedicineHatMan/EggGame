#ifndef _DOLPHIN__DVD_H
#define _DOLPHIN__DVD_H

#include "Dolphin/os.h"
#include "Dolphin/dvd.h"

#ifdef __cplusplus
extern "C" {
#endif

void __DVDInterruptHandler(__OSInterrupt interrupt, OSContext* context);
DVDQueue* __DVDPopWaitingQueue(void);
BOOL __DVDTestAlarm(OSAlarm* alarm);
extern u32 __DVDLongFileNameFlag;

#ifdef __cplusplus
};
#endif

#endif
