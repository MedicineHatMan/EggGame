#include <core/types.h>
#include <CRI/adx.h>
#include <CRI/cri_std.h>

#define MFCI_MAX_OBJ   40
#define MFCI_SCT_LEN   0x800
#define MFCI_FNAME_LEN 17

enum {
	MFCI_STAT_STOP     = 0,
	MFCI_STAT_COMPLETE = 1,
	MFCI_STAT_TRANS    = 2,
};

typedef struct {
	signed char used; /* 00 */
	signed char stat; /* 01 */
	int sct_len;      /* 04 */
	int fsize;        /* 08 */
	int fsize_sct;    /* 0C */
	int cur_sct;      /* 10 */
	int trns_size;    /* 14 */
	int rd_sct;       /* 18 */
	char fname[0x14]; /* 1C */
	int rd_ofst;      /* 30 */
	int rd_size;      /* 34 */
} MFCI_OBJ;

typedef MFCI_OBJ* MFCI;

typedef void (*MFCI_ERRFN)(void* obj, const char* msg, void* hn);

/* The CVFS device table in the order CVFS calls it */
typedef struct {
	void (*ExecServer)(void);
	void (*EntryErrFunc)(MFCI_ERRFN func, void* obj);
	int (*GetFileSize)(const char* fname);
	void* unk0C;
	MFCI (*Open)(const char* fname, void* dir, int rw);
	void (*Close)(MFCI hn);
	int (*Seek)(MFCI hn, int ofst, int whence);
	int (*Tell)(MFCI hn);
	int (*ReqRd)(MFCI hn, int nsct, void* buf);
	void* unk24;
	void (*StopTr)(MFCI hn);
	int (*GetStat)(MFCI hn);
	int (*GetSctLen)(MFCI hn);
	void (*SetSctLen)(MFCI hn, int len);
	int (*GetNumTr)(MFCI hn);
	void* unk3C[11];
} MFCI_IF;

const char* const volatile mfci_build = "\nMFCI/GC Ver.1.04 Build:Mar  3 2003 16:47:16\n";

static MFCI_OBJ mfci_obj[MFCI_MAX_OBJ];
static char mfci_err_msg[0x12C];
static void* mfci_err_obj;
static MFCI_ERRFN mfci_err_func;

void mfCiExecServer(void);
void mfCiEntryErrFunc(MFCI_ERRFN func, void* obj);
int mfCiGetFileSize(const char* fname);
MFCI mfCiOpen(const char* fname, void* dir, int rw);
void mfCiClose(MFCI hn);
int mfCiSeek(MFCI hn, int ofst, int whence);
int mfCiTell(MFCI hn);
int mfCiReqRd(MFCI hn, int nsct, void* buf);
void mfCiStopTr(MFCI hn);
int mfCiGetStat(MFCI hn);
int mfCiGetSctLen(MFCI hn);
void mfCiSetSctLen(MFCI hn, int len);
int mfCiGetNumTr(MFCI hn);

static MFCI_IF mfci_vtbl = {
	mfCiExecServer, mfCiEntryErrFunc, mfCiGetFileSize, 0, mfCiOpen, mfCiClose, mfCiSeek, mfCiTell, mfCiReqRd, 0, mfCiStopTr, mfCiGetStat,
	mfCiGetSctLen,  mfCiSetSctLen,    mfCiGetNumTr,
};

#define MFCI_ERR(msg, hn)                             \
	do {                                              \
		if (mfci_err_func != 0) {                     \
			mfci_err_func(mfci_err_obj, (msg), (hn)); \
		}                                             \
	} while (0)

// 0x8022EA04
MFCI_IF* mfCiGetInterface(void)
{
	mfci_build;
	return &mfci_vtbl;
}

// 0x8022E9F8
/* The DOL keeps the loop's first compare li r0,0 */
#pragma opt_propagation off
void mfCiExecServer(void)
{
	int i;
	for (i = 0; i < MFCI_MAX_OBJ; i++) { }
}
#pragma opt_propagation reset

// 0x8022E9E4
void mfCiEntryErrFunc(MFCI_ERRFN func, void* obj)
{
	mfci_err_func = func;
	mfci_err_obj  = obj;
}

