#include <CRI/lsc.h>
#include <CRI/adx.h>

static const char* const volatile lsc_build = "\nLSC/GC Ver.2.11 Build:Mar  3 2003 16:46:37\n";
LSC_OBJ lsc_obj[LSC_MAX_OBJ];
static int lsc_init_cnt;

// 0x8022BC90
void LSC_Init(void)
{
	int key;

	lsc_build;
	LSC_LockCrs(&key);
	if (lsc_init_cnt == 0) {
		memset(lsc_obj, 0, sizeof(lsc_obj));
		LSC_EntryErrFunc(0, 0);
	}
	lsc_init_cnt++;
	LSC_UnlockCrs(&key);
}

// 0x8022BBEC
void LSC_Finish(void)
{
	int key;
	int i;

	LSC_LockCrs(&key);
	if (--lsc_init_cnt == 0) {
		for (i = 0; i < LSC_MAX_OBJ; i++) {
			if (lsc_obj[i].used == 1) {
				LSC_Destroy(&lsc_obj[i]);
			}
		}
		memset(lsc_obj, 0, sizeof(lsc_obj));
		LSC_EntryErrFunc(0, 0);
	}
	LSC_UnlockCrs(&key);
}
