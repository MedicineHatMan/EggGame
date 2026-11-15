#ifndef GCAX_H
#define GCAX_H

#include "types.h"
#include "Dolphin/os.h"
#include "Dolphin/ax.h"
#include "Dolphin/mix.h"
#include "Dolphin/axfx.h"
#include "Dolphin/ar.h"

extern void ARQPostRequest(ARQRequest* task, u32 owner, u32 type, u32 priority, u32 source, u32 dest, u32 length,
                           void (*callback)(u32));
extern u32 ARAlloc(u32 length);

#define GCAX_ARQ_MAX 64
#define GCAX_BANK_MAX 16
#define GCAX_NOTE_MAX 64
#define GCAX_PORT_MAX 8
#define GCAX_MSG_MAX 0x400
#define GCAX_CHAN_MAX 16
#define GCAX_VOICE_MAX 64
#define GCAX_PORT_USED 0x80000000
#define GCAX_PORT_KILL 0x40000000 /* note off frees the note at once */
#define GCAX_CHAN_SUSTAIN 0x00000001

/* Voice flags */
#define GCAX_VOICE_USED 0x80000000    /* taken by a note */
#define GCAX_VOICE_DROPPED 0x40000000 /* AX took the voice back */
#define GCAX_VOICE_HOLD 0x00000001    /* envelope stopped */
#define GCAX_VOICE_PLAYING 0x00000002

/* Bank tables pack a count in the top byte and an offset from the bank start in the rest */
#define GCAX_COUNT(x) ((x) >> 24)
#define GCAX_OFFSET(x) ((x) & 0xFFFFFF)

#define GCAX_MAGIC 0x67636178     /* "gcax" */
#define GCAX_TYPE_MPB 0x4D504220  /* "MPB " */
#define GCAX_TYPE_MPBW 0x4D504257 /* "MPBW" wave data */
#define GCAX_TYPE_MPBP 0x4D504250 /* "MPBP" parameters */
#define GCAX_TYPE_MSB 0x4D534220  /* "MSB " */

typedef void (*GCAX_CALLBACK)(void* arg);

/* A heap block header */
typedef struct GCAX_MEMBLK {
	u16 magic;                 /* 00 "MF" free or "MB" busy */
	u16 pad;                   /* 02 */
	struct GCAX_MEMBLK* next;  /* 04 */
	u8 _08[0x18];
} GCAX_MEMBLK;

/* A chunk header in a bank file */
typedef struct {
	u32 magic; /* 00 */
	u32 type;  /* 04 */
	u32 _08;   /* 08 */
	u32 size;  /* 0C */
} GCAX_CHUNK;

typedef struct {
	union {
		u32 adr;
		void* ptr;
	};
	u32 size;
} GCAX_BANK;

typedef struct GCAX_NODE {
	struct GCAX_NODE* next;
} GCAX_NODE;

/* A list node pointing at a note or a voice */
typedef struct GCAX_LINK {
	struct GCAX_LINK* next;
	void* obj;
} GCAX_LINK;

typedef struct {
	f32 value;
	f32 bound;
} GCAX_LUT;

struct GCAX_PORT;

typedef struct {
	u8 active;               /* 00 */
	u8 nvoice;               /* 01 */
	u8 chan;                 /* 02 */
	u8 key;                  /* 03 */
	u8 vel;                  /* 04 */
	int prio;                /* 08 */
	struct GCAX_PORT* port;  /* 0C */
	GCAX_LINK* voices;       /* 10 */
} GCAX_NOTE;

/* Error codes the API returns */
#define GCAX_OK 0
#define GCAX_ERR_PARAM 0x3000001
#define GCAX_ERR_NO_HANDLE 0x7000101
#define GCAX_ERR_BAD_HANDLE 0x7000102
#define GCAX_ERR_NULL 0x7000103
#define GCAX_ERR_DATA 0x9000001
#define GCAX_ERR_LOAD 0x9000101
#define GCAX_ERR_NOT_INIT 0xD000001

