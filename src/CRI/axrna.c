#include <sdk_old/types.h> /* the old SDK headers below need its macros */
#include <CRI/sj.h>
#include "Dolphin/ax.h"
#include "Dolphin/mix.h"

#include <CRI/axrna.h>
#include <CRI/adx.h>
#include <CRI/cri_std.h>

#define AXRNA_MAX_OBJ 16
#define AXRNA_MAX_CH 2
#define AXRNA_BUF_SMPL 0x1000
#define AXRNA_FLASH_SIZE 0x1000
#define AXRNA_OWNER_ID 0x80000000

#define AXRNA_FLG_TRANS 1
#define AXRNA_FLG_PLAY 2

typedef struct _adxrna_obj {
	signed char used;              /* 00 */
	unsigned char flags;           /* 01 AXRNA_FLG_* */
	signed char maxnch;            /* 02 */
	signed char nch;               /* 03 */
	int rd_pos;                    /* 04 play position in the ring in samples */
	AXVPB* vpb[AXRNA_MAX_CH];      /* 08 */
	AXRNARES res[AXRNA_MAX_CH];    /* 10 */
	u32 aram_adr[AXRNA_MAX_CH];    /* 18 */
	int buf_len;                   /* 20 */
	int sfreq;                     /* 24 */
	u32 owner[AXRNA_MAX_CH];       /* 28 */
	SJ sji[AXRNA_MAX_CH];          /* 30 */
	SJ sjrbf[AXRNA_MAX_CH];        /* 38 */
	SJCK ck_data[AXRNA_MAX_CH];    /* 40 */
	SJCK ck_room[AXRNA_MAX_CH];    /* 50 */
	int dma_busy[AXRNA_MAX_CH];    /* 60 */
	int dma_smpl;                  /* 68 */
	int dma_total;                 /* 6C */
	int flash_busy[AXRNA_MAX_CH];  /* 70 */
	int flash_smpl;                /* 78 */
	int flash_total;               /* 7C */
	int bps;                       /* 80 */
	int outvol;                    /* 84 */
	int pan[AXRNA_MAX_CH];         /* 88 */
	int span;                      /* 90 */
	int aux_a;                     /* 94 */
	int aux_b;                     /* 98 */
	int fader;                     /* 9C */
	s16 sfreq_fix;                 /* A0 1 when the output runs at 32029 Hz as the AI does at 48 kHz */
	s16 srctype_set;               /* A2 */
	u32 srctype;                   /* A4 */
	ARQRequest req[AXRNA_MAX_CH];  /* A8 */
} AXRNA_OBJ;

typedef char axrna_size_ok[(sizeof(AXRNA_OBJ) == 0xE8) ? 1 : -1];

const char* const volatile axrna_build = "\nAXRNA Ver.1.02 Build:Mar  3 2003 16:47:43\n";
static const char* const axrna_sw_str[] = {"OFF", "ON "};

static u32 axrna_srctype = 1;
/* MIX pan for -15..15 */
static int axrna_pan_tbl[31] = {
	0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x21, 0x25, 0x29, 0x2D, 0x31, 0x35, 0x39, 0x40,
	0x44, 0x48, 0x4C, 0x51, 0x55, 0x59, 0x5D, 0x62, 0x66, 0x6A, 0x6E, 0x73, 0x77, 0x7B, 0x7F,
};

/* Reversed CRI emits uninitialised data in reverse */
static AXRNA_OBJ axrna_obj[AXRNA_MAX_OBJ];
static char axrna_flash_area[AXRNA_FLASH_SIZE + 0x40];
static int axrna_pos_log[32];
static int axrna_pos_idx;
static int axrna_unk;
static int axrna_sfreq_fix;
static char* axrna_flash_buf;
static u32 axrna_init_cnt;

