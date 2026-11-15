#include <gcax/gcax.h>

GCAX_WORK* gcax_work; /* 32-byte aligned in gcax_buffer */
void* gcax_buffer;    /* the block the work area was cut from */

/* Sets the master attenuation in dB from -96 to 0 */
// 0x80235698
void gcaxSndSetMasterDb(f32 db)
{
	if (db > 0.0f) {
		db = 0.0f;
	}
	if (db < -96.0f) {
		db = -96.0f;
	}
	gcax_work->master_db = db;
	gcaxSndUpdateChans();
}

/* The AX frame callback */
// 0x802356F4
void gcaxSndCallback(void)
{
	if (gcax_work != 0 && gcax_work->nested != 0) {
		gcax_work->nested_count++;
		OSEnableInterrupts();
		if (gcax_work != 0 && gcax_work->initialized != 0) {
			gcaxSndFrame();
			gcax_work->frame_count++;
		}
		OSDisableInterrupts();
		MIXUpdateSettings();
	} else {
		if (gcax_work != 0 && gcax_work->initialized != 0) {
			gcaxSndFrame();
		}
		MIXUpdateSettings();
	}
}

/* Puts an effect on aux bus A 0 or B 1 0 none 1 standard reverb 2 high reverb 3 chorus 4 delay */
// 0x802357A8
void gcaxSndSetAux(int bus, int type)
{
	void (*callback)(void*, void*);
	void* context;

	switch (type) {
	default:
		return;
	case 0:
		callback = 0;
		context = 0;
		break;
	case 1:
		callback = (void (*)(void*, void*))AXFXReverbStdCallback;
		context = &gcax_work->rev_std;
		break;
	case 2:
		callback = (void (*)(void*, void*))AXFXReverbHiCallback;
		context = &gcax_work->rev_hi;
		break;
	case 3:
		callback = (void (*)(void*, void*))AXFXChorusCallback;
		context = &gcax_work->chorus;
		break;
	case 4:
		callback = (void (*)(void*, void*))AXFXDelayCallback;
		context = &gcax_work->delay;
		break;
	}
	switch (bus) {
	case 0:
		AXRegisterAuxACallback(callback, context);
		break;
	case 1:
		AXRegisterAuxBCallback(callback, context);
		break;
	}
}

/* Takes the work area and the sound heap from buf or from the arena when buf is 0 */
// 0x802358B0
void gcaxSndAllocWork(void* buf, u32 size)
{
	BOOL level;
	void* work;
	u32 heap;
	u32 rest;

	level = OSDisableInterrupts();
	if (buf != 0) {
		work = buf;
	} else {
		work = OSAllocFromArenaLo(size, 0x20);
	}
	if (work == 0) {
		OSReport("AX SDLIB: Panic: cannot allocate memory (%d)\n", size);
		for (;;) {
		}
	}
	OSReport("AX SDLIB: allocated (%d)\n", size);
	gcax_buffer = work;
	gcax_work = (GCAX_WORK*)(((u32)work + 0x1F) & ~0x1F);
	gcax_work->initialized = 0;
	gcax_work->nested = 0;
	OSRestoreInterrupts(level);
	memset(gcax_work, 0, sizeof(GCAX_WORK));
	heap = ((u32)gcax_work + sizeof(GCAX_WORK) + 0x1F) & ~0x1F;
	rest = size - (heap - (u32)work);
	gcaxSndHeapInit((void*)heap, rest);

	gcax_work->rev_std.tempDisableFX = 0;
	gcax_work->rev_std.time = 3.0f;
	gcax_work->rev_std.preDelay = 0.1f;
	gcax_work->rev_std.damping = 0.5f;
	gcax_work->rev_std.coloration = 0.5f;
	gcax_work->rev_std.mix = 0.5f;

	gcax_work->rev_hi.tempDisableFX = 0;
	gcax_work->rev_hi.time = 3.0f;
	gcax_work->rev_hi.preDelay = 0.1f;
	gcax_work->rev_hi.damping = 0.5f;
	gcax_work->rev_hi.coloration = 0.5f;
	gcax_work->rev_hi.crosstalk = 0.3f;
	gcax_work->rev_hi.mix = 0.5f;

	gcax_work->chorus.baseDelay = 15;
	gcax_work->chorus.variation = 0;
	gcax_work->chorus.period = 500;

	gcax_work->delay.delay[0] = 500;
	gcax_work->delay.delay[1] = 500;
	gcax_work->delay.delay[2] = 500;
	gcax_work->delay.feedback[0] = 50;
	gcax_work->delay.feedback[1] = 50;
	gcax_work->delay.feedback[2] = 50;
	gcax_work->delay.output[0] = 100;
	gcax_work->delay.output[1] = 100;
	gcax_work->delay.output[2] = 100;
}

// 0x80235B3C
void gcaxSndInitFx(void)
{
	AXFXReverbStdInit(&gcax_work->rev_std);
	AXFXReverbHiInit(&gcax_work->rev_hi);
	AXFXChorusInit(&gcax_work->chorus);
	AXFXDelayInit(&gcax_work->delay);
}

// 0x80235B98
void gcaxSndInitVoices(void)
{
	gcaxVoiceInit();
	gcax_work->rand_seed = 123456;
	AXFXSetHooks(gcaxSndMalloc, gcaxSndFree);
	gcaxSndInitFx();
	gcax_work->rev_std;
}

/* Sets default ARAM bank sizes lets the game's table replace them and makes the banks */
// 0x80235BEC
void gcaxSndInitAram(void)
{
	int i;

	gcax_work->bank_size[0] = 0x7D000;
	gcax_work->bank_size[1] = 460000;
	gcax_work->bank_size[2] = 360000;
	gcax_work->bank_size[3] = 400000;
	gcax_work->bank_size[4] = 360000;
	gcax_work->bank_size[5] = 360000;
	gcax_work->bank_size[6] = 140000;
	gcax_work->bank_size[7] = 100000;
	gcax_work->bank_size[8] = 0x29820;
	gcax_work->bank_size[9] = 200000;
	gcax_work->bank_size[10] = 240000;
	gcax_work->bank_size[11] = 24000;
	gcax_work->bank_size[12] = 400000;
	gcax_work->bank_size[13] = 40000;
	gcaxSndInitParam();
	for (i = 0; i < GCAX_BANK_MAX; i++) {
		if ((u32)gcax_work->bank_size[i] != 0) {
			gcaxNewAramBank(i, (gcax_work->bank_size[i] + 0x1F) & ~0x1F);
		}
	}
	OSReport("AX SDLIB: aramRest=%d\n", gcax_work->aram_rest);
}
