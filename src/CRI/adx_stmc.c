#include <CRI/adx_stmc.h>
#include <CRI/adx.h>
#include <CRI/svm.h>

typedef char adxstm_size_ok[(sizeof(ADXSTM_OBJ) == 0x5C) ? 1 : -1];

int adxstm_lock;
int adxstm_retry_limit;
int adxstm_read_err_cnt;
ADXSTM_OBJ adxstm_obj[ADXSTM_MAX];

void adxstmf_stat_exec(ADXSTM stm);
void adxstmf_stat_read(ADXSTM stm);

/* Floor division by a sector */
#define ADXSTM_SCT_FLOOR(n)   (((n) >> 11) + ((n) < 0 && ((n) & 0x7FF) != 0))
#define ADXSTM_SCT_ROUNDUP(n) (((n) + 0x7FF) / ADXSTM_SCT)
#define ADXSTM_SCT_CEIL(n)    ((n) / ADXSTM_SCT + ((n) % ADXSTM_SCT > 0 ? 1 : 0))

#define ADXSTM_EXEC()                                   \
	do {                                                \
		int _i;                                         \
		if (SVM_TestAndSet(&adxstm_lock) != 0) {        \
			for (_i = 0; _i < ADXSTM_MAX; _i++) {       \
				if (adxstm_obj[_i].used == 1) {         \
					adxstmf_stat_exec(&adxstm_obj[_i]); \
				}                                       \
			}                                           \
			adxstm_lock = 0;                            \
		}                                               \
	} while (0)

#define ADXSTM_REQ_STOP(stm)                            \
	do {                                                \
		SVM_Lock();                                     \
		if ((stm)->stat == 2 && (stm)->read_flg == 1) { \
			(stm)->stop_req = 1;                        \
			if ((stm)->start_req == 1) {                \
				(stm)->start_req = 0;                   \
			}                                           \
		} else {                                        \
			(stm)->stat = 1;                            \
		}                                               \
		SVM_Unlock();                                   \
	} while (0)

#define ADXSTM_REQ_CLOSE(stm)        \
	do {                             \
		SVM_Lock();                  \
		if ((stm)->cvfs_open == 1) { \
			(stm)->close_req = 1;    \
		}                            \
		(stm)->bind_req = 0;         \
		SVM_Unlock();                \
	} while (0)

#define ADXSTM_STOP_BODY(stm)                              \
	do {                                                   \
		if ((stm)->cvfs != 0) {                            \
			cvFsStopTr((stm)->cvfs);                       \
		}                                                  \
		(stm)->stat     = 1;                               \
		(stm)->read_flg = 0;                               \
		(stm)->ck.data  = 0;                               \
		ADXSTM_REQ_STOP(stm);                              \
		do {                                               \
			ADXSTM_ExecServer();                           \
		} while ((stm)->stat != 1 || (stm)->ck.data != 0); \
	} while (0)

// * Handles below this sector count come from the first pool the rest from the second
#define ADXSTM_POOL_SCT 0x100

/* First slot of the first pool */
int adxstm_pool_nsct;

/* Slot counts first pool size second pool start second pool size */
int adxstm_create_prm[4] = { 0x10, 0x10, 0x18, 0 };

static inline ADXSTM adxstm_CreateIn(SJ sj, int start, int num)
{
	ADXSTM stm;
	int i;
	int data;

	stm = 0;
	for (i = 0; i < num; i++) {
		stm = &adxstm_obj[start + i];
		if (stm->used == 0) {
			break;
		}
	}
	if (i == num) {
		return 0;
	}
	stm->stat     = 1;
	stm->read_flg = 0;
	stm->sj       = sj;
	stm->cvfs     = 0;
	stm->ofst     = 0;
	stm->file_len = 0;
	stm->req_rd   = 0x200;
	stm->cur_ofst = 0;
	stm->rd_limit = ADXSTM_RD_ALL;
	stm->eos      = stm->file_len / ADXSTM_SCT;
	if (stm->file_len % ADXSTM_SCT > 0) {
		stm->eos++;
	}
	if (stm->sj != 0) {
		ADXCRS_Lock();
		data          = SJ_GetNumData(sj, SJ_LIN_DATA);
		stm->sj_total = SJ_GetNumData(sj, SJ_LIN_FREE) + data;
		ADXCRS_Unlock();
		stm->buf_max    = stm->sj_total;
		stm->flow_limit = stm->sj_total;
	}
	stm->pause = 0;
	stm->used  = 1;
	return stm;
}

