#include <CRI/adx.h>

int adxcrs_lock_cnt;

// 0x8021F0D4
void ADXCRS_Init(void)
{
	adxcrs_lock_cnt = 0;
}

// 0x8021F090
void ADXCRS_Lock(void)
{
	if (adxcrs_lock_cnt == 0) {
		SVM_Lock();
	}
	adxcrs_lock_cnt++;
}

// 0x8021F058
void ADXCRS_Unlock(void)
{
	if (--adxcrs_lock_cnt == 0) {
		SVM_Unlock();
	}
}