#define GCAX_LOADER_MAX 64
#define GCAX_TYPE_MLT 0x4D4C5420  /* "MLT " a bundle of banks */
#define GCAX_TYPE_MLTM 0x4D4C544D /* "MLTM" its bank table */

/* A bank bundle the game loads through the API */
typedef struct {
	u8 used;           /* 00 */
	s8 arq;            /* 01 ARQ slot to wait for or -1 */
	void* data;        /* 04 */
	u32 size;          /* 08 */
	GCAX_CALLBACK cb;  /* 0C */
	void* arg;         /* 10 */
} GCAX_LOADER;

/* A port's MIDI sequence player */
typedef struct {
	u8 _00[4];
	int speed;        /* 04 */
	f32 next;         /* 08 tick of the next event */
	f32 prev_next;    /* 0C */
	f32 tick;         /* 10 */
	f32 prev_tick;    /* 14 */
	f32 tempo;        /* 18 beats per minute */
	f32 loop_tempo;   /* 1C */
	u8 _20[8];
	OSTime time;      /* 28 */
	u8 status;        /* 30 running status */
	f32 beat;         /* 34 seconds per beat */
	f32 delta;        /* 38 */
	u32 prio;         /* 3C */
	u8 _40[4];
	u8* ptr;          /* 44 */
	u8* loop_ptr;     /* 48 */
	f32 loop_next;    /* 4C */
	f32 loop_tick;    /* 50 */
	int loop_count;   /* 54 */
} GCAX_SEQ;

typedef struct {
	int flags; /* 0 */
	int index; /* 4 */
	u32 bank; /* 8 */
	u32 prog; /* C */
	int vol; /* 10 */
	int expr; /* 14 */
	int pan; /* 18 */
	int bend_raw; /* 1C */
	int dpl_front; /* 20 */
	int dpl_pan; /* 24 */
	int _28; /* 28 */
	int bend; /* 2C */
	f32 out_r; /* 30 */
	f32 out_l; /* 34 */
	f32 tune; /* 38 */
	int vel_bias; /* 3C */
	u32 _40; /* 40 */
	f32 _44; /* 44 */
	f32 _48; /* 48 */
	u8 _4C[0x4];
} GCAX_CHAN;

typedef struct GCAX_PORT {
	int flags; /* 0 */
	int seq_flags; /* 4 */
	f32 vol; /* 8 */
	f32 _0C; /* C */
	f32 _10; /* 10 */
	f32 vol_target; /* 14 */
	f32 vol_start; /* 18 */
	f32 vol_time; /* 1C */
	f32 vol_rest; /* 20 */
	f32 pitch; /* 24 */
	f32 _28; /* 28 */
	f32 pitch_target; /* 2C */
	f32 pitch_start; /* 30 */
	f32 pitch_time; /* 34 */
	f32 pitch_rest; /* 38 */
	f32 fx_send; /* 3C */
	u8 _40[0x8];
	GCAX_SEQ seq; /* 48 */
	GCAX_LINK* notes; /* A0 */
	GCAX_CHAN chan[GCAX_CHAN_MAX]; /* A4 */
	u32 msg[GCAX_MSG_MAX]; /* 5A4 */
	int msg_head; /* 15A4 */
	int msg_tail; /* 15A8 */
	int msg_count; /* 15AC */
} GCAX_PORT;

/* A wave header in an MPB parameter bank */
typedef struct {
	u32 length;           /* 00 */
	u8 _04[8];
	u16 loop;             /* 0C */
	u32 loop_adr;         /* 10 */
	u32 end_adr;          /* 14 */
	u32 cur_adr;          /* 18 */
	u16 coef[16];         /* 1C */
	u16 gain;             /* 3C */
	u16 pred_scale;       /* 3E */
	u16 yn1;              /* 40 */
	u16 yn2;              /* 42 */
	u16 loop_pred_scale;  /* 44 */
	u16 loop_yn1;         /* 46 */
	u16 loop_yn2;         /* 48 */
	u8 format;            /* 4A AX sample format */
	u32 offset;           /* 4C start in the bank's wave data */
} GCAX_SAMPLE;

