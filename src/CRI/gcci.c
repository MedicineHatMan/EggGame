#include <Dolphin/os.h>
#include <Dolphin/dvd.h>
#include <CRI/cri_cvfs.h>
#include <CRI/adx.h>
#include <CRI/cri_std.h>

#define GCCI_MAX_HNDL       40
#define GCCI_SCT            0x800
#define GCCI_PATH_LEN       0x100
#define GCCI_CANCEL_TIMEOUT 2000

#define GCCI_STAT_STOP 0
#define GCCI_STAT_END  1
#define GCCI_STAT_EXEC 2
#define GCCI_STAT_ERR  3

typedef struct {
	signed char used;  /* 00 */
	signed char unk01; /* 01 */
	signed char stat;  /* 02 */
	char pad03;        /* 03 */
	int unk04;         /* 04 */
	void* rdbuf;       /* 08 the caller's buffer */
	int cmdstat;       /* 0C DVD command block state */
	int sctlen;        /* 10 */
	int fsize;         /* 14 */
	int fnsct;         /* 18 */
	int pos;           /* 1C in sectors */
	int trsize;        /* 20 bytes of the last read */
	int reqnsct;       /* 24 */
	DVDFileInfo fi;    /* 28 */
} GCCI_OBJ;

typedef GCCI_OBJ* GCCI;

/* The build string first in .rodata */
const char gcci_build[] = "\nGCCI Ver.1.09 Build:Mar  3 2003 16:46:38\n";

/* The command state the first of the file's variables in .bss */
typedef struct {
	int laststat;       /* 00 the last command block state seen */
	unsigned char stat; /* 04 the last handle state set */
	char pad05[3];      /* 05 */
	int cancel;         /* 08 set around DVDCancel */
} GCCI_WORK;

/* In .bss gcci_work gcci_errobj gcci_errfn gcci_hn gcci_unk */
/* 0 reads asynchronously otherwise DVDReadPrio waits */
extern int gcci_rdmode;
/* The root the file names are under at 0x802B7448 right after gcci_unk */
extern char gcci_tmp_path[0x100];
/* Only copied onto itself by gcCiGetInterface */
int gcci_unk;
GCCI_OBJ gcci_hn[GCCI_MAX_HNDL];
CVFS_ERRFN gcci_errfn;
void* gcci_errobj;
GCCI_WORK gcci_work;
extern CVFS_DEV gcci_dev_vtbl;

#define GCCI_TICKS_MS(t) OSTicksToMilliseconds(t)

void gcCiStopTr(void* fd);
void gcci_read_cb(s32 result, DVDFileInfo* fi);

static inline void gcci_CallErr(const char* msg)
{
	if (gcci_errfn != 0) {
		gcci_errfn(gcci_errobj, msg, 0);
	}
}

static inline void gcci_CallErrW(const char* msg, void* hn)
{
	if (gcci_errfn != 0) {
		gcci_errfn(gcci_errobj, msg, hn);
	}
}

static inline int gcci_IsIdle(GCCI hn)
{
	return hn->cmdstat == DVD_STATE_END || hn->cmdstat == DVD_STATE_CANCELED;
}

static inline void gcci_ConvPath(char* path)
{
	unsigned int n;
	unsigned int i;

	n = strlen(path);
	for (i = 0; i < n; i++) {
		if (path[i] == '\\') {
			path[i] = '/';
		}
	}
}

static inline void gcci_ExecServer(void)
{
	int i;
	GCCI hn;
	int len;
	int over;
	char* p;

	for (i = 0, hn = gcci_hn; i < GCCI_MAX_HNDL; i++, hn++) {
		if (hn->used != 1 || hn->stat != GCCI_STAT_EXEC) {
			continue;
		}
		hn->cmdstat        = DVDGetCommandBlockStatus(&hn->fi.cBlock);
		gcci_work.laststat = hn->cmdstat;
		switch (hn->cmdstat) {
		case DVD_STATE_FATAL_ERROR:
			hn->stat       = GCCI_STAT_ERR;
			gcci_work.stat = GCCI_STAT_ERR;
			break;
		case DVD_STATE_END:
			len = hn->reqnsct * hn->sctlen;
			DCInvalidateRange(hn->rdbuf, len);
			hn->trsize = len;
			hn->pos += hn->reqnsct;
			if (hn->pos * hn->sctlen > hn->fsize) {
				over = hn->pos * hn->sctlen - hn->fsize;
				p    = (char*)hn->rdbuf + hn->trsize - over;
				memset(p, 0, over);
				DCStoreRange(p, over);
			}
			hn->stat       = GCCI_STAT_END;
			gcci_work.stat = GCCI_STAT_END;
			break;
		case DVD_STATE_CANCELED:
			len = DVDGetTransferredSize(&hn->fi);
			DCInvalidateRange(hn->rdbuf, len);
			hn->trsize = hn->sctlen * (len / hn->sctlen);
			hn->pos += len / hn->sctlen;
			hn->stat = gcci_work.stat = GCCI_STAT_STOP;
			break;
		}
	}
}

