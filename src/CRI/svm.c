#include <core/types.h>
#include <stdarg.h>
#include <CRI/adx.h>
#include <CRI/cri_std.h>
#include <CRI/svm.h>

typedef struct {
	SVM_SVRFN fn;
	void* obj;
} SVM_CB;

const char* const volatile svm_build = "\nSVM/GC Ver.1.51 Build:Mar  3 2003 16:46:55\n";

typedef struct {
	void (*fn)(void*);
	void* obj;
} SVM_LOCKCB;

typedef struct {
	void (*fn)(void*, char*);
	void* obj;
} SVM_ERRCB;

int svm_svr_cnt[8];         /* 0x802BED30 */
int svm_svr_busy[8];        /* 0x802BED10 */
SVM_CB svm_svr[8][6];       /* 0x802BEB90 */
SVM_CB svm_bdr[8];          /* 0x802BEB50 */
SVM_LOCKCB svm_lockcb;      /* 0x802BEB48 */
SVM_LOCKCB svm_unlockcb;    /* 0x802BEB40 */
SVM_ERRCB svm_errcb;        /* 0x802BEB38 */
char svm_err_msg[0x80];     /* 0x802BEAB8 */
int (*svm_tas_fn)(int*);    /* 0x802BEAB4 */
char svm_reserved[0x20];    /* 0x802BEA94 */
volatile int svm_lock_type; /* 0x802BEA90 */
volatile int svm_lock_lev;  /* 0x802BEA8C */
volatile int svm_init_cnt;  /* 0x802BEA88 */

void SVM_CallErr(const char* fmt, ...);

static inline void svm_callcb(SVM_LOCKCB* cb)
{
	cb->fn(cb->obj);
}

static inline void svm_lock(int type)
{
	if (svm_lockcb.fn != 0) {
		svm_callcb(&svm_lockcb);
		if (svm_lock_lev == 0) {
			svm_lock_type = type;
		}
		svm_lock_lev++;
	}
}

static inline void svm_unlock(int type)
{
	if (svm_unlockcb.fn != 0) {
		svm_lock_lev--;
		if (svm_lock_lev == 0) {
			if (svm_lock_type != type) {
				SVM_CallErr("2103102:SVM:svm_unlock:lock type miss match.(type org=%d, type now=%d)", svm_lock_type, type);
			}
			svm_lock_type = 0;
		}
		svm_callcb(&svm_unlockcb);
	}
}

static inline void svm_err(const char* msg)
{
	strncpy(svm_err_msg, msg, 0x7F);
	if (svm_errcb.fn != 0) {
		svm_errcb.fn(svm_errcb.obj, svm_err_msg);
	}
}

// 0x8022DFA0
void SVM_Lock(void)
{
	svm_lock(1);
}

// 0x8022DF10
void SVM_Unlock(void)
{
	svm_unlock(1);
}

// 0x8022DEA8
void SVM_LockRsc(void)
{
	svm_lock(3);
}

// 0x8022DE40
void SVM_LockVar(void)
{
	svm_lock(5);
}

// 0x8022DDB0
void SVM_UnlockRsc(void)
{
	svm_unlock(3);
}

// 0x8022DD20
void SVM_UnlockVar(void)
{
	svm_unlock(5);
}

// 0x8022DC48
void SVM_CallErr(const char* fmt, ...)
{
	va_list args;
	memset(svm_err_msg, 0, sizeof(svm_err_msg));
	va_start(args, fmt);
	vsprintf(svm_err_msg, fmt, args);
	va_end(args);
	if (svm_errcb.fn != 0) {
		svm_errcb.fn(svm_errcb.obj, svm_err_msg);
	}
}

// 0x8022DBF0
void SVM_CallErr1(const char* msg)
{
	svm_err(msg);
}

/* Not in the DOL either */
void SVM_CallErr2(const char* msg1, const char* msg2)
{
	strncpy(svm_err_msg, msg1, 0x7F);
	strncat(svm_err_msg, " ", 0x7F - strlen(svm_err_msg));
	strncat(svm_err_msg, msg2, 0x7F - strlen(svm_err_msg));
	if (svm_errcb.fn != 0) {
		svm_errcb.fn(svm_errcb.obj, svm_err_msg);
	}
}

// 0x8022DA74
int SVM_SetCbSvr(int type, SVM_SVRFN fn, void* obj)
{
	SVM_CB* cb;
	int i;

	svm_lock(1);
	cb = svm_svr[type];
	for (i = 0; i < 6; cb++, i++) {
		if (cb->fn == 0) {
			cb->fn  = fn;
			cb->obj = obj;
			break;
		}
	}
	if (i == 6) {
		svm_err("1051001:SVM_SetCbSvr:too many server function");
	}
	svm_unlock(1);
	if (i == 6) {
		return -1;
	}
	return i;
}

