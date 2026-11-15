#ifndef _AUDIOLIB_H
#define _AUDIOLIB_H

#include "core/types.h"

extern "C" {
void ADXF_LoadPartitionNw(void* handle, ResName* name, int, void* user);
void ADXF_StopPtLd();
void ADXM_SetupThrd(int);
void ADXT_ClearErrCode(void*);
void* ADXT_Create(int, void*, u32);
void ADXT_Destroy(void*);
int ADXT_GetStat(void*);
void ADXT_Init();
void ADXT_Pause(void*, int);
void ADXT_SetOutVol(void*, int);
void ADXT_StartAfs(void*, int, int);
void ADXT_StartFname(void*, const char*);
void ADXT_Stop(void*);
void ADXT_SetDecodeSteAsMonoSw();
void ADXT_SetupDvdFs(int);
int ADXF_GetPtStatEx(void* handle);
void gcaxLoaderAlloc(int* handle);
void gcaxLoaderFree(int handle);
void gcaxLoaderLoad(int handle);
void gcaxLoaderSet(int handle, void* data, int size, int, int);
void gcaxSndCallback();
void gcaxSndClosePort(u32);
void gcaxSndInit(int, int, int);
int gcaxSndMakeMsg(u32 port, u8* msg, int status, ...);
void gcaxSndOpenPort(void*);
int gcaxSndResetAll();
int gcaxSndReverbOff();
void gcaxSndReverbOn(int, int);
int gcaxSndSendBank(int, void*, u32);
void gcaxSndSendMsg(u32, void*, int);
int gcaxSndSetMasterLevel(s8);
void gcaxSndSetOutput(int);
void gcaxSndSetSurround(int);
}

#endif
