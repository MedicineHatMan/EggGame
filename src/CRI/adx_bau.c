#include <CRI/adx.h>

#define ADXB_LE32(p) ((unsigned int)(p)[0] | ((unsigned int)(p)[1] << 8) | ((unsigned int)(p)[2] << 16) | ((unsigned int)(p)[3] << 24))
#define ADXB_SWAP32(v) (((v) << 24) | (((v) & 0xFF00) << 8) | (((v) >> 8) & 0xFF00) | ((v) >> 24))

short adxb_ulaw_tbl[256] = {
	-32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956,
	-23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764,
	-15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412,
	-11900, -11388, -10876, -10364, -9852, -9340, -8828, -8316,
	-7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
	-5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
	-3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
	-2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
	-1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
	-1372, -1308, -1244, -1180, -1116, -1052, -988, -924,
	-876, -844, -812, -780, -748, -716, -684, -652,
	-620, -588, -556, -524, -492, -460, -428, -396,
	-372, -356, -340, -324, -308, -292, -276, -260,
	-244, -228, -212, -196, -180, -164, -148, -132,
	-120, -112, -104, -96, -88, -80, -72, -64,
	-56, -48, -40, -32, -24, -16, -8, 0,
	32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
	23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
	15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
	11900, 11388, 10876, 10364, 9852, 9340, 8828, 8316,
	7932, 7676, 7420, 7164, 6908, 6652, 6396, 6140,
	5884, 5628, 5372, 5116, 4860, 4604, 4348, 4092,
	3900, 3772, 3644, 3516, 3388, 3260, 3132, 3004,
	2876, 2748, 2620, 2492, 2364, 2236, 2108, 1980,
	1884, 1820, 1756, 1692, 1628, 1564, 1500, 1436,
	1372, 1308, 1244, 1180, 1116, 1052, 988, 924,
	876, 844, 812, 780, 748, 716, 684, 652,
	620, 588, 556, 524, 492, 460, 428, 396,
	372, 356, 340, 324, 308, 292, 276, 260,
	244, 228, 212, 196, 180, 164, 148, 132,
	120, 112, 104, 96, 88, 80, 72, 64,
	56, 48, 40, 32, 24, 16, 8, 0,
};

// 0x8021D638
static unsigned char* adxb_DecodeInfoAu(unsigned char* hdr, int len, unsigned int* sfreq, unsigned int* nch, int* bps,
                                        int* nsmpl, int* pcm_type)
{
	unsigned int id;
	int hdsize;
	int enc;
	int size;

	id = ADXB_LE32(hdr);
	if (id != 0x0064732E && id != 0x646E732E) {
		return 0;
	}
	hdsize = *(int*)(hdr + 4);
	if (len < hdsize) {
		return 0;
	}
	enc = *(int*)(hdr + 0xC);
	size = *(int*)(hdr + 8);
	if (enc == 2) {
		*pcm_type = 1;
		*bps = 8;
	} else if (enc == 1) {
		*pcm_type = 2;
		*bps = 8;
	} else if (enc == 3) {
		*pcm_type = 0;
		*bps = 16;
	} else {
		return 0;
	}
	*sfreq = ADXB_LE32(hdr + 0x10);
	*sfreq = ADXB_SWAP32(*sfreq);
	*nch = ADXB_LE32(hdr + 0x14);
	*nch = ADXB_SWAP32(*nch);
	if (*pcm_type == 2) {
		*nsmpl = size / (int)*nch;
	} else if (*pcm_type == 1) {
		*nsmpl = size / (int)*nch;
	} else if (*pcm_type == 0) {
		*nsmpl = (size / 2) / (int)*nch;
	} else {
		*nsmpl = 0x7FFF0000;
	}
	return hdr + hdsize;
}

// 0x8021D5D0
int ADXB_CheckAu(void* hdr)
{
	if (memcmp(hdr, ".snd", 4) == 0 || memcmp(hdr, ".sd", 4) == 0) {
		return 1;
	}
	return 0;
}