static inline void gcci_StopTr(void* fd)
{
	GCCI hn;
	unsigned int start;
	unsigned int now;
	unsigned int ms;
	int ret;

	hn = (GCCI)fd;
	if (hn == 0) {
		gcci_CallErrW("E0092912:handl is null.", 0);
		return;
	}
	if (hn->stat == GCCI_STAT_END || hn->stat == GCCI_STAT_STOP) {
		return;
	}
	DVDGetCommandBlockStatus(&hn->fi.cBlock);
	DVDGetDriveStatus();
	gcci_work.cancel = 1;
	ret              = DVDCancel(&hn->fi.cBlock);
	gcci_work.cancel = 0;
	if (ret < 0) {
		gcci_CallErrW("E0092917:DVDCancel failed.", hn);
		return;
	}
	start = GCCI_TICKS_MS(OSGetTick());
	while (!gcci_IsIdle(hn)) {
		hn->cmdstat        = DVDGetCommandBlockStatus(&hn->fi.cBlock);
		gcci_work.laststat = hn->cmdstat;
		now                = GCCI_TICKS_MS(OSGetTick());
		ms                 = (now >= start) ? now - start : 0xFFFFFFFF - start + now;
		if (ms > GCCI_CANCEL_TIMEOUT) {
			gcci_CallErrW("E0092918:DVDCancel time out.", hn);
			break;
		}
	}
	hn->stat       = GCCI_STAT_STOP;
	gcci_work.stat = GCCI_STAT_STOP;
	DVDGetCommandBlockStatus(&hn->fi.cBlock);
	DVDGetDriveStatus();
}

// 0x8022B0A4
void gcci_SetRdMode(int a, int b, int c, int rdmode)
{
	gcci_rdmode = rdmode;
}

// 0x8022B038
CVFS_DEV* gcCiGetInterface(void)
{
	*(volatile int*)&gcci_unk = *(volatile int*)&gcci_unk;
	memset(gcci_tmp_path, 0, sizeof(gcci_tmp_path));
	gcci_errfn  = 0;
	gcci_errobj = 0;
	memset(&gcci_work, 0, sizeof(gcci_work));
	return &gcci_dev_vtbl;
}

/* Moves each handle that is reading on once its DVD command has ended */
// 0x8022AEC0
void gcCiExecServer(void)
{
	gcci_ExecServer();
}

// 0x8022AEAC
void gcCiEntryErrFunc(CVFS_ERRFN func, void* obj)
{
	gcci_errfn  = func;
	gcci_errobj = obj;
}

// 0x8022AD48
int gcCiGetFileSize(const char* fname)
{
	DVDFileInfo fi;
	char path[GCCI_PATH_LEN];
	int len;

	if (fname == 0) {
		gcci_CallErr("E0092901:fname is null.(gcCiGetFileSize)");
		return 0;
	}
	strcpy(path, gcci_tmp_path);
	strcat(path, fname);
	gcci_ConvPath(path);
	if (DVDOpen(path, &fi) == 0) {
		gcci_CallErr("E0040201:can't open a file.(gcCiGetFileSize)");
		return 0;
	}
	len = fi.length;
	if (len & 0x80000000) {
		len = 0x7FFFFFFF;
	}
	if (DVDClose(&fi) == 0) {
		gcci_CallErr("E0040202:can't close a file.(gcCiGetFileSize)");
		return 0;
	}
	return len;
}

// 0x8022AB38
static inline GCCI gcci_AllocHn(void)
{
	GCCI hn;
	int i;

	hn = 0;
	for (i = 0; i < GCCI_MAX_HNDL; i++) {
		if (gcci_hn[i].used == 0) {
			hn = &gcci_hn[i];
			break;
		}
	}
	return hn;
}

