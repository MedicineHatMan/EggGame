#include <CRI/adx.h>
#include <CRI/sj.h>

#pragma auto_inline off

#define ADXENC_MAX_OBJ 8
#define ADXENC_MAX_CH 16
#define ADXENC_MAX_NCH 2
#define ADXENC_BLK_SMPL 32

#define ADXENC_CLIP(x, lo, hi) ((((x) < (lo)) ? (lo) : (x)) > (hi) ? (hi) : (((x) < (lo)) ? (lo) : (x)))

typedef struct {
	signed char used; /* 00 */
	char pad1[3];     /* 01 */
	short c1;         /* 04 */
	short c2;         /* 06 */
	short h1;         /* 08 */
	short h2;         /* 0A */
} ADXENC_DEC;

/* One channel of analysis */
typedef struct {
	signed char used;              /* 00 */
	char pad1[3];                  /* 01 */
	short c1;                      /* 04 */
	short c2;                      /* 06 */
	short h1;                      /* 08 */
	short h2;                      /* 0A */
	int unkC;                      /* 0C */
	int unk10;                     /* 10 */
	int nsmpl;                     /* 14 */
	short resid[ADXENC_BLK_SMPL];  /* 18 */
	signed char nib[ADXENC_BLK_SMPL]; /* 58 */
	int maxabs;                    /* 78 */
	short scale;                   /* 7C */
	short pad7E;                   /* 7E */
	double inv;                    /* 80 */
	ADXENC_DEC* dec;               /* 88 */
	int pad8C;                     /* 8C */
} ADXENC_CH;

typedef struct {
	signed char used;                          /* 000 */
	signed char stat;                          /* 001 */
	signed char nch;                           /* 002 */
	signed char end_flg;                       /* 003 */
	SJ sji[ADXENC_MAX_NCH];                    /* 004 */
	SJ sjo;                                    /* 00C */
	int unk10[6];                              /* 010 */
	int unk28;                                 /* 028 */
	int out_len;                               /* 02C */
	int enc_nsmpl;                             /* 030 */
	int max_nsmpl;                             /* 034 */
	int unk38;                                 /* 038 */
	int req_nsmpl;                             /* 03C */
	int buf_nsmpl;                             /* 040 */
	int blk_smpl;                              /* 044 */
	int hdr_len;                               /* 048 */
	int type;                                  /* 04C */
	int blk_len;                               /* 050 */
	int bps;                                   /* 054 */
	int nch_prm;                               /* 058 */
	int sfreq;                                 /* 05C */
	int total_smpl;                            /* 060 */
	int cof;                                   /* 064 */
	int lp_ins_nsmpl;                          /* 068 */
	int nloop;                                 /* 06C */
	int lp_spos;                               /* 070 */
	int lp_sofst;                              /* 074 */
	int lp_epos;                               /* 078 */
	int lp_eofst;                              /* 07C */
	ADXENC_CH* ch[ADXENC_MAX_NCH];             /* 080 */
	short dly0[ADXENC_MAX_NCH];                /* 088 */
	short dly1[ADXENC_MAX_NCH];                /* 08C */
	short in[ADXENC_MAX_NCH][ADXENC_BLK_SMPL]; /* 090 */
	short w_res[ADXENC_MAX_NCH][ADXENC_BLK_SMPL]; /* 110 */
	short w_scl[ADXENC_MAX_NCH][ADXENC_BLK_SMPL]; /* 190 */
	short w_deq[ADXENC_MAX_NCH][ADXENC_BLK_SMPL]; /* 210 */
	short scale[ADXENC_MAX_NCH];               /* 290 */
	int pad294;                                /* 294 */
	double inv[ADXENC_MAX_NCH];                /* 298 */
	unsigned char blk[ADXENC_MAX_NCH][0x10];   /* 2A8 */
	short dly0_init[ADXENC_MAX_NCH];           /* 2C8 */
	short dly1_init[ADXENC_MAX_NCH];           /* 2CC */
	unsigned short key_x;                      /* 2D0 */
	short key_m;                               /* 2D2 */
	short key_a;                               /* 2D4 */
	short pad2D6;                              /* 2D6 */
} ADXENC_OBJ;

