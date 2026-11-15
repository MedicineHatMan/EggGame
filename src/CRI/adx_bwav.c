#include <CRI/adx.h>

#define ADXB_SWAP32(v) (((v) << 24) | (((v) & 0xFF00) << 8) | (((v) >> 8) & 0xFF00) | ((v) >> 24))
#define ADXB_SWAP16(v) ((unsigned short)(((v) >> 8) | ((v) << 8)))

char* adxb_wav_fmt_id  = "fmt ";
char* adxb_wav_data_id = "data";

// 0x8021BB00
static int adxb_DecodeInfoWav(unsigned char* hdr, int len, short* hdlen, signed char* type, signed char* bps, signed char* blk_len,
                              signed char* nch, unsigned int* sfreq, int* nsmpl, int* blk_smpl, short* pcm_type)
{
	unsigned char* fmt;
	unsigned int size;
	unsigned int dsize;
	unsigned int v;
	int i;
	int j;

	for (i = 0; i < len; i++) {
		if (memcmp(hdr + i, adxb_wav_fmt_id, 4) == 0) {
			break;
		}
	}
	if (i == len) {
		return -1;
	}
	if (i % 4 != 0) {
		return -1;
	}
	fmt = hdr + i;
	if ((short)ADXB_SWAP16(*(unsigned short*)(fmt + 8)) >= 2) {
		return -1;
	}
	for (j = 0; j < len; j++) {
		if (memcmp(hdr + j, adxb_wav_data_id, 4) == 0) {
			break;
		}
	}
	if (j == len) {
		return -1;
	}
	size      = *(unsigned int*)(hdr + j + 4);
	*hdlen    = j + 8;
	*type     = (signed char)0xFF;
	dsize     = ADXB_SWAP32(size);
	v         = *(unsigned int*)(fmt + 0xC);
	*sfreq    = ADXB_SWAP32(v);
	*nch      = *(unsigned short*)(fmt + 0xA) >> 8;
	*bps      = *(unsigned short*)(fmt + 0x16) >> 8;
	*blk_len  = *(unsigned short*)(fmt + 0x14) >> 8;
	*nsmpl    = (int)dsize / *blk_len;
	*blk_smpl = 1;
	if (*bps == 16) {
		*pcm_type = 0;
	} else if (*bps == 8) {
		*pcm_type = 1;
	} else if (*bps == 4) {
		*blk_len  = *nch << 1;
		*blk_smpl = 4;
		*nsmpl    = ((int)dsize / 2) / *nch;
		*bps      = 16;
		*pcm_type = 2;
	}
	if (*bps == 0) {
		return -1;
	}
	if (*blk_len == 0) {
		return -1;
	}
	if (*nch < 1 || *nch > 2) {
		return -1;
	}
	return (*sfreq == 0) ? -1 : 0;
}

// 0x8021BA14
int ADXB_DecodeHeaderWav(ADXB adxb, void* hdr, int len)
{
	short hdlen;

	adxb->hdr_ok = 1;
	if (adxb_DecodeInfoWav(hdr, len, &hdlen, &adxb->type, &adxb->bps, &adxb->blk_len, &adxb->nch, (unsigned int*)&adxb->sfreq,
	                       &adxb->total_smpl, &adxb->blk_smpl, &adxb->pcm_type)
	    < 0) {
		return 0;
	}
	ADXB_SET_PCM_PRM(adxb);
	adxb->format = ADXB_FMT_WAV;
	return hdlen;
}

// 0x8021B6E8
static void adxb_ExecOneWav16(ADXB adxb)
{
	unsigned short* src;
	short* dst_l;
	short* dst_r;
	int n;
	int i;

	src = (unsigned short*)adxb->ibuf;
	if (adxb->stat == ADXB_STAT_EXEC && ADXPD_GetStat(adxb->xpd) == 0) {
		adxb->get_wr(adxb->get_wr_obj, &adxb->wpos, &adxb->wlen, &adxb->wnsmpl);
		n = adxb->obuf_size - adxb->wpos;
		if (n > adxb->wlen) {
			n = adxb->wlen;
		}
		if (n > adxb->ibuf_nblk) {
			n = adxb->ibuf_nblk;
		}
		dst_l = adxb->obuf + adxb->wpos;
		if (adxb->nch == 2) {
			dst_r = adxb->obuf + (adxb->obuf_dist + adxb->wpos);
			for (i = 0; i < n; i++) {
				dst_l[i] = ADXB_SWAP16(src[i * 2]);
				dst_r[i] = ADXB_SWAP16(src[i * 2 + 1]);
			}
		} else {
			for (i = 0; i < n; i++) {
				dst_l[i] = ADXB_SWAP16(src[i]);
			}
		}
		adxb->dec_nsmpl = n;
		adxb->dec_dtlen = adxb->nch * (n * 2);
		adxb->stat      = ADXB_STAT_DECODE;
	}
	if (adxb->stat == ADXB_STAT_DECODE) {
		adxb->add_wr(adxb->add_wr_obj, adxb->dec_dtlen, adxb->dec_nsmpl);
		adxb->stat = ADXB_STAT_DECEND;
	}
}