/* Splits a file name into its address and size */
static void mfci_get_adr_size(const char* fname, unsigned long* adr, int* size)
{
	char* p;

	if (strlen(fname) != MFCI_FNAME_LEN) {
		sprintf(mfci_err_msg, "E01100308:length of '%s' is not 17 bytes.(mfci_get_adr_size)", fname);
		MFCI_ERR(mfci_err_msg, 0);
	}
	if (fname[8] != '.') {
		sprintf(mfci_err_msg, "E01100309:illegal file name format '%s'(mfci_get_adr_size)", fname);
		MFCI_ERR(mfci_err_msg, 0);
	}
	p    = (char*)fname;
	*adr = strtoul(p, &p, 16);
	if (*p != '\0') {
		p++;
	}
	if (size != 0) {
		*size = strtoul(p, &p, 16);
	}
}

// 0x8022E8D8
int mfCiGetFileSize(const char* fname)
{
	unsigned long adr;
	int size;

	mfci_get_adr_size(fname, &adr, &size);
	return size;
}

// 0x8022E6A8
static inline MFCI mfci_AllocHn(void)
{
	MFCI hn;
	int i;

	hn = 0;
	for (i = 0; i < MFCI_MAX_OBJ; i++) {
		if (mfci_obj[i].used == 0) {
			hn = &mfci_obj[i];
			break;
		}
	}
	return hn;
}

MFCI mfCiOpen(const char* fname, void* dir, int rw)
{
	MFCI hn;
	unsigned long adr;
	int size;

	if (fname == 0) {
		MFCI_ERR("E01100301:fname is null.(mfCiOpen)", 0);
		return 0;
	}
	if (rw != 0) {
		MFCI_ERR("E01100302:rw is illigal.(mfCiOpen)", 0);
		return 0;
	}
	hn = mfci_AllocHn();
	if (hn == 0) {
		MFCI_ERR("E01100303:not enough handle resource.(mfCiOpen)", 0);
		return 0;
	}
	strcpy(hn->fname, fname);
	hn->sct_len = MFCI_SCT_LEN;
	mfci_get_adr_size(hn->fname, &adr, &size);
	hn->fsize     = size;
	hn->fsize_sct = (hn->sct_len - 1 + hn->fsize) / hn->sct_len;
	hn->cur_sct   = 0;
	hn->rd_sct    = 0;
	hn->trns_size = 0;
	hn->stat      = MFCI_STAT_STOP;
	hn->used      = 1;
	return hn;
}

// 0x8022E610
void mfCiClose(MFCI hn)
{
	if (hn == 0) {
		return;
	}
	mfCiStopTr(hn);
	if (hn->used == 1) {
		hn->used = 0;
		memset(hn, 0, sizeof(MFCI_OBJ));
	}
}

// 0x8022E51C
int mfCiSeek(MFCI hn, int ofst, int whence)
{
	if (hn == 0) {
		MFCI_ERR("E01100305:handl is null.", 0);
		return 0;
	}
	SVM_Lock();
	if (whence == 0) {
		hn->cur_sct = ofst;
	} else if (whence == 2) {
		hn->cur_sct = hn->fsize_sct + ofst;
	} else if (whence == 1) {
		hn->cur_sct = hn->cur_sct + ofst;
	}
	hn->cur_sct = (hn->cur_sct < hn->fsize_sct) ? hn->cur_sct : hn->fsize_sct;
	hn->cur_sct = (hn->cur_sct > 0L) ? hn->cur_sct : 0L;
	SVM_Unlock();
	return hn->cur_sct;
}

// 0x8022E4BC
int mfCiTell(MFCI hn)
{
	if (hn == 0) {
		MFCI_ERR("E01100306:handl is null.", 0);
		return 0;
	}
	return hn->cur_sct;
}

