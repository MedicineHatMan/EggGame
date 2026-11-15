#include "Dolphin/ax.h"
#include "Dolphin/__ax.h"
#include "Dolphin/OS/OSThread.h"

static s16 __AXOutBuffer[3][320];
static s32 __AXOutSBuffer[160];
static u16 ax_dram_image[8192];
static DSPTaskInfo task;
static AXPROFILE __AXLocalProfile;

volatile static unsigned long __AXOutFrame;
volatile static unsigned long __AXAiDmaFrame;
volatile static unsigned long __AXOutDspReady;
volatile static long long __AXOsTime;
static void (* __AXUserFrameCallback)();
volatile static int __AXDSPInitFlag;
static int __AXDSPDoneFlag;
volatile static u32 __AXDebugSteppingMode;
static OSThreadQueue __AXOutThreadQueue;
static u32 __AXOutputBufferMode;

inline void memcpy(void *dst, const void *src, int count) {
  while (count--) {
    *((char *)dst)++ = *((const char *)src)++;
  }
}

#define BZERO_28(buffer, size)                \
    {                                         \
        p = (u32*)buffer;                     \
        for (i = 0; i < (size) / 0x28; i++) { \
            for (j = 0; j < 5; j++) {         \
                p[0] = 0;                     \
                p++;                          \
                p[0] = 0;                     \
                p++;                          \
            }                                 \
        }                                     \
    }

// functions
static void __AXDSPInitCallback(void *task);
static void __AXDSPResumeCallback(void *task);
static void __AXDSPDoneCallback(void *task);

// 0x801E68DC
void __AXOutNewFrame(u32 lessDspCycles) {
    u32 cl;
    // int old;
    AXPROFILE * profile;

    __AXLocalProfile.axFrameStart = OSGetTime();
    __AXSyncPBs(lessDspCycles);
    __AXPrintStudio();
    cl = __AXGetCommandListAddress();
    DSPSendMailToDSP(0xBABE0180);
    do {} while (DSPCheckMailToDSP() != 0U);
    DSPSendMailToDSP(cl);
    do {} while (DSPCheckMailToDSP() != 0U);
    // old = OSEnableInterrupts();
    __AXServiceCallbackStack();
    __AXLocalProfile.auxProcessingStart = OSGetTime();
    __AXProcessAux();
    __AXLocalProfile.auxProcessingEnd = OSGetTime();
    __AXLocalProfile.userCallbackStart = OSGetTime();
    if (__AXUserFrameCallback) {
        __AXUserFrameCallback();
    }
    __AXLocalProfile.userCallbackEnd = OSGetTime();
    __AXNextFrame(__AXOutSBuffer, &__AXOutBuffer[__AXOutFrame][0]);
    __AXOutFrame += 1;
    if (__AXOutputBufferMode == 1) {
        __AXOutFrame %= 3;
    } else {
        __AXOutFrame &= 1;
        AIInitDMA((u32)&__AXOutBuffer[__AXOutFrame][0], 0x280);
    }
    __AXLocalProfile.axFrameEnd = OSGetTime();
    __AXLocalProfile.axNumVoices = __AXGetNumVoices();
    profile = (void*)__AXGetCurrentProfile();
    
    if (profile) {
        memcpy(profile, &__AXLocalProfile, sizeof(AXPROFILE));
    }
    // OSRestoreInterrupts(old);
}

// 0x801E6A94
void __AXOutAiCallback(void) {
    if (__AXOutDspReady == 0) {
        __AXOsTime = OSGetTime();
    }
    if (__AXOutDspReady == 1) {
        __AXOutDspReady = 0;
        __AXOutNewFrame(0);
    } else {
        __AXOutDspReady = 2;
        DSPAssertTask(&task);
    }
    if (__AXOutputBufferMode == 1) {
        AIInitDMA((u32)&__AXOutBuffer[__AXAiDmaFrame][0], 0x280);
        __AXAiDmaFrame++;
        __AXAiDmaFrame %= 3;
    }
}