// 0x80223B60
int ADXSTM_Init(void)
{
	memset(adxstm_obj, 0, sizeof(ADXSTM_OBJ) * ADXSTM_MAX);
	return 1;
}

// 0x80223B5C
void ADXSTM_Finish(void)
{
}

// 0x802238A0
ADXSTM ADXSTM_Create(SJ sj, int maxsct)
{
	int* prm;

	prm = adxstm_create_prm;
	if (maxsct < ADXSTM_POOL_SCT) {
		return adxstm_CreateIn(sj, adxstm_pool_nsct, prm[0]);
	}
	return adxstm_CreateIn(sj, prm[1], prm[2]);
}

// 0x802235C4
void ADXSTM_Destroy(ADXSTM stm)
{
	if (stm != 0) {
		ADXSTM_STOP_BODY(stm);
		ADXSTM_STOP_BODY(stm);
		ADXSTM_REQ_STOP(stm);
		ADXSTM_REQ_CLOSE(stm);
		for (;;) {
			if (stm->cvfs_open == 0) {
				break;
			}
			ADXSTM_ExecServer();
		}
		stm->used = 0;
		memset(stm, 0, sizeof(ADXSTM_OBJ));
	}
}

// 0x80223568
void ADXSTM_BindFileNw(ADXSTM stm, const char* fname, void* dir, int ofst, int nsct)
{
	SVM_Lock();
	stm->ofst     = ofst;
	stm->file_len = nsct << 11;
	stm->fname    = fname;
	stm->dir      = dir;
	stm->bind_req = 1;
	SVM_Unlock();
}

// 0x802234AC
void ADXSTM_BindFile(ADXSTM stm, const char* fname, void* dir, int ofst, int nsct)
{
	SVM_Lock();
	stm->ofst     = ofst;
	stm->file_len = nsct << 11;
	stm->fname    = fname;
	stm->dir      = dir;
	stm->bind_req = 1;
	SVM_Unlock();
	do {
		ADXSTM_ExecServer();
	} while (stm->bind_req != 0);
}

// 0x80223418
void ADXSTM_ReleaseFileNw(ADXSTM stm)
{
	ADXSTM_REQ_STOP(stm);
	ADXSTM_REQ_CLOSE(stm);
}

// 0x80223230
void ADXSTM_ReleaseFile(ADXSTM stm)
{
	ADXSTM_STOP_BODY(stm);
	ADXSTM_REQ_STOP(stm);
	ADXSTM_REQ_CLOSE(stm);
	for (;;) {
		if (stm->cvfs_open == 0) {
			break;
		}
		ADXSTM_ExecServer();
	}
}

// 0x80223224
int ADXSTM_GetStat(ADXSTM stm)
{
	return stm->stat;
}

// 0x802231D4
int ADXSTM_Seek(ADXSTM stm, int ofst)
{
	stm->cur_ofst = ofst;
	if (stm->cur_ofst << 11 > stm->file_len) {
		stm->cur_ofst = ADXSTM_SCT_CEIL(stm->file_len);
	}
	return stm->cur_ofst;
}

// 0x802231B8
int ADXSTM_Tell(ADXSTM stm)
{
	if (stm->cvfs != 0) {
		return stm->cur_ofst;
	}
	return 0;
}

// 0x80223160
int ADXSTM_Start(ADXSTM stm)
{
	stm->rd_bytes = 0;
	stm->err_cnt  = 0;
	if (stm->file_len == 0) {
		stm->stat = 3;
	} else {
		stm->stat = 2;
	}
	stm->read_flg  = 0;
	stm->ck.data   = 0;
	stm->ck.len    = 0;
	stm->start_req = 1;
	stm->rd_limit  = ADXSTM_RD_ALL;
	return 1;
}

