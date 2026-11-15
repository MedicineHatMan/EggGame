#ifndef CRI_CRI_CVFS_H
#define CRI_CRI_CVFS_H

#define CVFS_MAX_DEV 32
#define CVFS_MAX_HNDL 40
#define CVFS_DEVNAME_LEN 12
#define CVFS_PATH_LEN 300

#define CVFS_SEEK_SET 0
#define CVFS_SEEK_CUR 1
#define CVFS_SEEK_END 2

typedef void (*CVFS_ERRFN)(void* obj, const char* msg, void* hn);

typedef struct {
	void (*ExecServer)(void);                             /* 00 */
	void (*EntryErrFunc)(CVFS_ERRFN func, void* obj);     /* 04 */
	int (*GetFileSize)(const char* fname);                /* 08 */
	void (*unk0C)(void);                                  /* 0C */
	void* (*Open)(const char* fname, void* dir, int rw);  /* 10 */
	void (*Close)(void* fd);                              /* 14 */
	int (*Seek)(void* fd, int ofst, int whence);          /* 18 */
	int (*Tell)(void* fd);                                /* 1C */
	int (*ReqRd)(void* fd, int nsct, void* buf);          /* 20 */
	int (*ReqWr)(void* fd, int nsct, void* buf);          /* 24 */
	void (*StopTr)(void* fd);                             /* 28 */
	int (*GetStat)(void* fd);                             /* 2C */
	int (*GetSctLen)(void* fd);                           /* 30 */
	void (*SetSctLen)(void* fd, int len);                 /* 34 */
	int (*GetNumTr)(void* fd);                            /* 38 */
	void (*unk3C[9])(void);                               /* 3C */
	int (*OptFn1)(void* fd, int id, int prm1, int prm2);  /* 60 */
	void (*unk64)(void);                                  /* 64 */
} CVFS_DEV;

/* Asking a device's OptFn1 this tells whether it wants the device name kept in front of the path */
#define CVFS_OPT_NEED_DEVNAME 100

typedef struct {
	CVFS_DEV* dev; /* 00 */
	void* fd;      /* 04 */
} CVFS_OBJ;

typedef CVFS_OBJ* CVFS;

typedef struct {
	CVFS_DEV* dev;                /* 00 */
	char name[CVFS_DEVNAME_LEN];  /* 04 */
} CVFS_DEVENT;

void cvFsEntryErrFunc(CVFS_ERRFN func, void* obj);
int cvFsGetStat(CVFS cvfs);
void cvFsExecServer(void);
void cvFsStopTr(CVFS cvfs);
int cvFsReqRd(CVFS cvfs, int nsct, void* buf);
int cvFsSeek(CVFS cvfs, int ofst, int whence);
int cvFsTell(CVFS cvfs);
void cvFsClose(CVFS cvfs);
CVFS cvFsOpen(const char* fname, void* dir, int rw);
void cvFsSetDefDev(char* name);
void cvFsAddDev(char* name, CVFS_DEV* (*getif)(void), void* prm);

CVFS_DEV* mfCiGetInterface(void);
CVFS_DEV* gcCiGetInterface(void);
void gcci_SetRdMode(int a, int b, int c, int rdmode);

#endif