// 0x8021D458
int ADXB_DecodeHeaderAu(ADXB adxb, void* hdr, int len)
{
	short hdlen;
	int ret;
	unsigned char* data;
	int pcm_type;
	int nsmpl;
	int bps;
	unsigned int nch;
	unsigned int sfreq;

	adxb->hdr_ok = 1;
	if (len < 8) {
		hdlen = 0;
		ret = -1;
	} else {
		data = adxb_DecodeInfoAu(hdr, len, &sfreq, &nch, &bps, &nsmpl, &pcm_type);
		if (data == 0) {
			ret = -1;
		} else {
			hdlen = (short)((int)data - (int)hdr);
			if (hdlen <= 0) {
				ret = -1;
			} else {
				ret = 0;
				adxb->sfreq = sfreq;
				adxb->nch = nch;
				adxb->bps = bps;
				adxb->total_smpl = nsmpl;
				adxb->type = (signed char)0xFF;
				adxb->blk_len = (adxb->nch * adxb->bps) / 8;
				adxb->blk_smpl = 1;
			}
		}
	}
	if (ret < 0) {
		return 0;
	}
	ADXB_SET_PCM_PRM(adxb);
	adxb->format = ADXB_FMT_AU;
	adxb->pcm_type = pcm_type;
	return hdlen;
}

// 0x8021D0A8
static void adxb_ExecOneAu16(ADXB adxb)
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
				dst_l[i] = (src[i * 2] << 8) | (src[i * 2] >> 8);
				dst_r[i] = (src[i * 2 + 1] << 8) | (src[i * 2 + 1] >> 8);
			}
		} else {
			for (i = 0; i < n; i++) {
				dst_l[i] = (src[i] << 8) | (src[i] >> 8);
			}
		}
		adxb->dec_nsmpl = n;
		adxb->dec_dtlen = adxb->nch * (n * 2);
		adxb->stat = ADXB_STAT_DECODE;
	}
	if (adxb->stat == ADXB_STAT_DECODE) {
		adxb->add_wr(adxb->add_wr_obj, adxb->dec_dtlen, adxb->dec_nsmpl);
		adxb->stat = ADXB_STAT_DECEND;
	}
}

// 0x8021CD70
static void adxb_ExecOneAu8(ADXB adxb)
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
				dst_l[i] = (src[i * 2] & 0xFF) << 8;
				dst_r[i] = (src[i * 2 + 1] & 0xFF) << 8;
			}
		} else {
			for (i = 0; i < n; i++) {
				dst_l[i] = (src[i] & 0xFF) << 8;
			}
		}
		adxb->dec_nsmpl = n;
		adxb->dec_dtlen = n * adxb->nch;
		adxb->stat = ADXB_STAT_DECODE;
	}
	if (adxb->stat == ADXB_STAT_DECODE) {
		adxb->add_wr(adxb->add_wr_obj, adxb->dec_dtlen, adxb->dec_nsmpl);
		adxb->stat = ADXB_STAT_DECEND;
	}
}

// 0x8021C9AC
static void adxb_ExecOneAuUlaw(ADXB adxb)
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
				dst_l[i] = adxb_ulaw_tbl[src[i * 2]];
				dst_r[i] = adxb_ulaw_tbl[src[i * 2 + 1]];
			}
		} else {
			for (i = 0; i < n; i++) {
				dst_l[i] = adxb_ulaw_tbl[src[i]];
			}
		}
		adxb->dec_nsmpl = n;
		adxb->dec_dtlen = n * adxb->nch;
		adxb->stat = ADXB_STAT_DECODE;
	}
	if (adxb->stat == ADXB_STAT_DECODE) {
		adxb->add_wr(adxb->add_wr_obj, adxb->dec_dtlen, adxb->dec_nsmpl);
		adxb->stat = ADXB_STAT_DECEND;
	}
}

// 0x8021C968
void ADXB_ExecOneAu(ADXB adxb)
{
	if (adxb->pcm_type == 2) {
		adxb_ExecOneAuUlaw(adxb);
	} else if (adxb->pcm_type == 1) {
		adxb_ExecOneAu8(adxb);
	} else {
		adxb_ExecOneAu16(adxb);
	}
}
