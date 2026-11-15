#include <CRI/cri_cvfs.h>
#include <CRI/adx.h>
#include <CRI/cri_std.h>

CVFS_OBJ cvfs_hn[CVFS_MAX_HNDL];
CVFS_DEVENT cvfs_devtbl[CVFS_MAX_DEV];
char cvfs_defdev[CVFS_DEVNAME_LEN];
char cvfs_tmp[CVFS_PATH_LEN];
int cvfs_unk;
void* cvfs_errobj;
CVFS_ERRFN cvfs_errfn;

/* Points at "CVFS/GC Ver.2.33 Build:Mar 3 2003 16:46:40" */
const char* const volatile cvfs_build = "\nCVFS/GC Ver.2.33 Build:Mar  3 2003 16:46:40\n";

static inline void cvfs_CallErr(const char* msg)
{
	if (cvfs_errfn != 0) {
		cvfs_errfn(cvfs_errobj, msg, 0);
	}
}

/* The same in the functions that reach the other variables from one base */
static inline void cvfs_CallErrW(const char* msg)
{
	if (cvfs_errfn != 0) {
		cvfs_errfn(cvfs_errobj, msg, 0);
	}
}

static inline void cvfs_ToUpper(char* s)
{
	unsigned int n;
	unsigned int i;
	char* p;

	n = strlen(s) + 1;
	p = s;
	for (i = 0; i < n; i++, p++) {
		if (*p >= 'a' && *p <= 'z') {
			*p -= 0x20;
		}
	}
}

static inline CVFS_DEV* cvfs_FindDev(const char* name)
{
	int len;
	unsigned int i;

	len = strlen(name);
	for (i = 0; i < CVFS_MAX_DEV; i++) {
		if (strncmp(name, cvfs_devtbl[i].name, len) == 0) {
			return cvfs_devtbl[i].dev;
		}
	}
	return 0;
}

static inline void cvfs_GetDefDev(char* name)
{
	int len;

	len = strlen(cvfs_defdev);
	if (cvfs_defdev[0] == 0) {
		name[0] = 0;
	} else {
		memcpy(name, cvfs_defdev, len + 1);
	}
}

/* Splits "DEV:path" into the device upper case and the path */
static inline void cvfs_SplitName(const char* fname, char* devname, char* path)
{
	int i;
	int j;

	if (fname == 0) {
		return;
	}
	for (i = 0; i < CVFS_PATH_LEN - 3; i++) {
		if (fname[i] == ':' || fname[i] == 0) {
			break;
		}
		devname[i] = fname[i];
	}
	if (fname[i] == 0) {
		devname[i] = 0;
		memcpy(path, devname, strlen(devname) + 1);
		devname[0] = 0;
		return;
	}
	devname[i] = 0;
	i++;
	if (i == 2) {
		i = 0;
		devname[0] = 0;
	}
	for (j = i; j < CVFS_PATH_LEN - 3 && fname[j] != 0; j++) {
		path[j - i] = fname[j];
	}
	path[j - i] = 0;
	cvfs_ToUpper(devname);
}

static inline CVFS cvfs_AllocHn(void)
{
	int i;

	for (i = 0; i < CVFS_MAX_HNDL; i++) {
		if (cvfs_hn[i].fd == 0) {
			break;
		}
	}
	if (i == CVFS_MAX_HNDL) {
		return 0;
	}
	return &cvfs_hn[i];
}

static inline int cvfs_IsNeedDevName(const char* name)
{
	CVFS_DEV* dev;

	if (name == 0) {
		name = cvfs_defdev;
	}
	dev = cvfs_FindDev(name);
	if (dev == 0) {
		return 0;
	}
	if (dev->OptFn1 == 0) {
		return 0;
	}
	return dev->OptFn1(0, CVFS_OPT_NEED_DEVNAME, 0, 0);
}

static void cvfs_AddDevName(const char* devname, char* path);

