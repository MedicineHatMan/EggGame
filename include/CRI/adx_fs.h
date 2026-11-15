#ifndef CRI_ADX_FS_H
#define CRI_ADX_FS_H

#include <CRI/sj.h>
#include <CRI/adx_stmc.h>

#define ADXF_MAX_OBJ   16
#define ADXF_MAX_PTID  256
#define ADXF_FNAME_LEN 256
#define ADXF_SCT       0x800

#define ADXF_STAT_STOP    1
#define ADXF_STAT_READING 2
#define ADXF_STAT_READEND 3
#define ADXF_STAT_ERROR   4

#define ADXF_ERR_PRM (-3)

typedef struct {
	signed char used;       /* 00 */
	signed char stat;       /* 01 */
	signed char sjflag;     /* 02 set when the stream joint is the caller's */
	signed char stopnw_flg; /* 03 */
	ADXSTM stm;             /* 04 */
	SJ sj;                  /* 08 */
	int fnsct;              /* 0C */
	int fsize;              /* 10 */
	int skpos;              /* 14 */
	int rdstpos;            /* 18 */
	int rqsct;              /* 1C */
	int rdsct;              /* 20 */
	void* buf;              /* 24 */
	int bsize;              /* 28 */
	int rqrdsct;            /* 2C */
	int ofst;               /* 30 */
	void* dir;              /* 34 */
	const char* fname;      /* 38 */
	int ofst2;              /* 3C */
	int nsct;               /* 40 */
} ADX_FS;

typedef ADX_FS* ADXF;

/* One AFS partition where it is and each file's size in sectors 32-bit or 16-bit by the revision */
typedef struct _adxf_ptinfo {
	struct _adxf_ptinfo* next;  /* 00 */
	int size;                   /* 04 */
	int nfile;                  /* 08 */
	unsigned short nentry;      /* 0C */
	unsigned char type;         /* 0E */
	char rev;                   /* 0F */
	char fname[ADXF_FNAME_LEN]; /* 10 */
	void* curdir;               /* 110 */
	int ofst;                   /* 114 */
	union {
		int i[1]; /* 118 [0] is the first file's sector then the sizes */
		unsigned short s[2];
	} sz;
} ADXF_PTINFO;

/* What each history entry records */
#define ADXF_CMD_OPEN      1
#define ADXF_CMD_OPEN_AFS  2
#define ADXF_CMD_CLOSE     3
#define ADXF_CMD_READ_NW32 4
#define ADXF_CMD_STOP      5
#define ADXF_CMD_NUM       16
#define ADXF_HSTRY_NUM     16

typedef struct {
	unsigned char cmdid;  /* 00 */
	unsigned char fg;     /* 01 0 on the way in 1 on the way out */
	unsigned short ncall; /* 02 */
	int prm[3];           /* 04 */
} ADXF_CMD_HSTRY;

void ADXF_Finish(void);
void ADXF_Init(void);
void ADXF_Ocbi(void* buf, int size);
int ADXF_GetFnameRangeEx(int ptid, int flid, char* fname, void** dir, int* ofst, int* fnsct);
void ADXF_ExecServer(void);
int ADXF_ReadNw32(ADXF adxf, int nsct, void* buf);
void ADXF_CloseAll(void);
void ADXF_Close(ADXF adxf);
ADXF ADXF_OpenAfs(int ptid, int flid);
ADXF ADXF_Open(const char* fname, void* dir);
int ADXF_GetPtStat(int ptid);
void ADXF_StopPtLd(void);
void adxf_CloseLdptnwHn(void);
int ADXF_LoadPartitionNw(int ptid, const char* fname, void* dir, ADXF_PTINFO* ptinfo);

extern int adxf_init_cnt;
extern int adxf_afs_ptid;
extern ADXF adxf_afs_hndl;
extern int adxf_ldptnw_nfile;
extern int adxf_hstry_no;
extern unsigned short adxf_cmd_ncall[ADXF_CMD_NUM];
extern ADXF_CMD_HSTRY adxf_cmd_hstry[ADXF_HSTRY_NUM];
extern int adxf_ocbi_fg;
extern ADXF_PTINFO* adxf_ptinfo[ADXF_MAX_PTID];
extern ADX_FS adxf_obj[ADXF_MAX_OBJ];
extern unsigned char* adxf_tmpbuf;
extern int adxf_tbsize_sct;
extern unsigned char adxf_tmpbuf_area[0x2080];

#endif
