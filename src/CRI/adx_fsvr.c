#include <CRI/adx.h>
#include <CRI/adx_fs.h>
#include <CRI/adx_stmc.h>
#include <CRI/cri_cvfs.h>

int adxt_exec_fssvr_flg;

// 0x8021FDEC
void ADXT_ExecFsSvr(void)
{
	ADXCRS_Lock();
	if (adxt_exec_fssvr_flg != 0) {
		ADXCRS_Unlock();
		return;
	}
	adxt_exec_fssvr_flg = 1;
	ADXCRS_Unlock();
	ADXCRS_Lock();
	ADXSTM_ExecServer();
	adxt_exec_fssvr_flg = 2;
	cvFsExecServer();
	adxt_exec_fssvr_flg = 5;
	ADXSTM_ExecServer();
	adxt_exec_fssvr_flg = 6;
	ADXF_ExecServer();
	adxt_exec_fssvr_flg = 0;
	ADXCRS_Unlock();
}