/* What every device's errors go through */
// 0x8022A034
void cvfs_err_func(void* obj, const char* msg, void* hn)
{
	if (cvfs_errfn != 0) {
		cvfs_errfn(cvfs_errobj, msg, hn);
	}
}

// 0x80229DB8
static inline int cvfs_GetFreeDev(void)
{
	int i;

	for (i = 0; i < CVFS_MAX_DEV; i++) {
		if (cvfs_devtbl[i].name[0] == 0) {
			break;
		}
	}
	return i;
}

static inline CVFS_DEV* cvfs_AddDevEnt(char* name, CVFS_DEV* dev)
{
	int i;

	if (cvfs_FindDev(name) != 0) {
		return dev;
	}
	i = cvfs_GetFreeDev();
	if (i == CVFS_MAX_DEV) {
		return 0;
	}
	cvfs_devtbl[i].dev = dev;
	memcpy(cvfs_devtbl[i].name, name, strlen(name) + 1);
	return dev;
}

void cvFsAddDev(char* name, CVFS_DEV* (*getif)(void), void* prm)
{
	CVFS_DEV* dev;

	cvfs_build;
	if (name == 0) {
		cvfs_CallErrW("cvFsAddDev #1:illegal device name");
		return;
	}
	if (getif == 0) {
		cvfs_CallErrW("cvFsAddDev #2:illegal I/F func name");
		return;
	}
	cvfs_ToUpper(name);
	dev = cvfs_AddDevEnt(name, getif());
	if (dev == 0) {
		cvfs_CallErrW("cvFsAddDev #3:failed added a device");
		return;
	}
	if (dev->EntryErrFunc != 0) {
		dev->EntryErrFunc(cvfs_err_func, 0);
	}
}

/* Stripped from the DOL */
void cvFsDelDev(char* name)
{
	int i;

	if (name == 0) {
		cvfs_CallErr("cvFsDelDev #1:illegal device name");
		return;
	}
	cvfs_ToUpper(name);
	for (i = 0; i < CVFS_MAX_DEV; i++) {
		if (strncmp(name, cvfs_devtbl[i].name, CVFS_DEVNAME_LEN) == 0) {
			memset(&cvfs_devtbl[i], 0, sizeof(CVFS_DEVENT));
		}
	}
}

static inline int cvfs_ExistDev(const char* name, int len)
{
	CVFS_DEVENT* ent;
	int i;

	for (i = 0, ent = cvfs_devtbl; i < CVFS_MAX_DEV; i++, ent++) {
		if (strncmp(name, ent->name, len) == 0) {
			return 1;
		}
	}
	return 0;
}

// 0x80229C84
void cvFsSetDefDev(char* name)
{
	int len;

	if (name == 0) {
		if (cvfs_errfn != 0) {
			cvfs_errfn(cvfs_errobj, "cvFsSetDefDev #1:illegal device name", 0);
		}
		return;
	}
	len = strlen(name);
	if (len == 0) {
		cvfs_defdev[0] = 0;
		return;
	}
	cvfs_ToUpper(name);
	if (cvfs_ExistDev(name, len) == 1) {
		memcpy(cvfs_defdev, name, len + 1);
	} else {
		if (cvfs_errfn != 0) {
			cvfs_errfn(cvfs_errobj, "cvFsSetDefDev #2:unknown device name", 0);
		}
	}
}