void* gcCiOpen(const char* fname, void* dir, int rw)
{
	GCCI hn;
	int len;
	char path[GCCI_PATH_LEN];

	if (fname == 0) {
		gcci_CallErrW("E0092908:fname is null.(gcCiOpen)", 0);
		return 0;
	}
	if (rw != 0) {
		gcci_CallErrW("E0092909:rw is illigal.(gcCiOpen)", 0);
		return 0;
	}
	hn = gcci_AllocHn();
	if (hn == 0) {
		gcci_CallErrW("E0092910:not enough handle resource.(gcCiOpen)", 0);
		return 0;
	}
	strcpy(path, gcci_tmp_path);
	strcat(path, fname);
	gcci_ConvPath(path);
	if (DVDOpen(path, &hn->fi) == 0) {
		gcci_CallErrW("E0092911:DVDOpen fail.(gcCiOpen)", 0);
		memset(hn, 0, sizeof(GCCI_OBJ));
		return 0;
	}
	hn->sctlen = GCCI_SCT;
	len        = hn->fi.length;
	if (len & 0x80000000) {
		len = 0x7FFFFFFF;
	}
	hn->fsize   = len;
	hn->fnsct   = (hn->sctlen - 1 + hn->fsize) / hn->sctlen;
	hn->pos     = 0;
	hn->rdbuf   = 0;
	hn->reqnsct = 0;
	hn->trsize  = 0;
	hn->stat    = GCCI_STAT_STOP;
	hn->used    = 1;
	return hn;
}

// 0x8022A974
void gcCiClose(void* fd)
{
	GCCI hn;

	hn = (GCCI)fd;
	if (hn == 0) {
		return;
	}
	gcci_StopTr(fd);
	DVDClose(&hn->fi);
	hn->used = 0;
	memset(hn, 0, sizeof(GCCI_OBJ));
}

// 0x8022A8A8
int gcCiSeek(void* fd, int ofst, int whence)
{
	GCCI hn;

	hn = (GCCI)fd;
	if (hn == 0) {
		gcci_CallErr("E0092912:handl is null.");
		return 0;
	}
	if (whence == CVFS_SEEK_SET) {
		hn->pos = ofst;
	} else if (whence == CVFS_SEEK_END) {
		hn->pos = hn->fnsct + ofst;
	} else if (whence == CVFS_SEEK_CUR) {
		hn->pos += ofst;
	}
	hn->pos = hn->pos < hn->fnsct ? hn->pos : hn->fnsct;
	hn->pos = (hn->pos > 0L) ? hn->pos : 0L;
	return hn->pos;
}

// 0x8022A848
int gcCiTell(void* fd)
{
	GCCI hn;

	hn = (GCCI)fd;
	if (hn == 0) {
		gcci_CallErr("E0092912:handl is null.");
		return 0;
	}
	return hn->pos;
}

/* Starts reading nsct sectors at the position unless another handle is reading */
// 0x8022A420
static inline int gcci_IsReady(GCCI hn)
{
	return hn->stat == GCCI_STAT_END || hn->stat == GCCI_STAT_STOP;
}

static inline int gcci_IsBusy(void)
{
	int i;

	for (i = 0; i < GCCI_MAX_HNDL; i++) {
		if (gcci_hn[i].used == 1 && gcci_hn[i].stat == GCCI_STAT_EXEC) {
			return 1;
		}
	}
	return 0;
}

int gcCiReqRd(void* fd, int nsct, void* buf)
{
	GCCI hn;
	int pos;
	int len;
	int ret;

	hn = (GCCI)fd;
	if (hn == 0) {
		gcci_CallErrW("E0092912:handl is null.", 0);
		return 0;
	}
	if (nsct < 0) {
		gcci_CallErrW("E0092913:nsct < 0.(gcCiReqRd)", hn);
		return 0;
	}
	if (buf == 0) {
		gcci_CallErrW("E0092914:buf is null.(gcCiReqRd)", hn);
		return 0;
	}
	if (!gcci_IsReady(hn)) {
		return 0;
	}
	if (!gcci_IsIdle(hn)) {
		return 0;
	}
	if (gcci_IsBusy()) {
		return 0;
	}
	if (nsct == 0) {
		hn->stat       = GCCI_STAT_END;
		gcci_work.stat = GCCI_STAT_END;
		return 0;
	}
	hn->trsize  = 0;
	hn->rdbuf   = buf;
	hn->reqnsct = nsct;
	gcci_ExecServer();
	pos = hn->pos * hn->sctlen;
	len = hn->reqnsct * hn->sctlen;
	if (pos + len > hn->fsize) {
		len = hn->fsize - pos;
		if (len < 0) {
			hn->stat       = GCCI_STAT_END;
			gcci_work.stat = GCCI_STAT_END;
			return nsct;
		}
	}
	len = (len + 0x1F) & ~0x1F;
	DCInvalidateRange(buf, len);
	if (gcci_rdmode == 0) {
		ret = DVDReadAsyncPrio(&hn->fi, buf, len, pos, gcci_read_cb, 2);
	} else {
		ret = DVDReadPrio(&hn->fi, buf, len, pos, 2);
	}
	if (ret == 0) {
		return 0;
	}
	hn->stat       = GCCI_STAT_EXEC;
	gcci_work.stat = GCCI_STAT_EXEC;
	return hn->reqnsct;
}

