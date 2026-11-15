#include <CRI/lsc.h>
#include <CRI/sj.h>

// 0x8022B1C4
void LSC_LockCrs(int* key)
{
	SJCRS_Lock();
}

// 0x8022B1A4
void LSC_UnlockCrs(int* key)
{
	SJCRS_Unlock();
}