// 0x80229620
CVFS cvFsOpen(const char* fname, void* dir, int rw)
{
	char path[CVFS_PATH_LEN];
	char devname[CVFS_PATH_LEN];
	CVFS hn;
	CVFS_DEV* dev;
	char* name;

	if (fname == 0) {
		cvfs_CallErrW("cvFsOpen #1:illegal file name");
		return 0;
	}
	cvfs_SplitName(fname, devname, path);
	if (path[0] == 0) {
		cvfs_CallErrW("cvFsOpen #1:illegal file name");
		return 0;
	}
	hn = cvfs_AllocHn();
	if (hn == 0) {
		cvfs_CallErrW("cvFsOpen #3:failed handle alloced");
		return 0;
	}
	name = devname;
	if (devname[0] == 0) {
		cvfs_GetDefDev(name);
		if (devname[0] == 0) {
			dev = 0;
			goto found;
		}
	}
	cvfs_AddDevName(name, path);
	dev = cvfs_FindDev(name);
	if (dev == 0) {
		cvfs_GetDefDev(name);
		dev = cvfs_FindDev(name);
		if (dev == 0) {
			dev = 0;
		} else {
			strcpy(path, fname);
		}
	}
found:
	hn->dev = dev;
	if (devname == 0) {
		hn->fd = 0;
		hn->dev = 0;
		cvfs_CallErrW("cvFsOpen #2:illegal device name");
		return 0;
	}
	if (hn->dev == 0) {
		hn->fd = 0;
		hn->dev = 0;
		cvfs_CallErrW("cvFsOpen #4:device not found");
		return 0;
	}
	if (hn->dev->Open == 0) {
		hn->fd = 0;
		hn->dev = 0;
		cvfs_CallErrW("cvFsOpen #5:vtbl error");
		return 0;
	}
	hn->fd = hn->dev->Open(path, dir, rw);
	if (hn->fd == 0) {
		hn->fd = 0;
		hn->dev = 0;
		cvfs_CallErrW("cvFsOpen #6:open failed");
		return 0;
	}
	return hn;
}

// 0x80229564
void cvFsClose(CVFS cvfs)
{
	if (cvfs == 0) {
		cvfs_CallErr("cvFsClose #1:handle error");
		return;
	}
	if (cvfs->dev->Close != 0) {
		cvfs->dev->Close(cvfs->fd);
		cvfs->fd = 0;
		cvfs->dev = 0;
	} else {
		cvfs_CallErr("cvFsClose #2:vtbl error");
	}
}

// 0x802294A4
int cvFsTell(CVFS cvfs)
{
	int pos;

	if (cvfs == 0) {
		cvfs_CallErr("cvFsTell #1:handle error");
		return 0;
	}
	if (cvfs->dev->Tell != 0) {
		pos = cvfs->dev->Tell(cvfs->fd);
	} else {
		pos = 0;
		cvfs_CallErr("cvFsTell #2:vtbl error");
	}
	return pos;
}

// 0x802293E4
int cvFsSeek(CVFS cvfs, int ofst, int whence)
{
	int pos;

	if (cvfs == 0) {
		cvfs_CallErr("cvFsSeek #1:handle error");
		return 0;
	}
	if (cvfs->dev->Seek != 0) {
		pos = cvfs->dev->Seek(cvfs->fd, ofst, whence);
	} else {
		pos = 0;
		cvfs_CallErr("cvFsSeek #2:vtbl error");
	}
	return pos;
}

// 0x80229324
int cvFsReqRd(CVFS cvfs, int nsct, void* buf)
{
	int ret;

	if (cvfs == 0) {
		cvfs_CallErr("cvFsReqRd #1:handle error");
		return 0;
	}
	if (cvfs->dev->ReqRd != 0) {
		ret = cvfs->dev->ReqRd(cvfs->fd, nsct, buf);
	} else {
		ret = 0;
		cvfs_CallErr("cvFsReqRd #2:vtbl error");
	}
	return ret;
}

/* Stripped from the DOL */
int cvFsReqWr(CVFS cvfs, int nsct, void* buf)
{
	int ret;

	if (cvfs == 0) {
		cvfs_CallErr("cvFsReqWr #1:handle error");
		return 0;
	}
	if (cvfs->dev->ReqWr != 0) {
		ret = cvfs->dev->ReqWr(cvfs->fd, nsct, buf);
	} else {
		ret = 0;
		cvfs_CallErr("cvFsReqWr #2:vtbl error");
	}
	return ret;
}