// 0x80223110
int ADXSTM_Start2(ADXSTM stm, unsigned int limit)
{
	stm->rd_bytes = 0;
	stm->err_cnt  = 0;
	if (stm->file_len == 0) {
		stm->stat = 3;
	} else {
		stm->stat = 2;
	}
	stm->read_flg  = 0;
	stm->ck.data   = 0;
	stm->ck.len    = 0;
	stm->start_req = 1;
	stm->rd_limit  = limit;
	return 1;
}

// 0x802230A0
void ADXSTM_StopNw(ADXSTM stm)
{
	ADXSTM_REQ_STOP(stm);
}

// 0x80222F88
void ADXSTM_Stop(ADXSTM stm)
{
	ADXSTM_STOP_BODY(stm);
}

// 0x80222F7C
void ADXSTM_EntryEosFunc(ADXSTM stm, void (*func)(void*), void* obj)
{
	stm->eos_func = func;
	stm->eos_obj  = obj;
}

// 0x80222F34
void ADXSTM_SetEos(ADXSTM stm, int eos)
{
	if (eos >= 0) {
		stm->eos = eos;
		return;
	}
	stm->eos = ADXSTM_SCT_CEIL(stm->file_len);
}

/* adxstmf_stat_exec calls this while a read is in progress */
// 0x80222B68
void adxstmf_stat_read(ADXSTM stm)
{
	int st;
	int nsct;
	int chunk_sct;
	int nbyte;
	SJCK data;
	SJCK rest;
	SJ sj;

	sj = stm->sj;
	st = cvFsGetStat(stm->cvfs);
	SVM_Lock();
	if (stm->read_flg == 1) {
		if (st == 1) {
			stm->read_flg = 0;
			SVM_Unlock();
			nbyte = stm->rd_sct * ADXSTM_SCT;
			SJ_SplitChunk(&stm->ck, nbyte, &data, &rest);
			SJ_PutChunk(sj, SJ_LIN_DATA, &data);
			SJ_UngetChunk(sj, SJ_LIN_FREE, &rest);
			stm->cur_ofst += stm->rd_sct;
			stm->rd_bytes += nbyte;
			stm->ck.data = 0;
			stm->ck.len  = 0;
			nbyte        = stm->file_len;
			nsct         = ADXSTM_SCT_CEIL(nbyte);
			if (stm->cur_ofst == stm->eos && stm->eos_func != 0) {
				stm->eos_func(stm->eos_obj);
			}
			if (stm->cur_ofst >= nsct) {
				stm->stat = 3;
			} else if (((unsigned int)stm->rd_bytes >> 11) >= stm->rd_limit && stm->rd_limit < ADXSTM_RD_ALL) {
				stm->stat = 3;
			}
			stm->err_cnt = 0;
		} else if (st == 3) {
			stm->read_flg = 0;
			SVM_Unlock();
			SJ_UngetChunk(sj, SJ_LIN_FREE, &stm->ck);
			stm->ck.data = 0;
			stm->ck.len  = 0;
			if (adxstm_retry_limit >= 0) {
				if (stm->err_cnt >= adxstm_retry_limit) {
					stm->stat = 4;
				} else {
					stm->err_cnt++;
				}
			}
		} else {
			SVM_Unlock();
		}
	} else {
		stm->read_flg = 1;
		stm->ck.data  = 0;
		stm->ck.len   = 0;
		SVM_Unlock();
		if (stm->pause == 1 || stm->stop_req == 1) {
			stm->read_flg = 0;
		} else if (stm->file_len == 0) {
			stm->read_flg = 0;
			stm->rd_sct   = 0;
			stm->stat     = 3;
		} else if (sj == 0 || sj->vtbl == 0) {
			stm->read_flg = 0;
			adxstm_read_err_cnt++;
		} else {
			nbyte = SJ_GetNumData(sj, SJ_LIN_FREE);
			if (stm->sj_total - nbyte >= stm->flow_limit) {
				stm->read_flg = 0;
			} else {
				SJ_GetChunk(sj, SJ_LIN_FREE, stm->buf_max, &stm->ck);
				chunk_sct = stm->ck.len / ADXSTM_SCT;
				nsct      = stm->eos - stm->cur_ofst;
				if (chunk_sct < nsct) {
					nsct = chunk_sct;
				}
				if (nsct < stm->req_rd) {
					chunk_sct = nsct;
				} else {
					chunk_sct = stm->req_rd;
				}
				cvFsSeek(stm->cvfs, stm->ofst + stm->cur_ofst, 0);
				stm->rd_sct = cvFsReqRd(stm->cvfs, chunk_sct, stm->ck.data);
				if (stm->rd_sct <= 0) {
					SJ_UngetChunk(sj, SJ_LIN_FREE, &stm->ck);
					stm->ck.data  = 0;
					stm->ck.len   = 0;
					stm->read_flg = 0;
					st            = cvFsGetStat(stm->cvfs);
					if (st == 3 && adxstm_retry_limit >= 0) {
						if (stm->err_cnt >= adxstm_retry_limit) {
							stm->stat = 4;
						} else {
							stm->err_cnt++;
						}
					}
				}
			}
		}
	}
}