static void __AXDSPInitCallback(void *task) {
    __AXDSPInitFlag = 1;
}

static void __AXDSPResumeCallback(void *task) {
    if (__AXOutDspReady == 2) {
        __AXOutDspReady = 0;
        __AXOutNewFrame((u32)(OSGetTime() - __AXOsTime) / 4);
        return;
    }
    __AXOutDspReady = 1U;
}

// 0x801E6BB4
static void __AXDSPDoneCallback(void *task) {
    __AXDSPDoneFlag = 1;
    OSWakeupThread(&__AXOutThreadQueue);
}

// 0x801E6BE0
void __AXOutInitDSP(void) {
    task.iram_mmem_addr = axDspSlave;
    task.iram_length = axDspSlaveLength;
    task.iram_addr = 0;
    task.dram_mmem_addr = ax_dram_image;
    task.dram_length = 0x2000;
    task.dram_addr = 0;
    task.dsp_init_vector = 0x10;
    task.dsp_resume_vector = 0x30;
    task.init_cb = __AXDSPInitCallback;
    task.res_cb = __AXDSPResumeCallback;
    task.done_cb = __AXDSPDoneCallback;
    task.req_cb = NULL;
    task.priority = 0;
    __AXDSPInitFlag = 0;
    __AXDSPDoneFlag = 0;
    OSInitThreadQueue(&__AXOutThreadQueue);
    if (DSPCheckInit() == 0) {
        DSPInit();
    }
    DSPAddTask(&task);
    do {} while (__AXDSPInitFlag == 0);
}

// 0x801E6CAC
// The 2003 SDK's form mode 1 keeps three buffers
void __AXOutInit(u32 outputBufferMode) {
    u32* p;
    int i;
    int j;
#ifdef DEBUG
    OSReport("Initializing AXOut code module\n");
#endif
    // ASSERTLINE(0x13B, ((u32)&__AXOutBuffer[0][0] & 0x1F) == 0);
    // ASSERTLINE(0x13C, ((u32)&__AXOutBuffer[1][0] & 0x1F) == 0);
    // ASSERTLINE(0x13D, ((u32)&__AXOutSBuffer[0] & 0x1F) == 0);
    __AXOutFrame = 0;
    __AXAiDmaFrame = 0;
    __AXOutputBufferMode = outputBufferMode;
    __AXDebugSteppingMode = 0;
    BZERO_28(__AXOutBuffer, sizeof(__AXOutBuffer));
    DCFlushRange(__AXOutBuffer, sizeof(__AXOutBuffer));
    BZERO_28(__AXOutSBuffer, sizeof(__AXOutSBuffer));
    DCFlushRange(__AXOutSBuffer, sizeof(__AXOutSBuffer));
    __AXOutInitDSP();
    AIRegisterDMACallback(__AXOutAiCallback);
    if (__AXOutputBufferMode == 1) {
        __AXNextFrame(__AXOutSBuffer, &__AXOutBuffer[2][0]);
    } else {
        __AXNextFrame(__AXOutSBuffer, &__AXOutBuffer[1][0]);
    }
    __AXOutDspReady = 1;
    __AXUserFrameCallback = NULL;
    if (__AXOutputBufferMode == 1) {
        AIInitDMA((u32)&__AXOutBuffer[__AXAiDmaFrame][0], sizeof(__AXOutBuffer[0]));
        __AXAiDmaFrame++;
        __AXAiDmaFrame &= 1;
    } else {
        AIInitDMA((u32)&__AXOutBuffer[__AXOutFrame][0], sizeof(__AXOutBuffer[0]));
    }
    AIStartDMA();
}

// This SDK has no __AXOutQuit the DOL goes straight from __AXOutInit to AXRegisterCallback

void AXRegisterCallback(void (* callback)()) {
    __AXUserFrameCallback = callback;
}