/* A key and velocity range of a layer */
typedef struct GCAX_REGION {
	u32 sample;     /* 00 */
	u8 _04;
	u8 velcurve;    /* 05 */
	u8 vel_lo;      /* 06 */
	u8 vel_hi;      /* 07 */
	u8 key_lo;      /* 08 */
	u8 key_hi;      /* 09 */
	u8 level;       /* 0A */
	u8 pan;         /* 0B */
	u8 root;        /* 0C */
	u8 fine;        /* 0D */
	u8 fx_invert;   /* 0E */
	u8 fx_send;     /* 0F */
	u8 atk;         /* 10 */
	u8 dec;         /* 11 */
	u8 sus;         /* 12 */
	u8 sus_level;   /* 13 */
	u8 rel;         /* 14 */
	u8 rate_bias;   /* 15 */
	u8 atten;       /* 16 */
	u8 _17[0x29];
} GCAX_REGION;

typedef struct GCAX_LAYER {
	u32 regions;   /* 00 count and offset */
	u8 bend_up;    /* 04 */
	u8 bend_down;  /* 05 */
	u8 _06[0xA];
} GCAX_LAYER;

/* The head of an MPB parameter bank */
typedef struct GCAX_MPB {
	u32 samples;   /* 00 offset */
	u32 velcurves; /* 04 offset */
	u32 progs;     /* 08 count and offset of a table of layer lists */
} GCAX_MPB;

typedef struct GCAX_VOICE {
	int flags; /* 0 */
	int index; /* 4 */
	int prio; /* 8 */
	int arg; /* C */
	f32 vol; /* 10 */
	f32 ratio; /* 14 */
	int bend_up; /* 18 */
	int bend_down; /* 1C */
	int level; /* 20 */
	int pan; /* 24 */
	int atten; /* 28 */
	f32 pan_l; /* 2C */
	f32 pan_r; /* 30 */
	AXVPB* vpb; /* 34 */
	AXPBADDR addr; /* 38 */
	AXPBSRC src; /* 48 */
	AXPBADPCM adpcm; /* 56 */
	AXPBADPCMLOOP adpcm_loop; /* 7E */
	AXPBMIX mix; /* 84 */
	f32 time; /* A8 */
	f32 env; /* AC */
	f32 env_db; /* B0 */
	f32 atk_time; /* B4 */
	f32 dec_time; /* B8 */
	f32 sus_time; /* BC */
	f32 rel_time; /* C0 */
	f32 sus_db; /* C4 */
	f32 atk_rate; /* C8 */
	f32 dec_rate; /* CC */
	f32 sus_rate; /* D0 */
	f32 rel_rate; /* D4 */
	int env_phase; /* D8 */
	int fx_invert; /* DC */
	int fx_send; /* E0 */
	int bank; /* E4 */
	struct GCAX_LAYER* layer; /* E8 */
	struct GCAX_MPB* mpb; /* EC */
	struct GCAX_REGION* region; /* F0 */
	int key; /* F4 */
	u8* veltab; /* F8 */
	int vel; /* FC */
	int velocity; /* 100 */
	int _104; /* 104 */
	GCAX_NOTE* note; /* 108 */
} GCAX_VOICE;

