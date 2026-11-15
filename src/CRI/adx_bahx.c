#include <CRI/adx.h>

void (*adxb_ahx_setsj_func)(void* ahx, SJ sj);
void (*adxb_ahx_exec_func)(ADXB adxb);

// 0x8021BDB8
void ADXB_SetAhxInSj(ADXB adxb, SJ sj)
{
	if (adxb->ahx != 0) {
		adxb_ahx_setsj_func(adxb->ahx, sj);
	}
}

// 0x8021BD94
void ADXB_SetAhxDecSmpl(ADXB adxb, int nsmpl)
{
	adxb->ahx_dec_smpl = nsmpl;
	adxb->ahx_dec_frm  = nsmpl / 96;
}

// 0x8021BD68
void ADXB_ExecOneAhx(ADXB adxb)
{
	adxb_ahx_exec_func(adxb);
}
