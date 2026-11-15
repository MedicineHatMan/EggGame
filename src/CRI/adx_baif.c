#include <CRI/adx.h>

#define ADXB_LE16(p) ((unsigned int)(p)[0] | ((unsigned int)(p)[1] << 8))
#define ADXB_LE32(p) ((unsigned int)(p)[0] | ((unsigned int)(p)[1] << 8) | ((unsigned int)(p)[2] << 16) | ((unsigned int)(p)[3] << 24))
#define ADXB_SWAP32(v) (((v) << 24) | (((v) & 0xFF00) << 8) | (((v) >> 8) & 0xFF00) | ((v) >> 24))
#define ADXB_SWAP16(v) ((((v) >> 8) & 0xFF) | (((v) & 0xFF) << 8))

// 0x8021C6F8
static unsigned char* adxb_DecodeInfoAiff(unsigned char* hdr, int* sfreq, unsigned int* nch, unsigned int* bps,
                                          unsigned int* nsmpl)
{
	int comm;
	int ssnd;
	unsigned char* data;
	unsigned char* p;
	unsigned char* end;
	unsigned int id;
	unsigned int v;
	int csize;

	comm = 0;
	ssnd = 0;
	data = 0;
	v = ADXB_LE32(hdr + 4);
	csize = ADXB_SWAP32(v);
	if (ADXB_LE32(hdr) != 0x4D524F46) {
		return 0;
	}
	if (ADXB_LE32(hdr + 8) != 0x46464941) {
		return 0;
	}
	p = hdr + 0xC;
	end = p + csize - 4;
	while (p < end) {
		id = ADXB_LE32(p);
		v = ADXB_LE32(p + 4);
		csize = ADXB_SWAP32(v);
		p += 8;
		if (id == 0x4D4D4F43) {
			if (comm) {
				continue;
			}
			if (csize < 0x12) {
				return 0;
			}
			comm = 1;
			*nch = ADXB_LE16(p);
			*nch = ADXB_SWAP16(*nch);
			*nsmpl = ADXB_LE32(p + 2);
			*nsmpl = ADXB_SWAP32(*nsmpl);
			*bps = ADXB_LE16(p + 6);
			*bps = ADXB_SWAP16(*bps);
			*sfreq = (unsigned short)ADXB_SWAP16(ADXB_LE16(p + 10)) >> (0x400E - (unsigned short)ADXB_SWAP16(ADXB_LE16(p + 8)));
			p += 0x12;
			if (ssnd) {
				return data;
			}
		} else if (id == 0x444E5353) {
			if (ssnd) {
				continue;
			}
			ssnd = 1;
			v = ADXB_LE32(p);
			p += 4;
			data = p + ADXB_SWAP32(v);
			if (comm) {
				return data;
			}
		} else {
			p += (csize + 1) & ~1;
		}
	}
	return data;
}

// 0x8021C690
int ADXB_CheckAiff(void* hdr)
{
	if (memcmp(hdr, "FORM", 4) == 0 && memcmp((char*)hdr + 8, "AIFF", 4) == 0) {
		return 1;
	}
	return 0;
}

// 0x8021C50C
int ADXB_DecodeHeaderAiff(ADXB adxb, void* hdr, int len)
{
	short hdlen;
	int ret;
	unsigned char* data;
	unsigned int nsmpl;
	unsigned int bps;
	unsigned int nch;
	int sfreq;

	adxb->hdr_ok = 1;
	if (len < 0x1000) {
		hdlen = 0;
		ret = -1;
	} else {
		data = adxb_DecodeInfoAiff(hdr, &sfreq, &nch, &bps, &nsmpl);
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
	adxb->format = ADXB_FMT_AIFF;
	if (adxb->bps == 8) {
		adxb->pcm_type = 1;
	} else {
		adxb->pcm_type = 0;
	}
	return hdlen;
}

// 0x8021C15C
static void adxb_ExecOneAiff16(ADXB adxb)
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

// 0x8021BE24
static void adxb_ExecOneAiff8(ADXB adxb)
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

// 0x8021BDF0
void ADXB_ExecOneAiff(ADXB adxb)
{
	if (adxb->pcm_type == 1) {
		adxb_ExecOneAiff8(adxb);
	} else {
		adxb_ExecOneAiff16(adxb);
	}
}
