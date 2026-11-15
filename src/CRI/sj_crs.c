#include <CRI/sj.h>
#include <CRI/cri_std.h>

static int sjcrs_key;
static volatile int sjcrs_cnt;

// 0x8022BF9C
void SJCRS_Lock(void)
{
	if (sjcrs_cnt == 0) {
		sjcrs_key = OSDisableInterrupts();
	}
	sjcrs_cnt++;
}

// 0x8022BF50
void SJCRS_Unlock(void)
{
	sjcrs_cnt--;
	if (sjcrs_cnt == 0) {
		OSRestoreInterrupts(sjcrs_key);
	}
}