// 0x8022927C
void cvFsStopTr(CVFS cvfs)
{
	if (cvfs == 0) {
		cvfs_CallErr("cvFsStopTr #1:handle error");
		return;
	}
	if (cvfs->dev->StopTr != 0) {
		cvfs->dev->StopTr(cvfs->fd);
	} else {
		cvfs_CallErr("cvFsStopTr #2:vtbl error");
	}
}

// 0x80229214
void cvFsExecServer(void)
{
	int i;

	for (i = 0; i < CVFS_MAX_DEV; i++) {
		if (cvfs_devtbl[i].dev != 0 && cvfs_devtbl[i].dev->ExecServer != 0) {
			cvfs_devtbl[i].dev->ExecServer();
		}
	}
}

// 0x80229154
int cvFsGetStat(CVFS cvfs)
{
	int stat;

	stat = 3;
	if (cvfs == 0) {
		cvfs_CallErr("cvFsGetStat #1:handle error");
		return 3;
	}
	if (cvfs->dev->GetStat != 0) {
		stat = cvfs->dev->GetStat(cvfs->fd);
	} else {
		cvfs_CallErr("cvFsGetStat #2:vtbl error");
	}
	return stat;
}

#define CVFS_SLOT(dev, n, type) ((type)(dev)->unk3C[(n)])

/* From here to cvFsEntryErrFunc every function was stripped from the DOL only their strings are left in the pool
 * which is why they are written back */
int cvFsGetFileSize(const char* fname);
int cvFsGetFileSize(const char* fname)
{
	char devname[CVFS_PATH_LEN];
	char path[CVFS_PATH_LEN];
	CVFS_DEV* dev;

	if (fname == 0) {
		cvfs_CallErr("cvFsGetFileSize #1:illegal file name");
		return 0;
	}
	cvfs_SplitName(fname, devname, path);
	if (devname[0] == 0) {
		cvfs_GetDefDev(devname);
	}
	if (devname[0] == 0) {
		cvfs_CallErr("cvFsGetFileSize #2:illegal device name");
		return 0;
	}
	dev = cvfs_FindDev(devname);
	if (dev == 0) {
		cvfs_CallErr("cvFsGetFileSize #3:device not found");
		return 0;
	}
	if (dev->GetFileSize == 0) {
		cvfs_CallErr("cvFsGetFileSize #4:vtbl error");
		return 0;
	}
	cvfs_AddDevName(devname, path);
	return dev->GetFileSize(path);
}

int cvFsGetFileSizeEx(const char* fname, int arg)
{
	char devname[CVFS_PATH_LEN];
	char path[CVFS_PATH_LEN];
	CVFS_DEV* dev;

	if (fname == 0) {
		cvfs_CallErr("cvFsGetFileSizeEx #1:illegal file name");
		return 0;
	}
	cvfs_SplitName(fname, devname, path);
	if (devname[0] == 0) {
		cvfs_GetDefDev(devname);
	}
	if (devname[0] == 0) {
		cvfs_CallErr("cvFsGetFileSizeEx #2:illegal device name");
		return 0;
	}
	dev = cvfs_FindDev(devname);
	if (dev == 0) {
		cvfs_CallErr("cvFsGetFileSizeEx #3:device not found");
		return 0;
	}
	if (dev->unk0C == 0) {
		cvfs_CallErr("cvFsGetFileSizeEx #4:vtbl error");
		return 0;
	}
	cvfs_AddDevName(devname, path);
	return ((int (*)(const char*, int))dev->unk0C)(path, arg);
}

int cvFsGetFileSizeByHndl(CVFS cvfs)
{
	if (cvfs == 0) {
		cvfs_CallErr("cvFsGetFileSizeByHndl #1:illegal file handle");
		return 0;
	}
	return CVFS_SLOT(cvfs->dev, 0, int (*)(void*))(cvfs->fd);
}

int cvFsGetFreeSize(const char* devname)
{
	CVFS_DEV* dev;

	dev = cvfs_FindDev(devname);
	if (dev == 0) {
		cvfs_CallErr("cvFsGetFreeSize #5:device not found");
		return 0;
	}
	if (dev->unk3C[1] == 0) {
		cvfs_CallErr("cvFsGetFreeSize #6:vtbl error");
		return 0;
	}
	return CVFS_SLOT(dev, 1, int (*)(void))();
}

