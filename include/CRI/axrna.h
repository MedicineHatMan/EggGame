#ifndef CRI_AXRNA_H
#define CRI_AXRNA_H

#include <CRI/cri_types.h>
#include "Dolphin/ar.h"

extern void ARQPostRequest(ARQRequest* task, u32 owner, u32 type, u32 priority, u32 source, u32 dest, u32 length,
                           void (*callback)(u32));
extern u32 ARAlloc(u32 length);
extern u32 ARFree(u32* length);

#define AXRNARES_MAX 32
#define AXRNARES_SIZE 0x40000
#define AXRNARES_BUF 0x2000

/* One ARAM buffer The address and length are in samples */
typedef struct {
	int used; /* 00 */
	u32 adr;  /* 04 */
	u32 len;  /* 08 */
} AXRNARES_OBJ;

typedef AXRNARES_OBJ* AXRNARES;

void axrna_call_errfn(const char* msg);
void axrna_entry_errfn(void (*func)(void*, char*), void* obj);
AXRNARES AXRNARES_Create(void);
void AXRNARES_Destroy(AXRNARES res);
u32 AXRNARES_GetAdr(AXRNARES res);
u32 AXRNARES_GetLen(AXRNARES res);
void AXRNARES_Init(void);
void AXRNARES_Finish(void);

void AXRNA_Init(void);
void AXRNA_Finish(void);
void AXRNA_EntryErrFunc(void (*func)(void*, char*), void* obj);
AXRNA AXRNA_Create(SJ* sj, int maxnch, short* work);
void AXRNA_Destroy(AXRNA rna);
void AXRNA_SetTransSw(AXRNA rna, int sw);
void AXRNA_SetPlaySw(AXRNA rna, int sw);
int AXRNA_GetNumData(AXRNA rna);
int AXRNA_GetNumRoom(AXRNA rna);
void AXRNA_ExecServer(void);
void AXRNA_SetNumChan(AXRNA rna, int nch);
void AXRNA_SetSfreq(AXRNA rna, int sfreq);
void AXRNA_SetOutVol(AXRNA rna, int vol);
void AXRNA_SetOutPan(AXRNA rna, int ch, int pan);
void AXRNA_SetBitPerSmpl(AXRNA rna, int bps);
int AXRNA_GetPauseTime(AXRNA rna);
int AXRNA_SetStmHdInfo(AXRNA rna, void* info);
void AXRNA_SetSfreqFix(AXRNA rna, s16 sw);

#endif
