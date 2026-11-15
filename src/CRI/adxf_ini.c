#include <Dolphin/os.h>
#include <CRI/adx_fs.h>
#include <CRI/adx.h>

#pragma auto_inline off

const char* const volatile adxf_build = "\nADXF/GC Ver.7.04 Build:Mar  3 2003 16:46:49\n";

/* Reversed CRI emits uninitialised data in reverse */
unsigned char adxf_tmpbuf_area[0x2080]; /* The DOL's size adx_fs.c loads one ADXF_SCT into it 32-aligned */
int adxf_tbsize_sct;
unsigned char* adxf_tmpbuf;
ADX_FS adxf_obj[ADXF_MAX_OBJ];
ADXF_PTINFO* adxf_ptinfo[ADXF_MAX_PTID];
int adxf_ocbi_fg;
ADXF_CMD_HSTRY adxf_cmd_hstry[ADXF_HSTRY_NUM];
unsigned short adxf_cmd_ncall[ADXF_CMD_NUM];
int adxf_hstry_no;
int adxf_ldptnw_nfile;
ADXF adxf_afs_hndl;
int adxf_afs_ptid;
int adxf_init_cnt;

// 0x8022782C
void ADXF_Init(void)
{
	adxf_build;
	if (adxf_init_cnt == 0) {
		memset(adxf_obj, 0, sizeof(adxf_obj));
		memset(adxf_ptinfo, 0, sizeof(adxf_ptinfo));
		memset(adxf_cmd_hstry, 0xFF, sizeof(adxf_cmd_hstry));
		memset(adxf_cmd_ncall, 0, sizeof(adxf_cmd_ncall));
		adxf_hstry_no     = 0;
		adxf_ocbi_fg      = 0;
		adxf_ldptnw_nfile = 0;
		adxf_afs_hndl     = 0;
		adxf_afs_ptid     = -1;
	}
	adxf_init_cnt++;
}

// 0x80227790
void ADXF_Finish(void)
{
	if (--adxf_init_cnt == 0) {
		ADXF_CloseAll();
		adxf_afs_ptid     = -1;
		adxf_afs_hndl     = 0;
		adxf_ldptnw_nfile = 0;
		adxf_ocbi_fg      = 0;
		adxf_hstry_no     = 0;
		memset(adxf_cmd_ncall, 0, sizeof(adxf_cmd_ncall));
		memset(adxf_cmd_hstry, 0xFF, sizeof(adxf_cmd_hstry));
		memset(adxf_ptinfo, 0, sizeof(adxf_ptinfo));
		memset(adxf_obj, 0, sizeof(adxf_obj));
	}
}