typedef struct {
	int initialized; /* 0 */
	u32 size; /* 4 */
	int busy; /* 8 */
	int seq_dt; /* C */
	OSTime time; /* 10 */
	OSTime prev_time; /* 18 */
	int voices_used; /* 20 */
	int notes_used; /* 24 */
	f32 master_db; /* 28 */
	u8 _2C[0x8];
	int max_voice; /* 34 */
	int voice_count; /* 38 */
	int frame_count; /* 3C */
	int _40; /* 40 */
	u8 _44[0x4];
	int note_rover; /* 48 */
	int voice_rover; /* 4C */
	GCAX_VOICE voice[GCAX_VOICE_MAX]; /* 50 */
	GCAX_NOTE note[GCAX_NOTE_MAX]; /* 4350 */
	GCAX_MEMBLK* heap_top; /* 4850 */
	u32 heap_size; /* 4854 */
	ARQRequest arq[GCAX_ARQ_MAX]; /* 4858 */
	u8 arq_busy[GCAX_ARQ_MAX]; /* 5058 */
	u32 arq_owner; /* 5098 */
	GCAX_CALLBACK arq_cb[GCAX_ARQ_MAX]; /* 509C */
	void* arq_arg[GCAX_ARQ_MAX]; /* 519C */
	u8 _529C[0x4];
	u32 aram_size; /* 52A0 */
	int aram_rest; /* 52A4 */
	u32 aram_next; /* 52A8 */
	u32 aram_base; /* 52AC */
	u32 zero_adr; /* 52B0 */
	int bank_size[GCAX_BANK_MAX]; /* 52B4 */
	GCAX_BANK aram_bank[GCAX_BANK_MAX]; /* 52F4 */
	GCAX_BANK mpbp[GCAX_BANK_MAX]; /* 5374 */
	GCAX_BANK msb[GCAX_BANK_MAX]; /* 53F4 */
	u8 _5474[0x4];
	GCAX_PORT port[GCAX_PORT_MAX]; /* 5478 */
	u8 _101F8[0x2];
	u8 mono; /* 101FA */
	u8 surround; /* 101FB */
	void* find_obj; /* 101FC */
	u32 debug; /* 10200 bit 0 is tested where a debug report was compiled out */
	AXFX_REVERBSTD rev_std; /* 10204 */
	AXFX_REVERBHI rev_hi; /* 10358 */
	AXFX_CHORUS chorus; /* 10538 */
	AXFX_DELAY delay; /* 105D4 */
	u8 _10634[0x3804];
	u8 nested; /* 13E38 */
	u8 nested_count; /* 13E39 */
	u8 _13E3A[0x1];
	u8 overruns; /* 13E3B */
	u8 _13E3C[0x4];
	u8 send_to_aram; /* 13E40 */
	u8 send_state; /* 13E41 */
	u8 _13E42[0x2];
	u8 trace_state; /* 13E44 */
	u8 _13E45[0xB];
	u32 send_total; /* 13E50 */
	u8 _13E54[0x4];
	int send_bank_rest; /* 13E58 */
	int send_chunk_rest; /* 13E5C */
	u8 _13E60[0x4];
	int send_len; /* 13E64 */
	u32 send_dst; /* 13E68 */
	int send_n; /* 13E6C */
	int send_slot; /* 13E70 */
	u32 rand_seed; /* 13E74 */
	u8 _13E78[0x4];
	f32 tempo_scale; /* 13E7C */
	u8 _13E80[0x4];
	u8 zero_buf[0x520]; /* 13E84 */
	GCAX_LOADER loader[GCAX_LOADER_MAX]; /* 143A4 */
	u8 load_nowait; /* 148A4 */
	u8 _148A5[0x3];
} GCAX_WORK;

extern GCAX_WORK* gcax_work;