typedef ADXENC_OBJ* ADXENC;

/* The caller's handle in the test driver only its stream joint is used */
typedef struct {
	char pad[0x14];
	SJ sj; /* 14 */
} ADXENC_DST;

int adxenc_max_scale = 0x1000;
char* adxenc_copyright = "(c)CRI";

char adxenc_test_sjmem[2][0x20];
char adxenc_test_sjrbf[0x400];
int adxenc_lock_cnt;
int adxenc_init_cnt;
int adxenc_pad[3];
ADXENC_CH adxenc_ch_obj[ADXENC_MAX_CH];
ADXENC_DEC adxenc_dec_obj[ADXENC_MAX_CH];
ADXENC_OBJ adxenc_obj[ADXENC_MAX_OBJ];

static inline int adxenc_Put16(SJ sj, short v)
{
	SJCK ck;

	SJ_GetChunk(sj, SJ_LIN_FREE, 2, &ck);
	if (ck.len < 2) {
		SJ_UngetChunk(sj, SJ_LIN_FREE, &ck);
		return 0;
	}
	*(short*)ck.data = v;
	SJ_PutChunk(sj, SJ_LIN_DATA, &ck);
	return 2;
}

static inline int adxenc_Put32(SJ sj, int v)
{
	SJCK ck;

	SJ_GetChunk(sj, SJ_LIN_FREE, 4, &ck);
	if (ck.len < 4) {
		SJ_UngetChunk(sj, SJ_LIN_FREE, &ck);
		return 0;
	}
	*(int*)ck.data = v;
	SJ_PutChunk(sj, SJ_LIN_DATA, &ck);
	return 4;
}

static inline int adxenc_PutBuf(SJ sj, const void* data, int len)
{
	SJCK ck;

	SJ_GetChunk(sj, SJ_LIN_FREE, len, &ck);
	if (ck.len < len) {
		SJ_UngetChunk(sj, SJ_LIN_FREE, &ck);
		return 0;
	}
	memcpy(ck.data, data, (unsigned short)len);
	SJ_PutChunk(sj, SJ_LIN_DATA, &ck);
	return len;
}

static inline int adxenc_Put8(SJ sj, unsigned char v)
{
	return adxenc_PutBuf(sj, &v, 1);
}

/* Prediction error of one sample kept in the channel's residual buffer */
static inline void adxenc_CalcResid(ADXENC_CH* ch, int i, short s)
{
	int e;
	int a;

	if (i == 0) {
		int j;
		for (j = 0; j < ch->nsmpl; j++) {
			ch->resid[j] = 0;
		}
		ch->maxabs = 0;
	}
	if (ch != 0) {
		e = s - ((ch->c1 * ch->h1) >> 12) - ((ch->c2 * ch->h2) >> 12);
		ch->resid[i] = ADXENC_CLIP(e, -0x8000, 0x7FFF);
		a = ch->resid[i];
		if (a < 0) {
			a = -a;
		}
		if (ch->maxabs < a) {
			ch->maxabs = a;
		}
		ch->h2 = ch->h1;
		ch->h1 = s;
	}
}

static inline int adxenc_Quantize(int q)
{
	int nib;

	if (q < 0) {
		nib = (q - 0x924) / 0x1249;
	} else {
		nib = (q + 0x924) / 0x1249;
	}
	return ADXENC_CLIP(nib, -8, 7);
}

