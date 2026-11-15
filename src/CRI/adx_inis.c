#include <CRI/adx.h>
#include <CRI/adx_fs.h>
#include <CRI/adx_stmc.h>
#include <CRI/adx_tlk.h>
#include <CRI/adxm.h>
#include <CRI/lsc.h>
#include <CRI/sj.h>
#include <CRI/svm.h>

ADX_TALK adxt_obj[ADXT_MAX_OBJ];

static const char adxt_build[]     = "\nADXT/GC Ver.8.66 Build:Mar  3 2003 16:47:46\n";
const char* const adxt_ver_info[2] = { adxt_build, 0 };
int adxini_chk_thrd_fg             = 1; /* 1 when the file server runs in its own callback */

unsigned int adxt_svcnt;
int adxt_fs_svr_id;
int adxt_main_svr_id;
int adxt_init_pad;
int adxt_init_cnt;

// 0x80219EC0
static void adxini_fserr_cbfn(void* obj, char* msg)
{
	ADXERR_CallErrFunc1(msg);
}

// 0x80219E9C
static void adxini_lscerr_cbfn(void* obj, char* msg)
{
	ADXERR_CallErrFunc1(msg);
}

// 0x80219E78
static int adxini_lscsvr_cbfn(void* obj)
{
	LSC_ExecServer();
	return 0;
}

// 0x80219E4C
static int adxini_allsvr_cbfn(void* obj)
{
	ADXT_ExecServer();
	ADXT_ExecFsSvr();
	LSC_ExecServer();
	return 0;
}

// 0x80219E28
static int adxini_vsync_cbfn(void* obj)
{
	ADXT_ExecServer();
	return 0;
}

// 0x80219E04
static int adxini_fssvr_cbfn(void* obj)
{
	ADXT_ExecFsSvr();
	return 0;
}

// 0x80219CC8
void ADXT_Init(void)
{
	*(const char* volatile*)adxt_ver_info;

	if (adxt_init_cnt == 0) {
		ADXCRS_Init();
		ADXCRS_Lock();
		SJUNI_Init();
		SJRBF_Init();
		SJMEM_Init();
		ADXERR_Init();
		ADXSTM_Init();
		ADXSJD_Init();
		ADXF_Init();
		ADXRNA_Init();
		LSC_Init();
		SVM_Init();
		ADXRNA_EntryErrFunc(adxini_fserr_cbfn, 0);
		LSC_EntryErrFunc(adxini_lscerr_cbfn, 0);
		memset(adxt_obj, 0, sizeof(adxt_obj));
		if (ADXM_IsSetupThrd() == 1 && adxini_chk_thrd_fg == 1) {
			SVM_SetCbSvrId(2, 1, adxini_vsync_cbfn, 0);
			adxt_fs_svr_id   = SVM_SetCbSvr(4, adxini_fssvr_cbfn, 0);
			adxt_main_svr_id = SVM_SetCbSvr(5, adxini_lscsvr_cbfn, 0);
		} else {
			adxt_main_svr_id = SVM_SetCbSvr(5, adxini_allsvr_cbfn, 0);
		}
		adxt_svcnt = 0;
		ADXCRS_Unlock();
	}
	adxt_init_cnt++;
}

// 0x80219C38
void ADXT_Finish(void)
{
	if (--adxt_init_cnt == 0) {
		ADXRNA_Finish();
		ADXF_Finish();
		ADXSTM_Finish();
		LSC_Finish();
		ADXCRS_Lock();
		SVM_DelCbSvr(2, 1);
		SVM_DelCbSvr(4, adxt_fs_svr_id);
		SVM_DelCbSvr(5, adxt_main_svr_id);
		SVM_Finish();
		ADXSJD_Finish();
		ADXERR_Finish();
		SJMEM_Finish();
		SJRBF_Finish();
		SJUNI_Finish();
		ADXCRS_Unlock();
	}
}