int cvFsGetSctLen(CVFS cvfs)
{
	if (cvfs == 0) {
		cvfs_CallErr("cvFsGetSctLen #1:handle error");
		return 0;
	}
	if (cvfs->dev->GetSctLen == 0) {
		cvfs_CallErr("cvFsGetSctLen #2:vtbl error");
		return 0;
	}
	return cvfs->dev->GetSctLen(cvfs->fd);
}

void cvFsSetSctLen(CVFS cvfs, int len)
{
	if (cvfs == 0) {
		cvfs_CallErr("cvFsSetSctLen #3:handle error");
		return;
	}
	if (cvfs->dev->SetSctLen == 0) {
		cvfs_CallErr("cvFsSetSctLen #4:vtbl error");
		return;
	}
	cvfs->dev->SetSctLen(cvfs->fd, len);
}

int cvFsGetNumTr(CVFS cvfs)
{
	if (cvfs == 0) {
		cvfs_CallErr("cvFsGetNumTr #1:handle error");
		return 0;
	}
	if (cvfs->dev->GetNumTr == 0) {
		cvfs_CallErr("cvFsGetNumTr #2:vtbl error");
		return 0;
	}
	return cvfs->dev->GetNumTr(cvfs->fd);
}

int cvFsChangeDir(const char* fname)
{
	char devname[CVFS_PATH_LEN];
	char path[CVFS_PATH_LEN];
	CVFS_DEV* dev;

	if (fname == 0) {
		cvfs_CallErr("cvFsChangeDir #1:illegal directory name");
		return 0;
	}
	cvfs_SplitName(fname, devname, path);
	if (devname[0] == 0) {
		cvfs_GetDefDev(devname);
	}
	if (devname[0] == 0) {
		cvfs_CallErr("cvFsChangeDir #2:illegal device name");
		return 0;
	}
	dev = cvfs_FindDev(devname);
	if (dev == 0) {
		cvfs_CallErr("cvFsChangeDir #3:device not found");
		return 0;
	}
	if (dev->unk3C[2] == 0) {
		cvfs_CallErr("cvFsChangeDir #4:vtbl error");
		return 0;
	}
	cvfs_AddDevName(devname, path);
	return CVFS_SLOT(dev, 2, int (*)(const char*))(path);
}

int cvFsIsExistFile(const char* fname)
{
	char devname[CVFS_PATH_LEN];
	char path[CVFS_PATH_LEN];
	CVFS_DEV* dev;

	cvfs_SplitName(fname, devname, path);
	if (devname[0] == 0) {
		cvfs_GetDefDev(devname);
	}
	if (devname[0] == 0) {
		cvfs_CallErr("cvFsIsExistFile #2:illegal device name");
		return 0;
	}
	if (fname == 0) {
		cvfs_CallErr("cvFsIsExistFile #1:illegal file name");
		return 0;
	}
	dev = cvfs_FindDev(devname);
	if (dev == 0) {
		cvfs_CallErr("cvFsIsExistFile #3:device not found");
		return 0;
	}
	if (dev->unk3C[3] == 0) {
		cvfs_CallErr("cvFsIsExistFile #4:vtbl error");
		return 0;
	}
	cvfs_AddDevName(devname, path);
	return CVFS_SLOT(dev, 3, int (*)(const char*))(path);
}

int cvFsGetMaxByteRate(CVFS cvfs)
{
	if (cvfs == 0) {
		cvfs_CallErr("cvFsGetMaxByteRate #1:handle error");
		return 0;
	}
	if (cvfs->dev->unk3C[4] == 0) {
		cvfs_CallErr("cvFsGetMaxByteRate #2:vtbl error");
		return 0;
	}
	return CVFS_SLOT(cvfs->dev, 4, int (*)(void*))(cvfs->fd);
}