// 0x8022D92C
void SVM_DelCbSvr(int type, int id)
{
	if (id < 0 || id >= 6) {
		svm_err("1051002:SVM_DelCbSvr:illegal id");
	}
	svm_lock(1);
	svm_svr[type][id].fn  = 0;
	svm_svr[type][id].obj = 0;
	svm_unlock(1);
}

// 0x8022D778
void SVM_SetCbSvrId(int type, int id, SVM_SVRFN fn, void* obj)
{
	SVM_CB* cb;

	if (id < 0 || id >= 6) {
		svm_err("1071201:SVM_SetCbSvrId:illegal id");
	}
	if (type < 0 || id >= 8) {
		svm_err("1071202:SVM_SetCbSvrId:illegal svtype");
	}
	svm_lock(1);
	cb = &svm_svr[type][id];
	if (cb->fn != 0) {
		svm_err("2100801:SVM_SetCbSvrId:over write callback function.");
	}
	cb->fn  = fn;
	cb->obj = obj;
	svm_unlock(1);
}

/* Not in the DOL the linker stripped it but its two messages are between svm_unlock's and SVM_SetCbSvrId's */
void SVM_ExecSvrFuncId(int type, int id)
{
	if (id < 0 || id >= 6) {
		svm_err("1071301:SVM_ExecSvrFuncId:illegal id");
	}
	if (type < 0 || type >= 8) {
		svm_err("1071302:SVM_ExecSvrFuncId:illegal svtype");
	}
	if (svm_svr[type][id].fn != 0) {
		svm_svr_busy[type] = 1;
		svm_svr[type][id].fn(svm_svr[type][id].obj);
		svm_svr_busy[type] = 0;
	}
}

// 0x8022D674
void SVM_SetCbBdr(int id, SVM_SVRFN fn, void* obj)
{
	svm_lock(1);
	svm_bdr[id].fn  = fn;
	svm_bdr[id].obj = obj;
	svm_unlock(1);
}

// 0x8022D630
void SVM_CallBdr(int id)
{
	if (svm_bdr[id].fn != 0) {
		svm_bdr[id].fn(svm_bdr[id].obj);
	}
}

// 0x8022D620
void SVM_SetCbLock(void (*fn)(void*), void* obj)
{
	svm_lockcb.fn  = fn;
	svm_lockcb.obj = obj;
}

// 0x8022D610
void SVM_SetCbUnlock(void (*fn)(void*), void* obj)
{
	svm_unlockcb.fn  = fn;
	svm_unlockcb.obj = obj;
}

static inline int svm_execsvr(int type)
{
	SVM_CB* cb = svm_svr[type];
	int i;
	int ret = 0;

	for (i = 0; i < 6; i++, cb++) {
		if (cb->fn != 0) {
			svm_svr_busy[type] = 1;
			ret |= cb->fn(cb->obj);
			svm_svr_busy[type] = 0;
		}
	}
	svm_svr_cnt[type]++;
	return ret;
}

// 0x8022D580
int SVM_ExecSvrVsync(void)
{
	return svm_execsvr(2);
}

// 0x8022D4F0
int SVM_ExecSvrFs(void)
{
	return svm_execsvr(4);
}

// 0x8022D460
int SVM_ExecSvrMain(void)
{
	return svm_execsvr(6);
}

// 0x8022D3C8
void SVM_Init(void)
{
	int i;
	int* cnt;

	if (svm_init_cnt == 0) {
		memset(svm_svr_busy, 0, sizeof(svm_svr_busy));
		memset(&svm_lockcb, 0, 8);
		memset(&svm_unlockcb, 0, 8);
		cnt = svm_svr_cnt;
		for (i = 0; i < 6; i++) {
			*cnt++ = 0;
		}
		svm_tas_fn = 0;
	}
	svm_init_cnt++;
}

// 0x8022D320
void SVM_Finish(void)
{
	int i;
	int* cnt;

	svm_init_cnt--;
	if (svm_init_cnt == 0) {
		memset(svm_svr_busy, 0, sizeof(svm_svr_busy));
		memset(&svm_lockcb, 0, 8);
		memset(&svm_unlockcb, 0, 8);
		cnt = svm_svr_cnt;
		for (i = 0; i < 6; i++) {
			cnt[i] = 0;
		}
		svm_tas_fn = 0;
		memset(&svm_errcb, 0, 8);
	}
}

// 0x8022D20C
int SVM_TestAndSet(int* mem)
{
	int ret;

	if (svm_tas_fn != 0) {
		ret = svm_tas_fn(mem);
	} else {
		svm_lock(1);
		ret  = *mem;
		*mem = 1;
		ret  = ret != 1;
		svm_unlock(1);
	}
	return ret;
}