// 0x80222350
int adxenc_EncodeCh(ADXENC enc, int c)
{
	short* in;
	ADXENC_CH* ch;
	ADXENC_DEC* dec;
	int sc;
	int i;
	int r;
	int q;
	int nib;
	int d;
	int v;

	in = enc->in[c];
	ch = enc->ch[c];
	dec = ch->dec;
	ch->h1 = enc->dly0[c];
	ch->h2 = enc->dly1[c];
	for (i = 0; i < enc->blk_smpl; i++) {
		adxenc_CalcResid(ch, i, in[i]);
	}
	sc = (ch->maxabs - 1) / 7 + 1;
	if (sc > adxenc_max_scale) {
		sc = adxenc_max_scale;
	}
	if (sc < 2) {
		sc = 1;
	}
	ch->scale = sc;
	if (ch->maxabs == 0) {
		ch->inv = 32767.0;
	} else {
		ch->inv = 32767.0 / (double)ch->maxabs;
	}
	dec->h1 = enc->dly0[c];
	dec->h2 = enc->dly1[c];
	for (i = 0; i < enc->blk_smpl; i++) {
		ch->h1 = dec->h1;
		ch->h2 = dec->h2;
		adxenc_CalcResid(ch, i, in[i]);
		if (ch == 0) {
			r = 0;
		} else {
			r = ch->resid[i];
		}
		q = (int)((double)r * ch->inv);
		q = (short)ADXENC_CLIP(q, -0x8000, 0x7FFF);
		nib = adxenc_Quantize(q);
		ch->nib[i] = nib;
		d = nib * ch->scale;
		d = ADXENC_CLIP(d, -0x8000, 0x7FFF);
		if (ch->dec != 0) {
			dec = ch->dec;
			v = (short)d + ((dec->c1 * dec->h1 + dec->c2 * dec->h2) >> 12);
			dec->h2 = dec->h1;
			dec->h1 = ADXENC_CLIP(v, -0x8000, 0x7FFF);
		}
	}
	return 0;
}

// 0x8022211C
void adxenc_EndBlock(ADXENC enc, int c)
{
	ADXENC_CH* ch;
	unsigned char* p;
	int per;
	int k;
	int i;
	int q;
	int nib;
	int d;
	int shift;
	int pos;

	k = 0;
	per = 8 / enc->bps;
	ch = enc->ch[c];
	p = enc->blk[c] - 1;
	for (i = 0; i < enc->blk_smpl; i++) {
		enc->w_res[c][i] = (ch == 0) ? 0 : ch->resid[i];
		q = (int)(ch->inv * (double)enc->w_res[c][i]);
		enc->w_scl[c][i] = ADXENC_CLIP(q, -0x8000, 0x7FFF);
		nib = adxenc_Quantize(enc->w_scl[c][i]);
		d = (int)(ch->inv * (double)(nib * ch->scale));
		enc->w_deq[c][i] = ADXENC_CLIP(d, -0x8000, 0x7FFF);
		if (i == (i / per) * per) {
			k = 1;
			*++p = 0;
		}
		pos = per - k;
		k++;
		shift = 8 - enc->bps;
		*p |= (((nib << shift) & 0xFF) >> shift) << (enc->bps * pos);
	}
}

// 0x80221EDC
int adxenc_PutBlocks(ADXENC enc)
{
	SJ sj;
	int n;
	int c;
	unsigned short v;
	int* w;

	n = 0;
	sj = enc->sjo;
	for (c = 0; c < enc->nch_prm; c++) {
		v = (enc->scale[c] - 1) ^ enc->key_x;
		enc->key_x = enc->key_a + enc->key_x * enc->key_m;
		enc->key_x &= 0x7FFF;
		w = (int*)enc->blk[c];
		if (w[0] == 0 && w[1] == 0 && w[2] == 0 && w[3] == 0) {
			v = 0;
		}
		adxenc_Put8(sj, v >> 8);
		adxenc_Put8(sj, v);
		adxenc_PutBuf(sj, enc->blk[c], 0x10);
		n += 0x12;
	}
	return n;
}