int cvFsMakeDir(const char* fname)
{
	char devname[CVFS_PATH_LEN];
	char path[CVFS_PATH_LEN];
	CVFS_DEV* dev;

	if (fname == 0) {
		cvfs_CallErr("cvFsMakeDir #1:illegal directory name");
		return 0;
	}
	cvfs_SplitName(fname, devname, path);
	if (devname[0] == 0) {
		cvfs_GetDefDev(devname);
	}
	if (devname[0] == 0) {
		cvfs_CallErr("cvFsMakeDir #2:illegal device name");
		return 0;
	}
	dev = cvfs_FindDev(devname);
	if (dev == 0) {
		cvfs_CallErr("cvFsMakeDir #3:device not found");
		return 0;
	}
	if (dev->unk3C[5] == 0) {
		cvfs_CallErr("cvFsMakeDir #4:vtbl error");
		return 0;
	}
	cvfs_AddDevName(devname, path);
	return CVFS_SLOT(dev, 5, int (*)(const char*))(path);
}

int cvFsRemoveDir(const char* fname)
{
	char devname[CVFS_PATH_LEN];
	char path[CVFS_PATH_LEN];
	CVFS_DEV* dev;

	if (fname == 0) {
		cvfs_CallErr("cvFsRemoveDir #1:illegal directory name");
		return 0;
	}
	cvfs_SplitName(fname, devname, path);
	if (devname[0] == 0) {
		cvfs_GetDefDev(devname);
	}
	if (devname[0] == 0) {
		cvfs_CallErr("cvFsRemoveDir #2:illegal device name");
		return 0;
	}
	dev = cvfs_FindDev(devname);
	if (dev == 0) {
		cvfs_CallErr("cvFsRemoveDir #3:device not found");
		return 0;
	}
	if (dev->unk3C[6] == 0) {
		cvfs_CallErr("cvFsRemoveDir #4:vtbl error");
		return 0;
	}
	cvfs_AddDevName(devname, path);
	return CVFS_SLOT(dev, 6, int (*)(const char*))(path);
}

int cvFsDeleteFile(const char* fname)
{
	char devname[CVFS_PATH_LEN];
	char path[CVFS_PATH_LEN];
	CVFS_DEV* dev;

	if (fname == 0) {
		cvfs_CallErr("cvFsDeleteFile #1:illegal file name");
		return 0;
	}
	cvfs_SplitName(fname, devname, path);
	if (devname[0] == 0) {
		cvfs_GetDefDev(devname);
	}
	if (devname[0] == 0) {
		cvfs_CallErr("cvFsDeleteFile #2:illegal device name");
		return 0;
	}
	dev = cvfs_FindDev(devname);
	if (dev == 0) {
		cvfs_CallErr("cvFsDeleteFile #3:device not found");
		return 0;
	}
	if (dev->unk3C[7] == 0) {
		cvfs_CallErr("cvFsDeleteFile #4:vtbl error");
		return 0;
	}
	cvfs_AddDevName(devname, path);
	return CVFS_SLOT(dev, 7, int (*)(const char*))(path);
}

const char* cvFsGetDevName(CVFS cvfs)
{
	if (cvfs->dev->unk3C[8] == 0) {
		cvfs_CallErr("cvFsGetDevName #1:vtbl error");
		return 0;
	}
	return CVFS_SLOT(cvfs->dev, 8, const char* (*)(void*))(cvfs->fd);
}

int cvFsOptFn1(CVFS cvfs, int id, int prm1, int prm2)
{
	if (cvfs == 0) {
		cvfs_CallErr("cvFsOptFn1 #1:handle error");
		return 0;
	}
	if (cvfs->dev->OptFn1 == 0) {
		cvfs_CallErr("cvFsOptFn1 #2:vtbl error");
		return 0;
	}
	return cvfs->dev->OptFn1(cvfs->fd, id, prm1, prm2);
}

