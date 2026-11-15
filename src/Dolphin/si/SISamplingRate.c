#include "types.h"
#include "Dolphin/OS.h"
#include "Dolphin/vi.h"
#include "Dolphin/si.h"
#include "Dolphin/hw_regs.h"
#include "Dolphin/sdk_assert.h"

struct tvformat {
	u16 mTvShort;
	u8 mTvByte;
};

static u32 SamplingRate;
static struct tvformat XYNTSC[12] = { 0x00F6, 0x02, 0x000F, 0x12, 0x001E, 0x09, 0x002C, 0x06, 0x0034, 0x05, 0x0041, 0x04,
                                      0x0057, 0x03, 0x0057, 0x03, 0x0057, 0x03, 0x0083, 0x02, 0x0083, 0x02, 0x0083, 0x02 };
static struct tvformat XYPAL[12]  = { 0x0128, 0x02, 0x000F, 0x15, 0x001D, 0x0B, 0x002D, 0x07, 0x0034, 0x06, 0x003F, 0x05,
                                      0x004E, 0x04, 0x0068, 0x03, 0x0068, 0x03, 0x0068, 0x03, 0x0068, 0x03, 0x009C, 0x02 };

/**
 * @brief Sets the controller port sampling rate in milliseconds.
 *
 * All the attached controller devices share the same sampling rate setting.
 * Since the sampling rate is controlled by the Flipper's video clock, the actual sampling rate set differs from the one specified.
 *
 * @param msec The desired sampling rate in milliseconds. If the value is greater than 11, it will be set to 11.
 *
 * @note Address: 0x800F675C
 * @note Size: 0xE4
 */
// 0x8023C5E4
void SISetSamplingRate(u32 msec)
{
	int progressive;
	BOOL enabled;
	struct tvformat* format;

	ASSERTMSGLINE(374, msec <= 11, "SISetSamplingRate(): out of rage (0 <= msec <= 11)");
	if (msec > 11) {
		msec = 11;
	}

	enabled      = OSDisableInterrupts();
	SamplingRate = msec;

	switch (VIGetTvFormat()) {
	case VI_NTSC:
	case VI_MPAL:
	case VI_EURGB60:
		format = XYNTSC;
		break;
	case VI_PAL:
		format = XYPAL;
		break;
	default:
		OSReport("SISetSamplingRate: unknown TV format. Use default.");
		msec   = 0;
		format = XYNTSC;
		break;
	}

	progressive = __VIRegs[VI_CLOCK_SEL] & 1;
	SISetXY((progressive ? 2 : 1) * format[msec].mTvShort, format[msec].mTvByte);
	OSRestoreInterrupts(enabled);
}

/**
 * @note Address: 0x800F6840
 * @note Size: 0x24
 */
void SIRefreshSamplingRate(void)
{
	SISetSamplingRate(SamplingRate);
}

#define LATENCY 8

// Checks the sampling rate tables. Nothing calls it, so the linker strips it and only its strings are
// left in .data; its name is unknown
void __SICheckSamplingRate(void)
{
	u32 msec;
	u32 line;
	u32 count;

	for (msec = 0; msec <= 11; msec++) {
		line  = XYNTSC[msec].mTvShort;
		count = XYNTSC[msec].mTvByte;
		OSReport("%2d[msec]: count %3d, line %3d, last %3d, diff0 %2d.%03d, diff1 %2d.%03d\n", msec, count, line,
		         263 - line * (count - 1), 636 * line / 10000, 636 * line / 10 % 1000, 636 * (263 - line * (count - 1)) / 10000,
		         636 * (263 - line * (count - 1)) / 10 % 1000);
		ASSERTLINE(0, line * (count - 1) + LATENCY < 263);
		ASSERTLINE(0, 636 * line < msec * 10000);
		ASSERTLINE(0, 636 * (263 - line * (count - 1)) < msec * 10000);
	}
	for (msec = 0; msec <= 11; msec++) {
		line  = XYPAL[msec].mTvShort;
		count = XYPAL[msec].mTvByte;
		OSReport("%2d[msec]: count %3d, line %3d, last %3d, diff0 %2d.%03d, diff1 %2d.%03d\n", msec, count, line,
		         313 - line * (count - 1), 640 * line / 10000, 640 * line / 10 % 1000, 640 * (313 - line * (count - 1)) / 10000,
		         640 * (313 - line * (count - 1)) / 10 % 1000);
		ASSERTLINE(0, line * (count - 1) + LATENCY < 313);
		ASSERTLINE(0, 640 * line < msec * 10000);
		ASSERTLINE(0, 640 * (313 - line * (count - 1)) < msec * 10000);
	}
}