void axrna_call_errfn(const char* msg);
void axrna_entry_errfn(void (*func)(void*, char*), void* obj);
void axrna_lock(void);
void axrna_unlock(void);
void AXRNA_SetPlaySw(AXRNA rna, int sw);
void AXRNA_SetTransSw(AXRNA rna, int sw);
void AXRNA_Destroy(AXRNA rna);
void axrna_dma_cb(u32 req);
void axrna_flash_cb(u32 req);
void axrna_update_pos(AXRNA rna);
void axrna_drop_cb(void* vpb);
void AXRNA_SetOutPan(AXRNA rna, int ch, int pan);
void AXRNA_SetOutVol(AXRNA rna, int vol);
void AXRNA_SetSfreq(AXRNA rna, int sfreq);
void AXRNA_SetNumChan(AXRNA rna, int nch);
void AXRNA_SetBitPerSmpl(AXRNA rna, int bps);
void AXRNA_ExecServer(void);
void axrna_exec_hndl(AXRNA rna);
int AXRNA_GetNumData(AXRNA rna);
int AXRNA_GetNumRoom(AXRNA rna);
static void axrna_init_srctype(AXRNA rna);

#define AXRNA_GET_TRANS_SW(rna) (((rna) == 0) ? -1 : ((rna)->flags & AXRNA_FLG_TRANS))
#define AXRNA_GET_PLAY_SW(rna) (((rna) == 0) ? -1 : (((rna)->flags >> 1) & 1))

// 0x80230230
void AXRNA_Init(void)
{
	axrna_build;
	if (axrna_init_cnt == 0) {
		AXRNARES_Init();
		memset(axrna_obj, 0, sizeof(axrna_obj));
		axrna_flash_buf = (char*)(((u32)axrna_flash_area + 0x1F) & ~0x1F);
	}
	axrna_init_cnt++;
}

// 0x80230110
void AXRNA_Finish(void)
{
	int i;

	if (--axrna_init_cnt != 0) {
		return;
	}
	for (i = 0; i < AXRNA_MAX_OBJ; i++) {
		if (axrna_obj[i].used == 1) {
			AXRNA_Destroy(&axrna_obj[i]);
		}
	}
	memset(axrna_obj, 0, sizeof(axrna_obj));
	AXRNARES_Finish();
}

/* AX calls this when it takes a voice back */
// 0x8023007C
void axrna_drop_cb(void* vpb)
{
	int i;
	int ch;

	for (i = 0; i < AXRNA_MAX_OBJ; i++) {
		for (ch = 0; ch < AXRNA_MAX_CH; ch++) {
			if ((AXVPB*)vpb == axrna_obj[i].vpb[ch]) {
				MIXReleaseChannel(axrna_obj[i].vpb[ch]);
				axrna_obj[i].vpb[ch] = 0;
				return;
			}
		}
	}
}

// 0x8022F990
AXRNA AXRNA_Create(SJ* sj, int maxnch, short* work)
{
	AXRNA rna;
	unsigned int i;
	unsigned int ch;

	if (maxnch <= 0) {
		axrna_call_errfn("E1070301:Illigal parameter(maxnch<=0).\n");
		return 0;
	}
	if (sj == 0) {
		axrna_call_errfn("E1070302:Illigal parameter(sj=null).\n");
		return 0;
	}
	for (i = 0; i < maxnch; i++) {
		if (sj[i] == 0) {
			axrna_call_errfn("E1070303:Illigal parameter(sj[]=null).\n");
			return 0;
		}
	}
	for (i = 0; i < AXRNA_MAX_OBJ; i++) {
		if (axrna_obj[i].used == 0) {
			break;
		}
	}
	if (i == AXRNA_MAX_OBJ) {
		axrna_call_errfn("E1070304:Not enough RNA handle.\n");
		return 0;
	}
	rna = &axrna_obj[i];
	rna->nch = maxnch;
	rna->maxnch = maxnch;
	for (ch = 0; ch < rna->maxnch; ch++) {
		rna->sji[ch] = sj[ch];
	}
	rna->outvol = 0;
	rna->span = 0x7F;
	rna->aux_a = -999;
	rna->aux_b = -999;
	rna->fader = 0;
	for (ch = 0; ch < rna->maxnch; ch++) {
		rna->owner[ch] = ((i << 1) + ch) | AXRNA_OWNER_ID;
		if ((rna->res[ch] = AXRNARES_Create()) == 0) {
			axrna_call_errfn("E1070305:Can't create RNARES.\n");
			AXRNA_Destroy(rna);
			return 0;
		}
		rna->aram_adr[ch] = AXRNARES_GetAdr(rna->res[ch]);
		rna->buf_len = AXRNARES_GetLen(rna->res[ch]);
		rna->sjrbf[ch] = SJRBF_Create((char*)(rna->aram_adr[ch] << 1), rna->buf_len << 1, 0);
		if (rna->sjrbf[ch] == 0) {
			axrna_call_errfn("E1070306:Can't create SJ.\n");
			AXRNA_Destroy(rna);
			return 0;
		}
		if ((rna->vpb[ch] = AXAcquireVoice(31, axrna_drop_cb, 0)) == 0) {
			axrna_call_errfn("E1070307:Can't acquire voice(AX).\n");
			AXRNA_Destroy(rna);
			return 0;
		}
		axrna_lock();
		if (rna->vpb[ch] != 0) {
			MIXInitChannel(rna->vpb[ch], 3, rna->outvol, rna->aux_a, rna->aux_b, 0x40, rna->span, rna->fader);
		}
		axrna_unlock();
	}
	axrna_init_srctype(rna);
	rna->srctype_set = 0;
	AXRNA_SetSfreq(rna, 48000);
	AXRNA_SetBitPerSmpl(rna, 16);
	if (rna->maxnch == 2) {
		AXRNA_SetOutPan(rna, 0, -15);
		AXRNA_SetOutPan(rna, 1, 15);
	} else {
		AXRNA_SetOutPan(rna, 0, 0);
	}
	rna->flags = 0;
	rna->used = 1;
	return rna;
}

