#include <Dolphin/os.h>
#include <Dolphin/vi.h>
#include <CRI/adx.h>
#include <CRI/svm.h>
#include <CRI/adx_tlk.h>

typedef struct {
	int lock;   /* 00 taken while a lock is held */
	int safe;   /* 04 */
	int vsync;  /* 08 */
	int fs;     /* 0C */
	int main;   /* 10 */
	int mwidle; /* 14 */
} ADXM_TPRM;

#define ADXM_STACK_LEN 0x2000
#define ADXM_SAFE_STACK_LEN 0x1000

/* The file's variables one after another in .bss */
typedef struct {
	int unk00;                                   /* 00 */
	int setup_cnt;                               /* 04 */
	int lock_level;                              /* 08 */
	int border;                                  /* 0C set while the main thread waits for the idle thread */
	int safe_cnt;                                /* 10 */
	int vsync_cnt;                               /* 14 */
	int fs_cnt;                                  /* 18 */
	int mwidle_cnt;                              /* 1C */
	int unk20;                                   /* 20 */
	OSThread* main_thrd;                         /* 24 */
	OSThread mwidle_thrd;                        /* 28 */
	OSThread vsync_thrd;                         /* 340 */
	OSThread fs_thrd;                            /* 658 */
	OSThread safe_thrd;                          /* 970 */
	OSPriority save_prio;                        /* C88 */
	int in_lock;                                 /* C8C */
	volatile int fs_end;                                /* C90 */
	volatile int fs_run;                                  /* C94 */
	volatile int vsync_end;                               /* C98 */
	volatile int vsync_run;                               /* C9C */
	volatile int mwidle_end;                              /* CA0 */
	volatile int mwidle_run;                              /* CA4 */
	volatile int safe_end;                                /* CA8 */
	volatile int safe_run;                                /* CAC */
	char mwidle_stack[ADXM_STACK_LEN];           /* CB0 */
	char fs_stack[ADXM_STACK_LEN];               /* 2CB0 */
	char vsync_stack[ADXM_STACK_LEN];            /* 4CB0 */
	char safe_stack[ADXM_SAFE_STACK_LEN];        /* 6CB0 */
} ADXM_WORK;

ADXM_WORK adxm_work;
/* The same as adxm_work.setup_cnt */
int adxm_setup_thrd_flag;
ADXM_TPRM adxm_tprm = { 0 };

/* Called from the v-sync thread and when the idle thread goes to sleep */
typedef struct {
	void (*func)(void* obj);
	void* obj;
} ADXM_CB;
ADXM_CB adxm_cb = { 0 };

/* V-sync count */

/* Points at "ADXGC Ver.1.21 Build:Mar 3 2003 16:48:00" */
const char* const volatile adxm_build = "\nADXGC Ver.1.21 Build:Mar  3 2003 16:48:00\n";

typedef int (*SVM_SVRFN)(void*);

#define SVM_BDR_MWIDLE 6
#define ADXM_BORDER_WAIT 200000000

// 0x80226CF4
void ADXM_WaitVsync(void)
{
	VIWaitForRetrace();
}

// 0x80226CD4
void ADXM_Lock(void)
{
	SVM_Lock();
}

// 0x80226CB4
void ADXM_Unlock(void)
{
	SVM_Unlock();
}

/* The SVM lock raises the caller to the lock priority and wakes the safe thread which keeps the lower threads out */
// 0x80226C00
void adxm_svm_lock(void* obj)
{
	ADXM_WORK* w;
	BOOL intr;
	OSThread* thrd;
	OSPriority prio;

	w = &adxm_work;
	if (w->lock_level == 0) {
		intr = OSDisableInterrupts();
		OSDisableScheduler();
		w->in_lock = 1;
		thrd = OSGetCurrentThread();
		prio = OSGetThreadPriority(thrd);
		OSSetThreadPriority(thrd, adxm_tprm.lock);
		w->save_prio = prio;
		w->in_lock = 0;
		OSEnableScheduler();
		OSRestoreInterrupts(intr);
		OSResumeThread(&w->safe_thrd);
	}
	w->lock_level++;
}

// 0x80226B9C
void adxm_svm_unlock(void* obj)
{
	ADXM_WORK* w;
	OSThread* thrd;

	w = &adxm_work;
	if (--w->lock_level == 0) {
		thrd = OSGetCurrentThread();
		OSSuspendThread(&w->safe_thrd);
		OSSetThreadPriority(thrd, w->save_prio);
	}
}

/* Lets the idle thread run its server once at a high priority waiting until it has */
// 0x80226AE0
void adxm_goto_mwidle_border(void* obj)
{
	ADXM_WORK* w;
	int i;

	w = &adxm_work;
	if (w->mwidle_end != 1) {
		w->border = 1;
		OSSetThreadPriority(&w->mwidle_thrd, adxm_tprm.lock);
		for (i = 0; i < ADXM_BORDER_WAIT; i++) {
			OSResumeThread(&w->mwidle_thrd);
			if (w->border == 0) {
				break;
			}
		}
		if (i == ADXM_BORDER_WAIT) {
			SVM_CallErr1("1060102: Internal Error: adxm_goto_mwidle_border");
		}
		OSSetThreadPriority(&w->mwidle_thrd, adxm_tprm.mwidle);
	}
}

