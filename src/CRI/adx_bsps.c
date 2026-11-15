#include <CRI/adx.h>

#pragma auto_inline off

// 0x8021ED24
static int adxb_DecodeInfoSpsd(unsigned char* hdr, int len, short* hdlen, signed char* type, signed char* bps,
                               signed char* blk_len, signed char* nch, unsigned int* sfreq, int* nsmpl, int* blk_smpl,
                               short* pcm_type)
{
	*hdlen = hdr[7] << 4;
	*nch = (hdr[9] & 3) + 1;
	*sfreq = *(unsigned short*)(hdr + 0x2A);
	switch (hdr[8]) {
	case 0:
		*bps = 16;
		*blk_len = *nch << 1;
		*blk_smpl = 1;
		*nsmpl = *(int*)(hdr + 0xC) / 2;
		*pcm_type = 0;
		break;
	case 1:
		*bps = 8;
		*blk_len = *nch;
		*blk_smpl = 1;
		*nsmpl = *(int*)(hdr + 0xC);
		*pcm_type = 1;
		break;
	case 2:
	case 3:
		*bps = 4;
		*blk_len = *nch;
		*blk_smpl = 2;
		*nsmpl = *(int*)(hdr + 0xC) << 1;
		*pcm_type = 2;
		break;
	}
	/* Only 16-bit stereo is played the fields are forced to that */
	*blk_len = 2;
	*blk_smpl = 1;
	*nsmpl = *(int*)(hdr + 0xC) / 2;
	*bps = 16;
	*type = (signed char)0xFF;
	return 0;
}

// 0x8021EC38
int ADXB_DecodeHeaderSpsd(ADXB adxb, void* hdr, int len)
{
	short hdlen;

	adxb->hdr_ok = 1;
	if (adxb_DecodeInfoSpsd(hdr, len, &hdlen, &adxb->type, &adxb->bps, &adxb->blk_len, &adxb->nch,
	                        (unsigned int*)&adxb->sfreq, &adxb->total_smpl, &adxb->blk_smpl, &adxb->pcm_type) < 0) {
		return 0;
	}
	ADXB_SET_PCM_PRM(adxb);
	adxb->format = ADXB_FMT_SPSD;
	return hdlen;
}

// 0x8021E960
void ADXB_ExecOneSpsd(ADXB adxb)
{
	short* src;
	short* dst_l;
	short* dst_r;
	int n;
	int i;

	src = (short*)adxb->ibuf;
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
				*dst_l++ = src[0];
				*dst_r++ = src[1];
				src += 2;
			}
		} else {
			for (i = 0; i < n; i++) {
				*dst_l++ = src[0];
				src += 1;
			}
		}
		adxb->dec_nsmpl = n;
		adxb->dec_dtlen = adxb->nch * n * 2;
		adxb->stat = ADXB_STAT_DECODE;
	}
	if (adxb->stat == ADXB_STAT_DECODE) {
		adxb->add_wr(adxb->add_wr_obj, adxb->dec_dtlen, adxb->dec_nsmpl);
		adxb->stat = ADXB_STAT_DECEND;
	}
}

// 0x8021E92C
int ADXB_CheckSpsd(void* hdr)
{
	return memcmp(hdr, "SPSD", 4) == 0;
}