// 0x80221C40
int adxenc_EncodeFrames(ADXENC enc)
{
	SJ sj;
	int written;
	int n;
	int got;
	int c;
	short* bufs[ADXENC_MAX_NCH];
	SJCK ck;

	written = 0;
	sj = enc->sjo;
	do {
		if ((SJ_GetNumData(sj, SJ_LIN_FREE) / 0x12) / enc->nch_prm < 1) {
			return written;
		}
		n = enc->req_nsmpl - enc->enc_nsmpl;
		bufs[0] = enc->in[0];
		bufs[1] = enc->in[1];
		if (n > enc->blk_smpl) {
			n = enc->blk_smpl;
		}
		got = 0;
		for (c = 0; c < enc->nch_prm; c++) {
			got = (unsigned int)SJ_GetNumData(enc->sji[c], SJ_LIN_DATA) >> 1;
			if (got < n) {
				got = 0;
				goto copied;
			}
		}
		for (c = 0; c < enc->nch_prm; c++) {
			for (got = 0; got < n; got += (short)((unsigned int)ck.len >> 1)) {
				SJ_GetChunk(enc->sji[c], SJ_LIN_DATA, n << 1, &ck);
				memcpy(bufs[c] + got, ck.data, ck.len);
				SJ_PutChunk(enc->sji[c], SJ_LIN_FREE, &ck);
			}
		}
	copied:
		if (got == 0) {
			n = 0;
		} else {
			if (n < enc->blk_smpl) {
				for (c = 0; c < enc->nch_prm; c++) {
					if (bufs[c] != 0) {
						memset(bufs[c] + n, 0, (enc->blk_smpl - n) * 2);
					}
				}
			}
			enc->enc_nsmpl += enc->blk_smpl;
			for (c = 0; c < enc->nch_prm; c++) {
				adxenc_EncodeCh(enc, c);
				enc->scale[c] = enc->ch[c]->scale;
				enc->inv[c] = enc->ch[c]->inv;
				enc->dly0[c] = enc->ch[c]->dec->h1;
				enc->dly1[c] = enc->ch[c]->dec->h2;
				adxenc_EndBlock(enc, c);
			}
			n = enc->blk_smpl;
		}
		if (n == 0) {
			return written;
		}
		written += adxenc_PutBlocks(enc);
	} while (enc->enc_nsmpl < enc->req_nsmpl);
	return written;
}

// 0x80221A10
int adxenc_PutFooter(ADXENC enc)
{
	SJ sj;
	int n;
	int m;
	int i;

	sj = enc->sjo;
	if (enc->nloop <= 0) {
		n = enc->blk_len;
	} else {
		n = ((enc->out_len + enc->blk_len + 0x7FF) / 0x800) * 0x800 - enc->out_len;
	}
	if (SJ_GetNumData(sj, SJ_LIN_FREE) < n) {
		return 0;
	}
	adxenc_Put16(sj, (short)0x8001);
	m = n - 4;
	adxenc_Put16(sj, m);
	for (i = 0; i < m; i++) {
		adxenc_Put8(sj, 0);
	}
	return n;
}