/* Runs while a lock is held so that no thread of a lower priority does */
// 0x80226AB0
void adxm_safe_thrd(void* prm)
{
	ADXM_WORK* w;

	w = &adxm_work;
	while (w->safe_run == 1) {
		w->safe_cnt++;
	}
	w->safe_end = 1;
}

// 0x80226A00
void adxm_vsync_thrd(void* prm)
{
	ADXM_WORK* w;
	ADXM_CB* cb;
	unsigned int* vcnt;

	w = &adxm_work;
	cb = &adxm_cb;
	vcnt = &adxt_svcnt;
	while (w->vsync_run == 1) {
		VIWaitForRetrace();
		w->vsync_cnt++;
		(*vcnt)++;
		SVM_ExecSvrVsync();
		if (w->mwidle_end == 0) {
			OSResumeThread(&w->mwidle_thrd);
			if (cb->func != 0) {
				cb->func(cb->obj);
			}
		}
	}
	w->vsync_end = 1;
}

// 0x802269A8
void adxm_fs_thrd(void* prm)
{
	ADXM_WORK* w;

	w = &adxm_work;
	while (w->fs_run == 1) {
		VIWaitForRetrace();
		w->fs_cnt++;
		SVM_ExecSvrFs();
	}
	w->fs_end = 1;
}

/* The idle thread runs the main server until it has nothing to do then sleeps until the next v-sync */
// 0x802268DC
void adxm_idle_thrd(void* prm)
{
	ADXM_WORK* w;
	ADXM_TPRM* tprm;
	ADXM_CB* cb;

	w = &adxm_work;
	tprm = &adxm_tprm;
	cb = &adxm_cb;
	while (w->mwidle_run == 1) {
		w->mwidle_cnt++;
		if (SVM_ExecSvrMain() == 0 || w->border == 1) {
			if (w->border == 1) {
				w->border = 0;
				OSSetThreadPriority(&w->mwidle_thrd, tprm->mwidle);
			}
			if (adxm_cb.func != 0) {
				adxm_cb.func(cb->obj);
			}
			OSSuspendThread(&w->mwidle_thrd);
		}
	}
	w->mwidle_end = 1;
}

// 0x802266C4
void ADXM_SetupThrd(ADXM_TPRM* tprm)
{
	ADXM_WORK* w;

	w = &adxm_work;
	adxm_build;
	if (w->setup_cnt == 0) {
		SVM_Init();
		SVM_SetCbLock(adxm_svm_lock, 0);
		SVM_SetCbUnlock(adxm_svm_unlock, 0);
		if (tprm == 0) {
			adxm_tprm.main = 16;
			adxm_tprm.lock = 1;
			adxm_tprm.safe = 8;
			adxm_tprm.vsync = 12;
			adxm_tprm.fs = 14;
			adxm_tprm.mwidle = 24;
		} else {
			adxm_tprm = *tprm;
		}
		OSCreateThread(&w->safe_thrd, (OSThreadStartFunction)adxm_safe_thrd, 0, (void*)((unsigned int)w->safe_stack + ADXM_SAFE_STACK_LEN),
		               ADXM_SAFE_STACK_LEN, adxm_tprm.safe, OS_THREAD_ATTR_DETACH);
		OSCreateThread(&w->vsync_thrd, (OSThreadStartFunction)adxm_vsync_thrd, 0, (void*)((unsigned int)w->vsync_stack + ADXM_STACK_LEN), ADXM_STACK_LEN,
		               adxm_tprm.vsync, OS_THREAD_ATTR_DETACH);
		OSCreateThread(&w->fs_thrd, (OSThreadStartFunction)adxm_fs_thrd, 0, (void*)((unsigned int)w->fs_stack + ADXM_STACK_LEN), ADXM_STACK_LEN,
		               adxm_tprm.fs, OS_THREAD_ATTR_DETACH);
		OSCreateThread(&w->mwidle_thrd, (OSThreadStartFunction)adxm_idle_thrd, 0, (void*)((unsigned int)w->mwidle_stack + ADXM_STACK_LEN),
		               ADXM_STACK_LEN, adxm_tprm.mwidle, OS_THREAD_ATTR_DETACH);
		w->main_thrd = OSGetCurrentThread();
		w->fs_run = 1;
		w->mwidle_run = 1;
		w->vsync_run = 1;
		w->safe_run = 1;
		w->fs_end = 0;
		w->mwidle_end = 0;
		w->vsync_end = 0;
		w->safe_end = 0;
		w->in_lock = 0;
		OSResumeThread(&w->vsync_thrd);
		OSResumeThread(&w->fs_thrd);
		OSResumeThread(&w->mwidle_thrd);
		SVM_SetCbBdr(SVM_BDR_MWIDLE, (SVM_SVRFN)adxm_goto_mwidle_border, 0);
	}
	w->setup_cnt++;
}

// 0x802266A8
int ADXM_IsSetupThrd(void)
{
	return adxm_setup_thrd_flag != 0;
}