/* Copies the defaults into a new handle */
/* Sets the source type the counterpart of AXRNA_SetSfreqFix */
static void axrna_set_srctype(AXRNA rna, u32 type)
{
	if (rna == 0) {
		return;
	}
	rna->srctype = type;
	rna->srctype_set = 1;
}

/* AXRNA_SetSfreqFix with an int the DOL stores the word it loads with no sign extension */
static void axrna_set_sfreq_fix(AXRNA rna, int sw)
{
	if (rna == 0) {
		return;
	}
	rna->sfreq_fix = sw;
}

static void axrna_init_srctype(AXRNA rna)
{
	axrna_set_sfreq_fix(rna, axrna_sfreq_fix);
	axrna_set_srctype(rna, axrna_srctype);
}

// 0x8022F8C0
void AXRNA_Destroy(AXRNA rna)
{
	int ch;

	if (rna == 0) {
		return;
	}
	AXRNA_SetPlaySw(rna, 0);
	AXRNA_SetTransSw(rna, 0);
	for (ch = 0; ch < rna->maxnch; ch++) {
		if (rna->sjrbf[ch] != 0) {
			SJ_Destroy(rna->sjrbf[ch]);
		}
		if (rna->res[ch] != 0) {
			AXRNARES_Destroy(rna->res[ch]);
		}
		axrna_lock();
		if (rna->vpb[ch] != 0) {
			MIXReleaseChannel(rna->vpb[ch]);
			AXFreeVoice(rna->vpb[ch]);
		}
		axrna_unlock();
	}
	memset(rna, 0, sizeof(AXRNA_OBJ));
}

// 0x8022F6C0
void AXRNA_SetTransSw(AXRNA rna, int sw)
{
	int ch;
	int i;
	long wait;

	if (rna == 0) {
		return;
	}
	if (sw == AXRNA_GET_TRANS_SW(rna)) {
		return;
	}
	if (sw == 1) {
		axrna_lock();
		for (ch = 0; ch < rna->nch; ch++) {
			SJ_Reset(rna->sjrbf[ch]);
			memset(&rna->ck_data[ch], 0, sizeof(SJCK));
			memset(&rna->ck_room[ch], 0, sizeof(SJCK));
			memset(&rna->req[ch], 0, sizeof(ARQRequest));
			rna->dma_busy[ch] = 0;
		}
		rna->dma_smpl = 0;
		rna->dma_total = 0;
		rna->flash_smpl = 0;
		rna->flash_total = 0;
		rna->rd_pos = -1;
		rna->flags |= AXRNA_FLG_TRANS;
		axrna_unlock();
	} else if (sw == 0) {
		for (ch = 0; ch < rna->nch; ch++) {
			for (i = 0; i < 200; i++) {
				if (rna->dma_busy[ch] == 0) {
					break;
				}
				for (wait = 0; wait < 100000; wait++) {
				}
			}
			if (i == 200) {
				axrna_call_errfn("E2071701:DMA transfer(data) to A-RAM did not finish.\n");
				return;
			}
			for (i = 0; i < 200; i++) {
				if (rna->flash_busy[ch] == 0) {
					break;
				}
				for (wait = 0; wait < 100000; wait++) {
				}
			}
			if (i == 200) {
				axrna_call_errfn("E2071701:DMA transfer(flash) to A-RAM did not finish.\n");
				return;
			}
		}
		rna->flags &= AXRNA_FLG_PLAY;
	} else {
		axrna_call_errfn("E1070308:Illigal parameter(sw).\n");
	}
}

