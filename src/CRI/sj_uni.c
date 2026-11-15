#include <CRI/sj.h>
#include <CRI/adx.h>

static char sjuni_obj[0xC00];
static int sjuni_init_cnt;

// 0x8022CF40
void SJUNI_Init(void)
{
	if (sjuni_init_cnt == 0) {
		memset(sjuni_obj, 0, sizeof(sjuni_obj));
	}
	sjuni_init_cnt++;
}

// 0x8022CEF8
void SJUNI_Finish(void)
{
	if (--sjuni_init_cnt == 0) {
		memset(sjuni_obj, 0, sizeof(sjuni_obj));
	}
}
