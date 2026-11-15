#include <CRI/adx.h>

int adx_decode_ste_as_mono_sw;

// 0x8021F790
void ADX_SetDecodeSteAsMonoSw(int sw)
{
	adx_decode_ste_as_mono_sw = sw;
}