/* gcaxSndMemory.c */
void gcaxSndHeapReset(void);
void gcaxSndHeapInit(void* base, u32 size);
void* gcaxSndMalloc(u32 size);
GCAX_MEMBLK* gcaxSndHeapPrev(GCAX_MEMBLK* blk);
void gcaxSndFree(void* p);
u32 gcaxSndHeapRest(void);
void gcaxArqCallback(u32 req);
int gcaxArqPost(u32 src, u32 dst, u32 len, GCAX_CALLBACK cb, void* arg);
void gcaxArqPostWait(u32 src, u32 dst, u32 len, GCAX_CALLBACK cb, void* arg);
int gcaxArqPostEx(u32 src, u32 dst, u32 len, GCAX_CALLBACK cb, void* arg, int nowait);
int gcaxNewAramBank(int bank, int size);
int gcaxSndLoadMpb(int bank, GCAX_CHUNK* mpb, GCAX_CALLBACK cb, void* arg, int nowait);
int gcaxSndLoadMsb(int bank, GCAX_CHUNK* msb, GCAX_CALLBACK cb, void* arg, int call);
void gcaxSndTrace(const char* label);
int gcaxSndSendBank(int bank, void* data, u32 size);
GCAX_NODE* gcaxListNew(u32 size, GCAX_NODE** list, void (*init)(GCAX_NODE*));
void gcaxListDelete(GCAX_NODE* node, GCAX_NODE** list, void (*fini)(GCAX_NODE*));
void gcaxListClear(GCAX_NODE** list, void (*fini)(GCAX_NODE*));
GCAX_NODE* gcaxListFind(GCAX_NODE** list, int (*test)(GCAX_NODE*));
GCAX_NODE* gcaxListNext(GCAX_NODE** list, GCAX_NODE* node);

/* gcaxSndNote.c */
f32 gcaxLookup(f32 x);
void gcaxSndInitParam(void);
u32 gcaxRand(void);
GCAX_NOTE* gcaxNoteAlloc(void);
GCAX_NOTE* gcaxNoteSteal(int prio);
GCAX_NOTE* gcaxNoteStealVoices(int prio, int need);
void gcaxNoteFree(GCAX_NOTE* note);
void gcaxNoteRelease(GCAX_NOTE* note);
int gcaxNoteCount(void);
GCAX_NOTE* gcaxNoteOn(void* port, int ch, int key, int vel, int prio);
GCAX_LINK* gcaxNoteListAdd(GCAX_LINK** list, GCAX_NOTE* note);
GCAX_LINK* gcaxNoteListNext(GCAX_LINK** list, GCAX_LINK* link);
void gcaxNoteListFini(GCAX_LINK* link);
void gcaxNoteListDelete(GCAX_LINK** list, GCAX_LINK* link);
void gcaxNoteListClear(GCAX_LINK** list);
int gcaxNoteListTest(GCAX_LINK* link);
GCAX_LINK* gcaxNoteListFind(GCAX_LINK** list, GCAX_NOTE* note);
int gcaxNoteCountVoices(GCAX_NOTE* note);
void gcaxNoteListUpdate(GCAX_LINK** list);

/* gcaxSndVoice.c */
void gcaxVoiceDropCallback(void* vpb);
void gcaxVoiceInit(void);
GCAX_VOICE* gcaxVoiceGet(void);
void gcaxVoiceFree(GCAX_VOICE* voice);
GCAX_VOICE* gcaxVoiceAlloc(GCAX_NOTE* note, int prio, int arg);
int gcaxVoiceCountUsed(void);
int gcaxFreqToCode(int freq, int* oct, int* frac);
f32 gcaxKeyRatio(int root, int fine, int key);
u32 gcaxBendSrcRatio(f32 ratio, int bend);
f32 gcaxCentToRatio(int cent);
void gcaxVoiceStart(int bank, GCAX_VOICE* voice, GCAX_LAYER* layer, GCAX_MPB* mpb, GCAX_REGION* region, int key,
                    u8* veltab, int vel);
int gcaxSndCountVoices(u32 bank, u32 prog, int key);
void gcaxSndStartVoices(GCAX_LINK** list, u32 bank, u32 prog, int key, int vel);
void gcaxVoiceSetMix(GCAX_VOICE* voice, void* handle);
void gcaxVoiceSetPitch(GCAX_VOICE* voice, int bend, f32 scale);
GCAX_LINK* gcaxVoiceListAdd(GCAX_LINK** list, GCAX_VOICE* voice);
GCAX_LINK* gcaxVoiceListNext(GCAX_LINK** list, GCAX_LINK* link);
void gcaxVoiceListFini(GCAX_LINK* link);
void gcaxVoiceListClear(GCAX_LINK** list);
void gcaxVoiceEnvelope(GCAX_VOICE* voice);