// 0x80220B04
int adxenc_PutHeader(ADXENC enc, SJ sj)
{
	int crlen;
	int n;
	int i;

	crlen = strlen(adxenc_copyright);
	if (adxenc_Put16(sj, (short)0x8000) != 2) {
		return 0;
	}
	if (adxenc_Put16(sj, enc->hdr_len) != 2) {
		return 0;
	}
	if (adxenc_Put8(sj, enc->type) != 1) {
		return 0;
	}
	if (adxenc_Put8(sj, enc->blk_len) != 1) {
		return 0;
	}
	if (adxenc_Put8(sj, enc->bps) != 1) {
		return 0;
	}
	if (adxenc_Put8(sj, enc->nch_prm) != 1) {
		return 0;
	}
	if (adxenc_Put32(sj, enc->sfreq) != 4) {
		return 0;
	}
	if (adxenc_Put32(sj, enc->total_smpl) != 4) {
		return 0;
	}
	if (adxenc_Put16(sj, enc->cof) != 2) {
		return 0;
	}
	if (adxenc_Put8(sj, 4) != 1) {
		return 0;
	}
	if (adxenc_Put8(sj, (enc->key_x != 0) ? 8 : 0) != 1) {
		return 0;
	}
	if (adxenc_Put32(sj, 0) != 4) {
		return 0;
	}
	if (adxenc_Put16(sj, enc->dly0_init[0]) != 2) {
		return 0;
	}
	if (adxenc_Put16(sj, enc->dly1_init[0]) != 2) {
		return 0;
	}
	if (adxenc_Put16(sj, enc->dly0_init[1]) != 2) {
		return 0;
	}
	if (adxenc_Put16(sj, enc->dly1_init[1]) != 2) {
		return 0;
	}
	n = 0x1C;
	if (enc->nloop > 0) {
		if (adxenc_Put16(sj, enc->lp_ins_nsmpl) != 2) {
			return 0;
		}
		if (adxenc_Put16(sj, enc->nloop) != 2) {
			return 0;
		}
		n = 0x20;
		for (i = 0; i < enc->nloop; i++) {
			if (adxenc_Put16(sj, i) != 2) {
				return 0;
			}
			if (adxenc_Put16(sj, 1) != 2) {
				return 0;
			}
			if (adxenc_Put32(sj, enc->lp_spos) != 4) {
				return 0;
			}
			if (adxenc_Put32(sj, enc->lp_sofst) != 4) {
				return 0;
			}
			if (adxenc_Put32(sj, enc->lp_epos) != 4) {
				return 0;
			}
			if (adxenc_Put32(sj, enc->lp_eofst) != 4) {
				return 0;
			}
			n += 0x14;
		}
	}
	for (; n < enc->hdr_len - crlen; n++) {
		if (adxenc_Put8(sj, 0) != 1) {
			return 0;
		}
	}
	if (adxenc_PutBuf(sj, adxenc_copyright, crlen) != crlen) {
		return 0;
	}
	return n + crlen + 4;
}

// 0x80220AC0
void ADXENC_Init(void)
{
	adxenc_init_cnt++;
	memset(adxenc_obj, 0, sizeof(adxenc_obj));
}

// 0x80220A7C
void ADXENC_Finish(void)
{
	adxenc_init_cnt--;
	memset(adxenc_obj, 0, sizeof(adxenc_obj));
}

static inline ADXENC_CH* adxenc_CreateCh(int blk_smpl)
{
	ADXENC_DEC* dec;
	ADXENC_CH* ch;
	int i;

	for (i = 0; i < ADXENC_MAX_CH; i++) {
		if (adxenc_dec_obj[i].used == 0) {
			break;
		}
	}
	dec = &adxenc_dec_obj[i];
	if (i >= ADXENC_MAX_CH) {
		dec = 0;
	}
	if (dec == 0) {
		return 0;
	}
	for (i = 0; i < ADXENC_MAX_CH; i++) {
		if (adxenc_ch_obj[i].used == 0) {
			break;
		}
	}
	ch = &adxenc_ch_obj[i];
	ch->nsmpl = blk_smpl;
	ch->dec = dec;
	if (i < ADXENC_MAX_CH) {
		ch->used = 1;
	} else {
		ch = 0;
	}
	return ch;
}

