#include "ui/TAeLayer.h"
#include "engine/MotionPose.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "ui/AeUtil.h"
struct NJS_QUATERNION;

extern "C" {
extern const u32 sAeGaugeBlinkCells[2] = {
	0x000000FC, 0x000000F0,
}; // the two blink cells

int sNodeTrackIndex;          // 0x802C5240 the node being drawn
const int* sNodeTrackList;    // 0x802C5244 the next node wanted
Vec3f* sNodeTrackOut;         // 0x802C5248 where its point goes
int sNodeTrackLast;           // 0x802C524C
int sNodeTrackCount;          // 0x802C5250
}

// 0x800D2EE0
// Five cells one row per tenth of value rounded up
extern "C" void consoleSetGauge(TAeLayer** cells, int value)
{
	int row;
	int i;

	if (cells == 0) {
		return;
	}
	row = (value + 9) / 10;
	if (row > 10) {
		return;
	}
	if (row < 0) {
		return;
	}
	for (i = 0; i < 5; i++) {
		if (sAeGaugeCells[row][i] != cells[i]->getId()) {
			cells[i]->reset(sAeGaugeCells[row][i], 1);
		}
	}
}

// 0x800D2F78
// Flips between two cells every ten counts
extern "C" void consoleSetBlink(TAeLayer* layer, int value)
{
	int tens = value / 10;

	if (tens < 10 && tens >= 0) {
		layer->reset(sAeGaugeBlinkCells[tens % 2], 1);
	}
}

// 0x800D2FD4
// out = a + b - a * t for four floats
extern "C" void lerp4f(f32 t, f32* out, const f32* a, const f32* b)
{
	f32 s = 1.0f - t;

	out[0] = b[0] * t + a[0] * s;
	out[1] = b[1] * t + a[1] * s;
	out[2] = b[2] * t + a[2] * s;
	out[3] = b[3] * t + a[3] * s;
}

// 0x800D3030
// One digit cell value clamped to 0-9 from the cell base
extern "C" void aeClipSetDigit(TAeLayer* layer, int value, int base, u32 slot)
{
	int digit;

	if (layer == 0) {
		return;
	}
	if (value < 0) {
		digit = 0;
	} else {
		digit = value > 9 ? 9 : value;
	}
	digit += base;
	layer->setFrame(slot, digit);
}

// 0x800D3088
// Two digit cells value clamped to 0-99
extern "C" void aeClipSetDigits2(TAeLayer* layer, int value, int base, u32 tens, u32 ones)
{
	int n;

	if (layer == 0) {
		return;
	}
	n = value < 0 ? 0 : (value > 99 ? 99 : value);
	int hi = n / 10;
	int lo = n % 10;
	hi += base;
	lo += base;
	layer->setFrame(tens, hi);
	layer->setFrame(ones, lo);
}

// 0x800D3110
// Three digit cells value clamped to 0-999
extern "C" void aeClipSetDigits3(TAeLayer* layer, int value, int base, u32 hundreds, u32 tens, u32 ones)
{
	int n;

	if (layer == 0) {
		return;
	}
	n = value < 0 ? 0 : (value > 999 ? 999 : value);
	int o;
	int te;
	int h = n / 100;
	o = n % 10;
	te = (n - h * 100) / 10;
	h += base;
	te += base;
	o += base;
	layer->setFrame(hundreds, h);
	layer->setFrame(tens, te);
	layer->setFrame(ones, o);
}

// 0x800D31C4
extern "C" void nodeTrackOriginCb()
{
	nodeTrackOrigins();
	sNodeTrackIndex++;
}

// 0x800D31F0
extern "C" void nodeTrackPointCb()
{
	nodeTrackPoints();
	sNodeTrackIndex++;
}

// 0x800D321C
extern "C" void nodeTrackOrigins()
{
	while (sNodeTrackIndex == *sNodeTrackList) {
		njCalcPointEx(&gVecZero, sNodeTrackOut);
		sNodeTrackOut++;
		sNodeTrackList++;
	}
}

// 0x800D3284
extern "C" void nodeTrackPoints()
{
	while (sNodeTrackIndex == *sNodeTrackList) {
		njCalcPointEx(sNodeTrackOut, sNodeTrackOut);
		sNodeTrackOut++;
		sNodeTrackList++;
	}
}

// 0x800D32DC
extern "C" int nodeTrackIndex()
{
	return sNodeTrackIndex;
}

// 0x800D32E4
extern "C" void nodeTrackNext()
{
	sNodeTrackIndex++;
}

// 0x800D32F4
extern "C" void nodeTrackSet(const int* nodes, Vec3f* out)
{
	sNodeTrackIndex = 0;
	sNodeTrackList  = nodes;
	sNodeTrackOut   = out;
	sNodeTrackLast  = -1;
	sNodeTrackCount = 0;
}

// 0x800D3314
extern "C" void nodeTrackSetCallback(void (*callback)())
{
	gjSetMotionCallback((void (*)(NJS_OBJECT*))callback);
}

// 0x800D3334
extern "C" void nodeTrackBeginOrigins(const int* nodes, Vec3f* out)
{
	nodeTrackSet(nodes, out);
	gjSetMotionCallback((void (*)(NJS_OBJECT*))nodeTrackOriginCb);
}

// 0x800D3360
extern "C" void nodeTrackBeginPoints(const int* nodes, Vec3f* points)
{
	nodeTrackSet(nodes, points);
	gjSetMotionCallback((void (*)(NJS_OBJECT*))nodeTrackPointCb);
}

// 0x800D338C
extern "C" void nodeTrackEnd()
{
	gjSetMotionCallback(0);
}
