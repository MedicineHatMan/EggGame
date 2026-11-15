#include <Dolphin/ax.h>
#include <Dolphin/axfx.h>
#include "dolphin/__axfx.h"
#define NULL 0

#define ASSERTLINE(...)

void AXFXDelayCallback(struct AXFX_BUFFERUPDATE * bufferUpdate, struct AXFX_DELAY * delay) {
    long l;
    long r;
    long s;
    long * lBuf;
    long * rBuf;
    long * sBuf;
    u32 i;
    long * left;
    long * right;
    long * sur;

    left = bufferUpdate->left;
    right = bufferUpdate->right;
    sur = bufferUpdate->surround;
    lBuf = delay->left + (delay->currentPos[0] * 0xA0);
    rBuf = delay->right + (delay->currentPos[1] * 0xA0);
    sBuf = delay->sur + (delay->currentPos[2] * 0xA0);

    for(i = 0; i < 160; i++) {
        l = *lBuf;
        r = *rBuf;
        s = *sBuf;
        *lBuf++ = *left + ((s32)(l * delay->currentFeedback[0]) >> 7);
        *rBuf++ = *right + ((s32)(r * delay->currentFeedback[1]) >> 7);
        *sBuf++ = *sur + ((s32)(s * delay->currentFeedback[2]) >> 7);
        *left++ = (s32)(l * delay->currentOutput[0]) >> 7;
        *right++ = (s32)(r * delay->currentOutput[1]) >> 7;
        *sur++ = (s32)(s * delay->currentOutput[2]) >> 7;
    }
    delay->currentPos[0] = (s32) ((delay->currentPos[0] + 1) % delay->currentSize[0]);
    delay->currentPos[1] = (s32) ((delay->currentPos[1] + 1) % delay->currentSize[1]);
    delay->currentPos[2] = (s32) ((delay->currentPos[2] + 1) % delay->currentSize[2]);
}

int AXFXDelaySettings(struct AXFX_DELAY * delay) {
    unsigned long i;
    long * l;
    long * r;
    long * s;
    int old;

    // This SDK checks the settings first each delay 10 to 5000 feedback and output up to 100
    if (delay->delay[0] < 10 || delay->delay[0] > 5000 || delay->delay[1] < 10 || delay->delay[1] > 5000
        || delay->delay[2] < 10 || delay->delay[2] > 5000 || delay->feedback[0] > 100 || delay->feedback[1] > 100
        || delay->feedback[2] > 100 || delay->output[0] > 100 || delay->output[1] > 100 || delay->output[2] > 100) {
        return 0;
    }
    AXFXDelayShutdown(delay);
    old = OSDisableInterrupts();

    for(i = 0; i < 3; i++) {
        delay->currentSize[i] = (((delay->delay[i] - 5) << 5) + 0x9F) / 160U;
        delay->currentPos[i] = 0;
        delay->currentFeedback[i] = (delay->feedback[i] << 7) / 100U;
        delay->currentOutput[i] = (delay->output[i] << 7) / 100U;
    }
    delay->left = __AXFXAlloc(delay->currentSize[0] * 0xA0 * 4);
    delay->right = __AXFXAlloc(delay->currentSize[1] * 0xA0 * 4);
    delay->sur = __AXFXAlloc(delay->currentSize[2] * 0xA0 * 4);
    if (delay->left == NULL || delay->right == NULL || delay->sur == NULL) {
        AXFXDelayShutdown(delay);
        return 0;
    }
    l = delay->left;
    r = delay->right;
    s = delay->sur;
    for(i = 0; i < delay->currentSize[0] * 0xA0; i++) {
        *l++ = 0;
    }
    for(i = 0; i < delay->currentSize[1] * 0xA0; i++) {
        *r++ = 0;
    }
    for(i = 0; i < delay->currentSize[2] * 0xA0; i++) {
        *s++ = 0;
    }
    OSRestoreInterrupts(old);
    return 1;
}

int AXFXDelayInit(struct AXFX_DELAY * delay) {
    int old;

    old = OSDisableInterrupts();
    delay->left = NULL;
    delay->right = NULL;
    delay->sur = NULL;
    OSRestoreInterrupts(old);
    AXFXDelaySettings(delay);
}

// 0x8023AB18
int AXFXDelayShutdown(struct AXFX_DELAY * delay) {
    int old;

    old = OSDisableInterrupts();
    if (delay->left) {
        __AXFXFree(delay->left);
    }
    if (delay->right) {
        __AXFXFree(delay->right);
    }
    if (delay->sur) {
        __AXFXFree(delay->sur);
    }
    delay->left = NULL;
    delay->right = NULL;
    delay->sur = NULL;
    OSRestoreInterrupts(old);
    return 1;
}