// 0x8022047C
ADXENC ADXENC_Create(int nch, SJ* sji, SJ sjo)
{
	ADXENC enc;
	int i;

	for (i = 0; i < ADXENC_MAX_OBJ; i++) {
		if (adxenc_obj[i].used == 0) {
			break;
		}
	}
	if (i == ADXENC_MAX_OBJ) {
		return 0;
	}
	enc = &adxenc_obj[i];
	enc->nch = nch;
	for (i = 0; i < nch; i++) {
		enc->sji[i] = sji[i];
	}
	enc->sjo = sjo;
	enc->stat = 0;
	enc->unk28 = 0;
	enc->out_len = 0;
	enc->enc_nsmpl = 0;
	enc->max_nsmpl = 0x7FFF0000;
	enc->hdr_len = ADX_CalcHdrInfoLen(0, 0, 4, 4);
	enc->type = 3;
	enc->nch_prm = nch;
	enc->sfreq = 44100;
	enc->bps = 4;
	enc->blk_len = 0x12;
	enc->blk_smpl = ((enc->blk_len - 2) * 8) / enc->bps;
	enc->total_smpl = 0x7FFF0000;
	enc->cof = 500;
	enc->lp_ins_nsmpl = 0;
	enc->nloop = 0;
	enc->lp_spos = 0;
	enc->lp_sofst = 0;
	enc->lp_epos = 0;
	enc->lp_eofst = 0;
	enc->unk38 = 0;
	enc->req_nsmpl = 0;
	enc->buf_nsmpl = enc->blk_smpl * ((enc->total_smpl - 1) / enc->blk_smpl + 1);
	enc->ch[0] = adxenc_CreateCh(enc->blk_smpl);
	enc->ch[1] = adxenc_CreateCh(enc->blk_smpl);
	enc->dly1_init[0] = 0;
	enc->dly0_init[0] = 0;
	enc->dly1_init[1] = 0;
	enc->dly0_init[1] = 0;
	enc->used = 1;
	return enc;
}

// 0x802203F0
void ADXENC_Destroy(ADXENC enc)
{
	ADXENC_CH* ch;

	if (enc != 0) {
		ADXCRS_Lock();
		ch = enc->ch[0];
		if (ch != 0 && ch != 0) {
			ch->used = 0;
			memset(ch, 0, sizeof(ADXENC_CH));
		}
		ch = enc->ch[1];
		if (ch != 0 && ch != 0) {
			ch->used = 0;
			memset(ch, 0, sizeof(ADXENC_CH));
		}
		memset(enc, 0, sizeof(enc));
		ADXCRS_Unlock();
	}
}

// 0x802202F0
void ADXENC_Start(ADXENC enc)
{
	int c;
	int len;
	SJCK ck;

	for (c = 0; c < enc->nch_prm; c++) {
		len = enc->lp_ins_nsmpl << 1;
		SJ_GetChunk(enc->sji[c], SJ_LIN_FREE, len, &ck);
		if (ck.len != len) {
			SJ_UngetChunk(enc->sji[c], SJ_LIN_FREE, &ck);
			for (;;) {
			}
		}
		memset(ck.data, 0, ck.len);
		SJ_PutChunk(enc->sji[c], SJ_LIN_DATA, &ck);
	}
	enc->unk28 = 0;
	enc->out_len = 0;
	enc->enc_nsmpl = 0;
	enc->end_flg = 0;
	enc->stat = 1;
}

// 0x802202E4
void ADXENC_Terminate(ADXENC enc)
{
	enc->end_flg = 1;
}

// 0x802202C8
void ADXENC_SetPrm(ADXENC enc, int nch, int sfreq, int nsmpl)
{
	enc->nch_prm = nch;
	enc->sfreq = sfreq;
	enc->req_nsmpl = nsmpl;
	enc->total_smpl = nsmpl;
	enc->hdr_len = 0x11C;
}

// 0x8022023C
void adxenc_ExecData(ADXENC enc)
{
	int n;

	for (;;) {
		if (enc->end_flg != 0) {
			break;
		}
		do {
			n = adxenc_EncodeFrames(enc);
			if (n == 0) {
				return;
			}
			enc->out_len += n;
		} while (enc->enc_nsmpl < enc->req_nsmpl);
		enc->end_flg = 1;
	}
	if (adxenc_PutFooter(enc) > 0) {
		enc->stat = 3;
	}
}