// 0x8022F520
#pragma opt_common_subs off
void AXRNA_SetPlaySw(AXRNA rna, int sw)
{
	AXPBADDR addr;
	int ch;
	int adr;
	int end;

	if (rna == 0) {
		return;
	}
	if (sw == AXRNA_GET_PLAY_SW(rna)) {
		return;
	}
	axrna_lock();
	if (sw == 1) {
		rna->rd_pos = -1;
		for (ch = 0; ch < rna->nch; ch++) {
			if (rna->vpb[ch] != 0) {
				end = rna->buf_len - 1;
				adr = rna->aram_adr[ch];
				addr.loopFlag = 1;
				addr.format = 10;
				addr.loopAddressHi = adr >> 16;
				addr.loopAddressLo = adr;
				end += adr;
				addr.endAddressHi = end >> 16;
				addr.endAddressLo = end;
				addr.currentAddressHi = adr >> 16;
				addr.currentAddressLo = adr;
				AXSetVoiceAddr(rna->vpb[ch], &addr);
				AXSetVoiceState(rna->vpb[ch], 1);
			}
		}
		rna->flags |= AXRNA_FLG_PLAY;
	} else if (sw == 0) {
		for (ch = 0; ch < rna->nch; ch++) {
			if (rna->vpb[ch] != 0) {
				AXSetVoiceState(rna->vpb[ch], 0);
			}
		}
		for (ch = 0; ch < rna->maxnch; ch++) {
			SJ_Reset(rna->sjrbf[ch]);
		}
		rna->flags &= AXRNA_FLG_TRANS;
	} else {
		axrna_call_errfn("E1070309:Illigal parameter(sw).\n");
	}
	axrna_unlock();
}
#pragma opt_common_subs reset

// 0x8022F4C4
int AXRNA_GetNumData(AXRNA rna)
{
	if (rna == 0) {
		return -1;
	}
	return AXRNA_BUF_SMPL - ((u32)SJ_GetNumData(rna->sjrbf[rna->nch - 1], SJ_LIN_FREE) >> 1);
}

// 0x8022F46C
int AXRNA_GetNumRoom(AXRNA rna)
{
	if (rna == 0) {
		return -1;
	}
	return (u32)SJ_GetNumData(rna->sjrbf[rna->nch - 1], SJ_LIN_FREE) >> 1;
}

/* Reads how far the voices have played and gives those samples back to the ring */
// 0x8022F2F4
void axrna_update_pos(AXRNA rna)
{
	SJCK ck;
	AXVPB* vpb;
	int rd_pos;
	int pos;
	int nsmpl;
	int nsct;
	int ch;
	int len;

	rd_pos = rna->rd_pos;
	vpb = rna->vpb[rna->nch - 1];
	if (vpb == 0) {
		return;
	}
	pos = *(u32*)&vpb->pb.addr.currentAddressHi - rna->aram_adr[rna->nch - 1];
	axrna_pos_log[axrna_pos_idx++] = pos;
	if (axrna_pos_idx == 32) {
		axrna_pos_idx = 0;
	}
	if (pos < 0 || pos > rna->buf_len) {
		for (;;) {
		}
	}
	if (rd_pos == -1) {
		if (pos == 0) {
			nsmpl = 0;
		} else {
			rd_pos = 0;
			rna->rd_pos = 0;
		}
	}
	if (rd_pos != -1) {
		if (pos > rd_pos) {
			nsmpl = pos - rd_pos;
		} else {
			nsmpl = AXRNA_BUF_SMPL - (rd_pos - pos);
		}
	}
	nsct = nsmpl / 0x800;
	nsmpl = nsct * 0x800;
	if (nsmpl <= 0) {
		return;
	}
	len = nsmpl * 2;
	for (ch = 0; ch < rna->nch; ch++) {
		SJ_GetChunk(rna->sjrbf[ch], SJ_LIN_DATA, len, &ck);
		SJ_PutChunk(rna->sjrbf[ch], SJ_LIN_FREE, &ck);
	}
	rna->rd_pos += nsmpl;
	if (rna->rd_pos >= AXRNA_BUF_SMPL) {
		rna->rd_pos -= AXRNA_BUF_SMPL;
	}
}