int cvFsOptFn2(CVFS cvfs, int id, int prm1, int prm2)
{
	if (cvfs == 0) {
		cvfs_CallErr("cvFsOptFn2 #1:handle error");
		return 0;
	}
	if (cvfs->dev->unk64 == 0) {
		cvfs_CallErr("cvFsOptFn2 #2:vtbl error");
		return 0;
	}
	return ((int (*)(void*, int, int, int))cvfs->dev->unk64)(cvfs->fd, id, prm1, prm2);
}

int cvFsSetCurVolume(const char* devname, void* img)
{
	CVFS_DEV* dev;

	if (devname == 0) {
		cvfs_CallErr("cvFsSetCurVolume #1:illegal device name");
		return 0;
	}
	if (img == 0) {
		cvfs_CallErr("cvFsSetCurVolume #2:illegal image handle");
		return 0;
	}
	if ((dev = cvfs_FindDev(devname)) == 0) {
		cvfs_CallErr("cvFsSetCurVolume #3:device not found");
		return 0;
	}
	return 1;
}

int cvFsAddVolumeEx(const char* devname, const char* volname, void* img, int flag)
{
	CVFS_DEV* dev;

	if (devname == 0) {
		cvfs_CallErr("cvFsAddVolumeEx #1:illegal device name");
		return 0;
	}
	if (volname == 0) {
		cvfs_CallErr("cvFsAddVolumeEx #2:illegal volume name");
		return 0;
	}
	if (img == 0) {
		cvfs_CallErr("cvFsAddVolumeEx #3:illegal image handle");
		return 0;
	}
	if ((dev = cvfs_FindDev(devname)) == 0) {
		cvfs_CallErr("cvFsAddVolumeEx #3:device not found");
		return 0;
	}
	return 1;
}

int cvFsDelVolume(const char* devname, const char* volname)
{
	CVFS_DEV* dev;

	if (devname == 0) {
		cvfs_CallErr("cvFsDelVolume #1:illegal device name");
		return 0;
	}
	if (volname == 0) {
		cvfs_CallErr("cvFsDelVolume #2:illegal volume name");
		return 0;
	}
	if ((dev = cvfs_FindDev(devname)) == 0) {
		cvfs_CallErr("cvFsDelVolume #3:device not found");
		return 0;
	}
	return 1;
}

int cvFsGetVolumeInfo(const char* devname, const char* volname, void* info)
{
	CVFS_DEV* dev;

	if (devname == 0) {
		cvfs_CallErr("cvFsGetVolumeInfo #1:illegal device name");
		return 0;
	}
	if (volname == 0) {
		cvfs_CallErr("cvFsGetVolumeInfo #2:illegal volume name");
		return 0;
	}
	if ((dev = cvfs_FindDev(devname)) == 0) {
		cvfs_CallErr("cvFsGetVolumeInfo #3:device not found");
		return 0;
	}
	return 1;
}

int cvFsSetDefVol(const char* devname, const char* volname)
{
	CVFS_DEV* dev;

	if (devname == 0) {
		cvfs_CallErr("cvFsSetDefVol #1:illegal device name");
		return 0;
	}
	if (volname == 0) {
		cvfs_CallErr("cvFsSetDefVol #2:illegal volume name");
		return 0;
	}
	if ((dev = cvfs_FindDev(devname)) == 0) {
		cvfs_CallErr("cvFsSetDefVol #3:device not found");
		return 0;
	}
	return 1;
}

// 0x80229120
void cvFsEntryErrFunc(CVFS_ERRFN func, void* obj)
{
	if (func == 0) {
		cvfs_errfn = 0;
		cvfs_errobj = 0;
	} else {
		cvfs_errfn = func;
		cvfs_errobj = obj;
	}
}

static void cvfs_AddDevName(const char* devname, char* path)
{
	if (cvfs_IsNeedDevName(devname) == 1) {
		strcpy(cvfs_tmp, path);
		sprintf(path, "%s:%s", devname, cvfs_tmp);
	}
}
