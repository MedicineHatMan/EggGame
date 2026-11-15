#include <CRI/cri_cvfs.h>
#include <CRI/adx_tlk.h>
#include <CRI/adx.h>

const char* const volatile adxgcsdk_build = "\nADXGCSDK Ver.05Sep2002Patch2 Build:Mar  3 2003 16:48:01\n";

/* The memory file system in the next CRI file */

void adxgcsdk_fs_err(void* obj, const char* msg, void* hn);

/* What the game passes to ADXT_SetupDvdFs */
typedef struct {
	int rdmode;
} ADXGC_SPRM;

// 0x80226DF8
void ADXT_SetSfreqFix(ADXT adxt, short sw)
{
	ADXRNA_SetSfreqFix(adxt->rna, sw);
}

// 0x80226DD4
void adxgcsdk_fs_err(void* obj, const char* msg, void* hn)
{
	ADXERR_CallErrFunc1(msg);
}

/* Sets up the file systems "MFS" for memory and "GCD" for the disc the default */
// 0x80226D14
void ADXT_SetupDvdFs(ADXGC_SPRM* sprm)
{
	adxgcsdk_build;
	cvFsEntryErrFunc(adxgcsdk_fs_err, 0);
	cvFsAddDev("MFS", mfCiGetInterface, 0);
	cvFsEntryErrFunc(adxgcsdk_fs_err, 0);
	cvFsAddDev("GCD", gcCiGetInterface, 0);
	cvFsSetDefDev("GCD");
	if (sprm != 0) {
		gcci_SetRdMode(0, 0, 0, sprm->rdmode);
	} else {
		gcci_SetRdMode(0, 0, 0, 0);
	}
}