/* ARQ callback for a data transfer */
// 0x8022F218
void axrna_dma_cb(u32 req)
{
	AXRNA rna;
	int id;
	int ch;

	id = ((ARQRequest*)req)->owner & 0x7FFFFFFF;
	rna = &axrna_obj[id / 2];
	ch = id % 2;
	if (rna->dma_busy[ch] != 1) {
		return;
	}
	SJ_PutChunk(rna->sji[ch], SJ_LIN_FREE, &rna->ck_data[ch]);
	SJ_PutChunk(rna->sjrbf[ch], SJ_LIN_DATA, &rna->ck_room[ch]);
	rna->dma_busy[ch] = 0;
	if (ch == rna->nch - 1) {
		rna->dma_total += rna->dma_smpl;
	}
}

/* ARQ callback for a flash which fills the ring with silence */
// 0x8022F14C
void axrna_flash_cb(u32 req)
{
	AXRNA rna;
	int id;
	int ch;

	id = ((ARQRequest*)req)->owner & 0x7FFFFFFF;
	rna = &axrna_obj[id / 2];
	ch = id % 2;
	if (rna->flash_busy[ch] != 1) {
		return;
	}
	SJ_PutChunk(rna->sjrbf[ch], SJ_LIN_DATA, &rna->ck_room[ch]);
	rna->flash_busy[ch] = 0;
	if (ch == rna->nch - 1) {
		rna->flash_total += rna->flash_smpl;
	}
}

/* Name ours */
// 0x8022EE14
void axrna_exec_hndl(AXRNA rna)
{
	SJCK room;
	SJCK room_rest;
	SJCK data;
	SJCK data_rest;
	int ch;
	int len;
	int n;

	if (rna == 0) {
		return;
	}
	if (AXRNA_GET_PLAY_SW(rna) == 1) {
		axrna_update_pos(rna);
	}
	if (AXRNA_GET_TRANS_SW(rna) == 1) {
		for (ch = 0; ch < rna->nch; ch++) {
			if (rna->vpb[ch] == 0 || rna->dma_busy[ch] != 0) {
				continue;
			}
			SJ_GetChunk(rna->sjrbf[ch], SJ_LIN_FREE, 0x2000, &room);
			SJ_GetChunk(rna->sji[ch], SJ_LIN_DATA, room.len, &data);
			len = (data.len < room.len) ? data.len : room.len;
			n = len / 32;
			len = n * 32;
			SJ_SplitChunk(&room, len, &room, &room_rest);
			SJ_UngetChunk(rna->sjrbf[ch], SJ_LIN_FREE, &room_rest);
			SJ_SplitChunk(&data, len, &data, &data_rest);
			SJ_UngetChunk(rna->sji[ch], SJ_LIN_DATA, &data_rest);
			if (len == 0) {
				return;
			}
			if (data.len != room.len) {
				for (;;) {
				}
			}
			rna->ck_data[ch] = data;
			rna->ck_room[ch] = room;
			rna->dma_smpl = n * 16;
			DCFlushRange(rna->ck_data[ch].data, rna->ck_data[ch].len);
			rna->dma_busy[ch] = 1;
			ARQPostRequest(&rna->req[ch], rna->owner[ch], 0, 1, (u32)data.data, (u32)room.data, len, axrna_dma_cb);
		}
	} else if (AXRNA_GET_PLAY_SW(rna) == 1 && rna->flash_total < rna->buf_len) {
		for (ch = 0; ch < rna->nch; ch++) {
			if (rna->flash_busy[ch] != 0) {
				continue;
			}
			SJ_GetChunk(rna->sjrbf[ch], SJ_LIN_FREE, 0x2000, &room);
			n = room.len / 32;
			len = n * 32;
			SJ_SplitChunk(&room, len, &room, &room_rest);
			SJ_UngetChunk(rna->sjrbf[ch], SJ_LIN_FREE, &room_rest);
			if (len == 0) {
				return;
			}
			rna->ck_room[ch] = room;
			rna->flash_smpl = n * 16;
			DCFlushRange(axrna_flash_buf, AXRNA_FLASH_SIZE);
			rna->flash_busy[ch] = 1;
			ARQPostRequest(&rna->req[ch], rna->owner[ch], 0, 1, (u32)axrna_flash_buf, (u32)room.data, len,
			               axrna_flash_cb);
		}
	}
}