/* Cancels the read waiting up to two seconds for DVD to say it has */
// 0x8022A280
void gcCiStopTr(void* fd)
{
	GCCI hn;
	unsigned int start;
	unsigned int now;
	unsigned int ms;
	int ret;

	hn = (GCCI)fd;
	if (hn == 0) {
		gcci_CallErrW("E0092912:handl is null.", 0);
		return;
	}
	if (hn->stat == GCCI_STAT_END || hn->stat == GCCI_STAT_STOP) {
		return;
	}
	DVDGetCommandBlockStatus(&hn->fi.cBlock);
	DVDGetDriveStatus();
	gcci_work.cancel = 1;
	ret              = DVDCancel(&hn->fi.cBlock);
	gcci_work.cancel = 0;
	if (ret < 0) {
		gcci_CallErrW("E0092917:DVDCancel failed.", hn);
		return;
	}
	start = GCCI_TICKS_MS(OSGetTick());
	while (!gcci_IsIdle(hn)) {
		hn->cmdstat        = DVDGetCommandBlockStatus(&hn->fi.cBlock);
		gcci_work.laststat = hn->cmdstat;
		now                = GCCI_TICKS_MS(OSGetTick());
		ms                 = (now >= start) ? now - start : 0xFFFFFFFF - start + now;
		if (ms > GCCI_CANCEL_TIMEOUT) {
			gcci_CallErrW("E0092918:DVDCancel time out.", hn);
			break;
		}
	}
	hn->stat       = GCCI_STAT_STOP;
	gcci_work.stat = GCCI_STAT_STOP;
	DVDGetCommandBlockStatus(&hn->fi.cBlock);
	DVDGetDriveStatus();
}

// 0x8022A21C
int gcCiGetStat(void* fd)
{
	GCCI hn;

	hn = (GCCI)fd;
	if (hn == 0) {
		gcci_CallErr("E0092912:handl is null.");
		return 0;
	}
	return hn->stat;
}

// 0x8022A1BC
int gcCiGetSctLen(void* fd)
{
	GCCI hn;

	hn = (GCCI)fd;
	if (hn == 0) {
		gcci_CallErr("E0040301:handl is null.");
		return 0;
	}
	return hn->sctlen;
}

/* Changes the sector length keeping the position in bytes */
// 0x8022A0D8
void gcCiSetSctLen(void* fd, int len)
{
	GCCI hn;
	int bpos;

	hn = (GCCI)fd;
	if (hn == 0) {
		gcci_CallErr("E0040302:handl is null.");
		return;
	}
	if (hn->sctlen % 32 != 0) {
		gcci_CallErr("E0040303:invalidate size.");
		return;
	}
	bpos       = hn->pos * hn->sctlen;
	hn->sctlen = len;
	hn->fnsct  = (hn->sctlen - 1 + hn->fsize) / hn->sctlen;
	hn->pos    = bpos / hn->sctlen;
	hn->trsize = hn->reqnsct * len;
}

// 0x8022A078
int gcCiGetNumTr(void* fd)
{
	GCCI hn;

	hn = (GCCI)fd;
	if (hn == 0) {
		gcci_CallErr("E0092912:handl is null.");
		return 0;
	}
	return hn->trsize;
}

/* The DVD read callback the server sees the end through the command block */
// 0x8022A074
void gcci_read_cb(s32 result, DVDFileInfo* fi)
{
}

CVFS_DEV gcci_dev_vtbl
    = { gcCiExecServer, gcCiEntryErrFunc, gcCiGetFileSize, 0,           gcCiOpen, gcCiClose, gcCiSeek, gcCiTell, gcCiReqRd, 0, gcCiStopTr,
        gcCiGetStat,    gcCiGetSctLen,    gcCiSetSctLen,   gcCiGetNumTr };