/* gcaxSndPort.c */
void gcaxChanReset(GCAX_PORT* port, int ch);
GCAX_PORT* gcaxPortAlloc(void);
void gcaxPortFree(GCAX_PORT* port);
void gcaxPortPost(GCAX_PORT* port, u32 msg);
void gcaxPortProcess(GCAX_PORT* port);
void gcaxPortNoteOff(GCAX_PORT* port, u8 ch, u8 key, u8 vel);
void gcaxPortNoteOn(GCAX_PORT* port, u8 ch, u8 key, u8 vel, int prio);
void gcaxPortControl(GCAX_PORT* port, u8 ch, u8 cc, u8 val);
void gcaxPortProgram(GCAX_PORT* port, u8 ch, u8 prog);
void gcaxPortBend(GCAX_PORT* port, u8 ch, u16 bend);
void gcaxChanUpdate(GCAX_PORT* port, int ch);
void gcaxChanSetBend(GCAX_PORT* port, int ch);
void gcaxPortUpdateChans(GCAX_PORT* port);
void gcaxPortUpdateVoices(GCAX_PORT* port);
void gcaxSndUpdateChans(void);
void gcaxPortVolFadeSet(GCAX_PORT* port);
void gcaxPortVolFade(GCAX_PORT* port);
void gcaxPortPitchFadeSet(GCAX_PORT* port);
void gcaxPortPitchFade(GCAX_PORT* port);
void gcaxChanRandomize(GCAX_PORT* port, u8 ch, u8 key, u8 vel, int prio);
void gcaxSndFrame(void);
void gcaxSndSetMono(int mono);
void gcaxSndSetSurround(int surround);

/* gcaxSndSys.c */
extern void* gcax_buffer;
extern u8 gcax_vol_db[0x100];
extern u8 gcax_pan_map[0x20];
extern u8 gcax_pan_idx[0x80];
extern u8 gcax_level_db[0x40];
extern f32 gcax_pan_gain[0x20];
extern f32 gcax_atk_time[0x40];
extern f32 gcax_dec_time[0x40];
extern f32 gcax_dpl_gain[0x80];
void gcaxSndSetMasterDb(f32 db);
void gcaxSndCallback(void);
void gcaxSndSetAux(int bus, int type);
void gcaxSndAllocWork(void* buf, u32 size);
void gcaxSndInitFx(void);
void gcaxSndInitVoices(void);
void gcaxSndInitAram(void);

/* gcaxSndApi.c */
int gcaxSndInitEx(void* buf, int max_voice, u32 size, u32 aram_size);
int gcaxSndInit(void* buf);
int gcaxSndSetOutput(int mode);
int gcaxSndSetMasterLevel(s8 level);
int gcaxLoaderAlloc(GCAX_LOADER** out);
int gcaxLoaderCheck(GCAX_LOADER* loader);
int gcaxLoaderSet(int handle, void* data, u32 size, GCAX_CALLBACK cb, void* arg);
int gcaxLoaderFree(int handle);
int gcaxLoaderLoad(int handle);
int gcaxPortCheck(GCAX_PORT* port);
int gcaxSndOpenPort(GCAX_PORT** out);
int gcaxSndClosePort(u32 handle);
int gcaxSndMakeMsg(u32 handle, u8* msg, int status, ...);
int gcaxSndSendMsg(u32 handle, u8* msg, u8 prio);
int gcaxSndReverbOn(void);
int gcaxSndReverbOff(void);
int gcaxSndResetPort(u32 handle);
int gcaxSndResetAll(void);

/* gcaxSndSeq.c */
int gcaxSeqReadVar(u8** pp);
void gcaxSeqUpdate(GCAX_PORT* port);

void* memset(void* dst, int c, u32 n);
void* memcpy(void*, const void*, u32);
void* OSAllocFromArenaLo(u32 size, u32 align);
void AXFXSetHooks(void* (*alloc)(u32), void (*free)(void*));
double log10(double x);
double pow(double x, double y);

#endif