// 0x8021B344
static void adxb_ExecOneWav8(ADXB adxb)
{
	unsigned char* src;
	short* dst_l;
	short* dst_r;
	int i;
	int n;

	src = (unsigned char*)adxb->ibuf;
	if (adxb->stat == ADXB_STAT_EXEC && ADXPD_GetStat(adxb->xpd) == 0) {
		adxb->get_wr(adxb->get_wr_obj, &adxb->wpos, &adxb->wlen, &adxb->wnsmpl);
		n = adxb->obuf_size - adxb->wpos;
		if (n > adxb->wlen) {
			n = adxb->wlen;
		}
		if (n > adxb->ibuf_nblk) {
			n = adxb->ibuf_nblk;
		}
		dst_l = adxb->obuf + adxb->wpos;
		if (adxb->nch == 2) {
			dst_r = adxb->obuf + (adxb->obuf_dist + adxb->wpos);
			for (i = 0; i < n; i++) {
				dst_l[i] = ((src[i * 2] - 0x80) & 0xFF) << 8;
				dst_r[i] = ((src[i * 2 + 1] - 0x80) & 0xFF) << 8;
			}
		} else {
			for (i = 0; i < n; i++) {
				dst_l[i] = ((src[i] - 0x80) & 0xFF) << 8;
			}
		}
		adxb->dec_nsmpl = n;
		adxb->dec_dtlen = n * adxb->nch;
		adxb->stat      = ADXB_STAT_DECODE;
	}
	if (adxb->stat == ADXB_STAT_DECODE) {
		adxb->add_wr(adxb->add_wr_obj, adxb->dec_dtlen, adxb->dec_nsmpl);
		adxb->stat = ADXB_STAT_DECEND;
	}
}

// 0x8021AF94
static void adxb_ExecOneWav4(ADXB adxb)
{
	unsigned char* src;
	short* dst_l;
	short* dst_r;
	int n;
	int i;

	src = (unsigned char*)adxb->ibuf;
	if (adxb->stat == ADXB_STAT_EXEC && ADXPD_GetStat(adxb->xpd) == 0) {
		adxb->get_wr(adxb->get_wr_obj, &adxb->wpos, &adxb->wlen, &adxb->wnsmpl);
		n = adxb->obuf_size - adxb->wpos;
		if (n > adxb->wlen) {
			n = adxb->wlen;
		}
		if (n > adxb->ibuf_nblk) {
			n = adxb->ibuf_nblk;
		}
		dst_l = adxb->obuf + adxb->wpos;
		if (adxb->nch == 2) {
			dst_r = adxb->obuf + (adxb->obuf_dist + adxb->wpos);
			for (i = 0; i < n; i++) {
				dst_l[i] = src[i * 4] | (src[i * 4 + 2] << 8);
				dst_r[i] = src[i * 4 + 1] | (src[i * 4 + 3] << 8);
			}
		} else {
			for (i = 0; i < n; i++) {
				dst_l[i] = src[i * 2] | (src[i * 2 + 1] << 8);
			}
		}
		adxb->dec_nsmpl = n;
		adxb->dec_dtlen = adxb->nch * (n * 2);
		adxb->stat      = ADXB_STAT_DECODE;
	}
	if (adxb->stat == ADXB_STAT_DECODE) {
		adxb->add_wr(adxb->add_wr_obj, adxb->dec_dtlen, adxb->dec_nsmpl);
		adxb->stat = ADXB_STAT_DECEND;
	}
}

// 0x8021AF2C
int ADXB_CheckWav(void* hdr)
{
	if (memcmp(hdr, "RIFF", 4) == 0 && memcmp((char*)hdr + 8, "WAVE", 4) == 0) {
		return 1;
	}
	return 0;
}

// 0x8021AEE0
void ADXB_ExecOneWav(ADXB adxb)
{
	if (adxb->pcm_type == 2) {
		adxb_ExecOneWav4(adxb);
	} else if (adxb->pcm_type == 1) {
		adxb_ExecOneWav8(adxb);
	} else if (adxb->pcm_type == 0) {
		adxb_ExecOneWav16(adxb);
	}
}
