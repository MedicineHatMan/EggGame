#include <CRI/adx_tlk.h>
#include <CRI/axrna.h>

// 0x80227068
void ADXRNA_Init(void)
{
	AXRNA_Init();
}

// 0x80227048
void ADXRNA_Finish(void)
{
	AXRNA_Finish();
}

// 0x80227028
void ADXRNA_EntryErrFunc(void (*func)(void*, char*), void* obj)
{
	AXRNA_EntryErrFunc(func, obj);
}

// 0x80227008
ADXRNA ADXRNA_Create(SJ* sjo, int maxnch, short* work)
{
	return AXRNA_Create(sjo, maxnch, work);
}

// 0x80226FC4
void ADXRNA_Destroy(ADXRNA rna)
{
	AXRNA_SetPlaySw(rna, 0);
	AXRNA_SetTransSw(rna, 0);
	AXRNA_Destroy(rna);
}

// 0x80226FA4
void ADXRNA_SetTransSw(ADXRNA rna, int sw)
{
	AXRNA_SetTransSw(rna, sw);
}

// 0x80226F84
void ADXRNA_SetPlaySw(ADXRNA rna, int sw)
{
	AXRNA_SetPlaySw(rna, sw);
}

// 0x80226F64
int ADXRNA_GetNumData(ADXRNA rna)
{
	return AXRNA_GetNumData(rna);
}

// 0x80226F44
int ADXRNA_GetNumRoom(ADXRNA rna)
{
	return AXRNA_GetNumRoom(rna);
}

// 0x80226F24
void ADXRNA_ExecServer(void)
{
	AXRNA_ExecServer();
}

// 0x80226F04
void ADXRNA_SetNumChan(ADXRNA rna, int nch)
{
	AXRNA_SetNumChan(rna, nch);
}

// 0x80226EE4
void ADXRNA_SetSfreq(ADXRNA rna, int sfreq)
{
	AXRNA_SetSfreq(rna, sfreq);
}

// 0x80226EC4
void ADXRNA_SetOutVol(ADXRNA rna, int vol)
{
	AXRNA_SetOutVol(rna, vol);
}

// 0x80226EA4
void ADXRNA_SetOutPan(ADXRNA rna, int ch, int pan)
{
	AXRNA_SetOutPan(rna, ch, pan);
}

// 0x80226E84
void ADXRNA_SetBitPerSmpl(ADXRNA rna, int bps)
{
	AXRNA_SetBitPerSmpl(rna, bps);
}

// 0x80226E64
int ADXRNA_GetPauseTime(ADXRNA rna)
{
	return AXRNA_GetPauseTime(rna);
}

// 0x80226E60
void ADXRNA_SetTotalNumSmpl(ADXRNA rna, int nsmpl)
{
}

// 0x80226E3C
int ADXRNA_SetStmHdInfo(ADXRNA rna, void* info)
{
	AXRNA_SetStmHdInfo(rna, info);
	return 0;
}

// 0x80226E1C
void ADXRNA_SetSfreqFix(ADXRNA rna, short sw)
{
	AXRNA_SetSfreqFix(rna, sw);
}