// 0x8022EDB8
void AXRNA_ExecServer(void)
{
	u32 i;
	AXRNA rna;

	for (i = 0; i < AXRNA_MAX_OBJ; i++) {
		rna = &axrna_obj[i];
		if (rna->used == 1) {
			axrna_exec_hndl(rna);
		}
	}
}

// 0x8022EDA8
void AXRNA_SetNumChan(AXRNA rna, int nch)
{
	if (rna == 0) {
		return;
	}
	rna->nch = nch;
}

// 0x8022EC54
void AXRNA_SetSfreq(AXRNA rna, int sfreq)
{
	AXPBSRC src;
	int fixed;
	int ch;

	if (rna == 0) {
		return;
	}
	rna->sfreq = sfreq;
	for (ch = 0; ch < rna->maxnch; ch++) {
		axrna_lock();
		if (rna->vpb[ch] != 0) {
			if (rna->sfreq_fix == 1) {
				if (sfreq == 32000 && rna->srctype_set == 0) {
					axrna_set_srctype(rna, 0);
				}
				/* Computed here the DOL hoists fixed out of the loop but keeps its unsigned divisions inside */
				fixed = (sfreq * 1124 + 1124) / 1125;
				src.ratioHi = (u32)fixed / 32000;
				src.ratioLo = ((u32)fixed << 8) / 125;
			} else {
				src.ratioHi = sfreq / 32000;
				src.ratioLo = ((sfreq << 8) / 125) & 0xFFFF;
			}
			src.currentAddressFrac = 0;
			src.last_samples[0] = 0;
			src.last_samples[1] = 0;
			src.last_samples[2] = 0;
			src.last_samples[3] = 0;
			AXSetVoiceSrcType(rna->vpb[ch], rna->srctype);
			AXSetVoiceSrc(rna->vpb[ch], &src);
		}
		axrna_unlock();
	}
}

// 0x8022EBA8
void AXRNA_SetOutVol(AXRNA rna, int vol)
{
	int ch;

	if (rna == 0) {
		return;
	}
	vol = (vol < 0L) ? vol : 0L;
	vol = (vol > -999L) ? vol : -999L;
	if (vol == rna->outvol) {
		return;
	}
	rna->outvol = vol;
	for (ch = 0; ch < rna->maxnch; ch++) {
		axrna_lock();
		if (rna->vpb[ch] != 0) {
			MIXSetInput(rna->vpb[ch], vol);
		}
		axrna_unlock();
	}
}

// 0x8022EAF4
void AXRNA_SetOutPan(AXRNA rna, int ch, int pan)
{
	if (rna == 0) {
		return;
	}
	if (ch >= rna->maxnch) {
		return;
	}
	pan = (pan < 15) ? pan : 15;
	pan = (pan > -15L) ? pan : -15;
	if (pan == rna->pan[ch]) {
		return;
	}
	rna->pan[ch] = pan;
	axrna_lock();
	if (rna->vpb[ch] != 0) {
		MIXSetPan(rna->vpb[ch], axrna_pan_tbl[pan + 15]);
	}
	axrna_unlock();
}

// 0x8022EAE4
void AXRNA_SetBitPerSmpl(AXRNA rna, int bps)
{
	if (rna == 0) {
		return;
	}
	rna->bps = bps;
}

/* Does nothing on GC */
// 0x8022EADC
int AXRNA_SetStmHdInfo(AXRNA rna, void* info)
{
	return 0;
}

/* Does nothing on GC */
// 0x8022EAD4
int AXRNA_GetPauseTime(AXRNA rna)
{
	return 0;
}

/* Sets the flag AXRNA_SetSfreq reads to correct for the 32029 Hz output */
// 0x8022EAC4
void AXRNA_SetSfreqFix(AXRNA rna, s16 sw)
{
	if (rna == 0) {
		return;
	}
	rna->sfreq_fix = sw;
}

/* axrna_lock axrna_unlock axrna_entry_errfn and axrna_call_errfn are in axrna_err.c */
