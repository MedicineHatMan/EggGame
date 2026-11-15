#ifndef _CRI_SVM_H
#define _CRI_SVM_H

typedef int (*SVM_SVRFN)(void*);

void SVM_Init(void);
void SVM_CallErr1(const char* msg);
void SVM_SetCbBdr(int id, SVM_SVRFN fn, void* obj);
void SVM_SetCbLock(void (*fn)(void*), void* obj);
void SVM_SetCbUnlock(void (*fn)(void*), void* obj);
int SVM_ExecSvrVsync(void);
int SVM_ExecSvrFs(void);
int SVM_ExecSvrMain(void);
void SVM_Finish(void);
int SVM_SetCbSvr(int type, SVM_SVRFN fn, void* obj);
void SVM_DelCbSvr(int type, int id);
void SVM_SetCbSvrId(int type, int id, SVM_SVRFN fn, void* obj);
int SVM_TestAndSet(int* mem);

#endif