// 0x802200AC
void adxenc_ExecHn(ADXENC enc)
{
	SJ sjo;
	int j;
	int i;
	int c;
	int n;
	short c2;
	short c1;
	SJCK ck;

	if (enc->stat == 1) {
		sjo = enc->sjo;
		for (c = 0; c < enc->nch_prm; c++) {
			SJ_GetChunk(enc->sji[c], SJ_LIN_DATA, 2, &ck);
			if (ck.len == 0) {
				break;
			}
			enc->dly0_init[c] = enc->dly1_init[c] = *(short*)ck.data;
			SJ_UngetChunk(enc->sji[c], SJ_LIN_DATA, &ck);
		}
		if (c >= enc->nch_prm) {
			for (i = 0; i < enc->nch_prm; i++) {
				enc->dly0[i] = enc->dly0_init[i];
				enc->dly1[i] = enc->dly1_init[i];
			}
			n = adxenc_PutHeader(enc, sjo);
			if (n != 0) {
				enc->out_len += n;
				for (j = 0; j < enc->nch_prm; j++) {
					ADXENC_CH* ch = enc->ch[j];
					ADX_GetCoefficient((short)enc->cof, enc->sfreq, &c1, &c2);
					ch->c1 = c1;
					ch->c2 = c2;
					{
						ADXENC_DEC* dec = ch->dec;
						dec->c1 = c1;
						dec->c2 = c2;
					}
				}
				enc->stat = 2;
			}
		}
	} else if (enc->stat == 2) {
		adxenc_ExecData(enc);
	}
}

// 0x80220050
void ADXENC_ExecServer(void)
{
	ADXENC enc;
	int i;

	for (i = 0; i < ADXENC_MAX_OBJ; i++) {
		enc = &adxenc_obj[i];
		if (enc->used == 1) {
			adxenc_ExecHn(enc);
		}
	}
}

// 0x80220038
void ADXENC_Lock(void)
{
	adxenc_lock_cnt++;
}

// 0x80220020
void ADXENC_Unlock(void)
{
	adxenc_lock_cnt--;
}

// 0x8021FE78
void adxenc_EncodeToBuf(ADXENC_DST* dst, int nch, int sfreq, int nsmpl)
{
	SJ rbf;
	SJ osj;
	ADXENC enc;
	SJ sji[2];
	SJCK cki;
	SJCK cko;

	ADXENC_Init();
	rbf = SJRBF_Create(adxenc_test_sjrbf, sizeof(adxenc_test_sjrbf), 0);
	sji[0] = SJMEM_Create(adxenc_test_sjmem[0], sizeof(adxenc_test_sjmem[0]));
	sji[1] = SJMEM_Create(adxenc_test_sjmem[1], sizeof(adxenc_test_sjmem[1]));
	osj = dst->sj;
	enc = ADXENC_Create(2, sji, rbf);
	ADXENC_SetPrm(enc, nch, sfreq, nsmpl);
	ADXENC_Start(enc);
	ADXENC_ExecServer();
	SJ_GetChunk(rbf, SJ_LIN_DATA, 0x400, &cki);
	if (cki.len == 0) {
		for (;;) {
		}
	}
	SJ_GetChunk(osj, SJ_LIN_FREE, cki.len, &cko);
	if (cko.len < cki.len) {
		for (;;) {
		}
	}
	memcpy(cko.data, cki.data, cki.len);
	SJ_PutChunk(rbf, SJ_LIN_FREE, &cki);
	SJ_PutChunk(osj, SJ_LIN_DATA, &cko);
	ADXENC_Terminate(enc);
	ADXENC_Destroy(enc);
	SJ_Destroy(rbf);
	SJ_Destroy(sji[1]);
	SJ_Destroy(sji[0]);
	ADXENC_Finish();
}
