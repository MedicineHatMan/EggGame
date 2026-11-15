#include <CRI/adx.h>

// 0x8021F014
void ADXAMP_Destroy(ADXAMP amp)
{
	if (amp != 0) {
		ADXCRS_Lock();
		memset(amp, 0, sizeof(ADXAMP_OBJ));
		ADXCRS_Unlock();
	}
}

// 0x8021EE5C
void ADXAMP_Start(ADXAMP amp)
{
	int i;
	SJ sj;
	SJCK ck;

	for (i = 0; i < amp->maxnch; i++) {
		amp->amp[i] = 0;
	}
	amp->frm_cnt = 0;
	for (i = 0; i < amp->maxnch; i++) {
		sj = amp->sji[i];
		SJ_Reset(sj);
		SJ_GetChunk(sj, SJ_LIN_FREE, SJ_GetNumData(sj, SJ_LIN_FREE), &ck);
		memset(ck.data, 0, ck.len);
		SJ_UngetChunk(sj, SJ_LIN_FREE, &ck);
	}
	for (i = 0; i < amp->maxnch; i++) {
		sj = amp->sjo[i];
		SJ_Reset(sj);
		SJ_GetChunk(sj, SJ_LIN_FREE, SJ_GetNumData(sj, SJ_LIN_FREE), &ck);
		memset(ck.data, 0, ck.len);
		SJ_UngetChunk(sj, SJ_LIN_FREE, &ck);
	}
	amp->stat = 2;
}

// 0x8021EE50
void ADXAMP_Stop(ADXAMP amp)
{
	amp->stat = 0;
}

// 0x8021EE48
void ADXAMP_SetSfreq(ADXAMP amp, int sfreq)
{
	amp->sfreq = sfreq;
}