// 0x8022E234
int mfCiReqRd(MFCI hn, int nsct, void* buf)
{
	unsigned long adr;
	int size;
	int len;
	int nbyte;
	int ofst;

	if (hn == 0) {
		MFCI_ERR("E01100307:handl is null.", 0);
		return 0;
	}
	if (nsct < 0) {
		MFCI_ERR("E01100308:nsct < 0.(mfCiReqRd)", hn);
		return 0;
	}
	if (buf == 0) {
		MFCI_ERR("E01100309:buf is null.(mfCiReqRd)", hn);
		return 0;
	}
	if (nsct == 0) {
		hn->stat = MFCI_STAT_COMPLETE;
		return 0;
	}
	SVM_Lock();
	hn->trns_size = 0;
	len           = hn->fsize_sct - hn->cur_sct;
	hn->rd_sct    = (nsct < len) ? nsct : len;
	ofst          = hn->cur_sct * hn->sct_len;
	nbyte         = hn->rd_sct * hn->sct_len;
	if (nbyte == 0) {
		hn->stat = MFCI_STAT_COMPLETE;
		SVM_Unlock();
		return 0;
	}
	hn->rd_ofst = ofst;
	hn->rd_size = nbyte;
	hn->stat    = MFCI_STAT_TRANS;
	mfci_get_adr_size(hn->fname, &adr, &size);
	len = size - hn->rd_ofst;
	len = (hn->rd_size > len) ? len : hn->rd_size;
	memcpy(buf, (void*)(adr + hn->rd_ofst), hn->rd_size);
	memset((char*)buf + len, 0, hn->rd_size - len);
	hn->trns_size = hn->rd_sct * hn->sct_len;
	hn->cur_sct   = hn->cur_sct + hn->rd_sct;
	hn->stat      = MFCI_STAT_COMPLETE;
	SVM_Unlock();
	return hn->rd_sct;
}

// 0x8022E1C4
void mfCiStopTr(MFCI hn)
{
	if (hn == 0) {
		MFCI_ERR("E0092912:handl is null.", 0);
		return;
	}
	SVM_Lock();
	hn->stat = MFCI_STAT_STOP;
	SVM_Unlock();
}

// 0x8022E160
int mfCiGetStat(MFCI hn)
{
	if (hn == 0) {
		MFCI_ERR("E0092912:handl is null.", 0);
		return 0;
	}
	return hn->stat;
}

// 0x8022E100
int mfCiGetSctLen(MFCI hn)
{
	if (hn == 0) {
		MFCI_ERR("E0040301:handl is null.", 0);
		return 0;
	}
	return hn->sct_len;
}

// 0x8022E068
void mfCiSetSctLen(MFCI hn, int len)
{
	int old;

	if (hn == 0) {
		MFCI_ERR("E0040302:handl is null.", 0);
		return;
	}
	old           = hn->cur_sct * hn->sct_len;
	hn->sct_len   = len;
	hn->fsize_sct = (hn->sct_len - 1 + hn->fsize) / hn->sct_len;
	hn->cur_sct   = old / hn->sct_len;
	hn->trns_size = hn->rd_sct * len;
}

// 0x8022E008
int mfCiGetNumTr(MFCI hn)
{
	if (hn == 0) {
		MFCI_ERR("E0092912:handl is null.", 0);
		return 0;
	}
	return hn->trns_size;
}

/* Opens entry flid of an AFS-style table of names */
MFCI mfCiOpenEntry(const char** names, int flid, int rw)
{
	MFCI hn;
	unsigned long adr;
	int i;

	if (flid < 0) {
		MFCI_ERR("E1041001:invalid entry number.(mfCiOpenEntry)", 0);
		return 0;
	}
	if (rw != 0) {
		MFCI_ERR("E1041002:rw is illigal.(mfCiOpenEntry)", 0);
		return 0;
	}
	hn = 0;
	for (i = 0; i < MFCI_MAX_OBJ; i++) {
		if (mfci_obj[i].used == 0) {
			hn = &mfci_obj[i];
			break;
		}
	}
	if (hn == 0) {
		MFCI_ERR("E1041002:not enough handle resource.(mfCiOpenEntry)", 0);
		return 0;
	}
	strcpy(hn->fname, names[flid]);
	hn->sct_len = MFCI_SCT_LEN;
	mfci_get_adr_size(hn->fname, &adr, &hn->fsize);
	hn->fsize_sct = (hn->sct_len + hn->fsize - 1) / hn->sct_len;
	hn->cur_sct   = 0;
	hn->rd_sct    = 0;
	hn->trns_size = 0;
	hn->stat      = MFCI_STAT_STOP;
	hn->used      = 1;
	return hn;
}