// 0x80222950
void adxstmf_stat_exec(ADXSTM stm)
{
	int size;
	int len;

	if (stm->read_flg == 0) {
		if (stm->stop_req == 1) {
			stm->stop_req = 0;
			if (stm->start_req == 0) {
				stm->stat = 1;
			}
		}
		if (stm->close_req == 1) {
			if (stm->cvfs != 0) {
				cvFsClose(stm->cvfs);
				stm->cvfs = 0;
			}
			stm->close_req = 0;
			stm->cvfs_open = 0;
		}
		SVM_Lock();
		if (stm->bind_req == 1) {
			stm->cvfs_open = 1;
			SVM_Unlock();
			if (stm->cvfs == 0) {
				if ((stm->cvfs = cvFsOpen(stm->fname, stm->dir, 0)) == 0) {
					ADXERR_CallErrFunc2("E02110501 adxstmf_stat_exec: can't open ", stm->fname);
					stm->stat      = 4;
					stm->cvfs_open = 0;
					stm->bind_req  = 0;
					return;
				}
				cvFsSeek(stm->cvfs, 0, 2);
				size = cvFsTell(stm->cvfs);
				len  = size << 11;
				cvFsSeek(stm->cvfs, 0, 0);
				if (stm->file_len == (ADXSTM_RD_ALL << 11)) {
					stm->file_len = len;
				} else {
					if (stm->ofst > size) {
						stm->ofst = size;
					}
					if (stm->file_len / ADXSTM_SCT + stm->ofst > size) {
						stm->file_len = (size - stm->ofst) * ADXSTM_SCT;
					}
				}
				stm->cur_ofst = 0;
				if (stm->cur_ofst << 11 > stm->file_len) {
					stm->cur_ofst = ADXSTM_SCT_CEIL(stm->file_len);
				}
				stm->bind_req = 0;
			}
		} else {
			SVM_Unlock();
		}
		if (stm->start_req == 1) {
			stm->start_req = 0;
		}
	}
	if (stm->stat == 2 && stm->cvfs_open == 1) {
		adxstmf_stat_read(stm);
	}
}

// 0x802228D4
void ADXSTM_ExecServer(void)
{
	ADXSTM_EXEC();
}

// 0x802228C4
int ADXSTM_SetBufSize(ADXSTM stm, int min, int max)
{
	stm->flow_limit = min;
	stm->buf_max    = max;
	return 1;
}

// 0x802228B8
int ADXSTM_SetReqRdSize(ADXSTM stm, int size)
{
	stm->req_rd = size;
	return 1;
}

// 0x802228B0
int ADXSTM_GetFileLen(ADXSTM stm)
{
	return stm->file_len;
}

// 0x802228A8
void ADXSTM_SetPause(ADXSTM stm, int sw)
{
	stm->pause = sw;
}

// 0x80222818
void ADXSTM_SetSj(ADXSTM stm, SJ sj)
{
	int data;
	int free;

	stm->sj = sj;
	ADXCRS_Lock();
	data          = SJ_GetNumData(sj, SJ_LIN_DATA);
	free          = SJ_GetNumData(sj, SJ_LIN_FREE);
	stm->sj_total = free + data;
	ADXCRS_Unlock();
	free            = stm->sj_total;
	stm->buf_max    = free;
	stm->flow_limit = free;
}
