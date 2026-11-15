#include "Dolphin/exi.h"
#include "Dolphin/os.h"
#include "Dolphin/sdk_assert.h"

#ifndef OFFSET
#define OFFSET(n, a) (((u32)(n)) & ((a) - 1))
#endif

#define REG_MAX 5
#define REG(chan, idx) (__EXIRegs[((chan) * REG_MAX) + (idx)])

#define STATE_IDLE     0
#define STATE_DMA      1
#define STATE_IMM      2
#define STATE_BUSY     3
#define STATE_SELECTED 4
#define STATE_ATTACHED 8
#define STATE_LOCKED   16

#define MAX_CHAN 3

#define MAX_IMM 4
#define MAX_TYPE 3
#define MAX_DEV 3
#define MAX_FREQ 6

#define EXI_0LENGTH_EXILENGTH_MASK 0x03FFFFE0

#if DEBUG
const char * __EXIVersion = "<< Dolphin SDK - EXI\tdebug build: Apr 17 2003 12:21:00 (0x2301) >>";
#else
const char * __EXIVersion = "<< Dolphin SDK - EXI\trelease build: Apr  5 2004 04:14:14 (0x2301) >>";
#endif

static EXIControl Ecb[3];
static u32 IDSerialPort1;

// external functions

// prototypes
BOOL EXIClearInterrupts(s32 chan, BOOL exi, BOOL tc, BOOL ext);
static int __EXIProbe(s32 chan);

// 0x8023C700
static void SetExiInterruptMask(s32 chan, EXIControl* exi) {
    EXIControl* exi2;
    exi2 = &Ecb[2];

    switch (chan) {
    case 0:
        if ((exi->exiCallback == 0 && exi2->exiCallback == 0) || (exi->state & STATE_LOCKED)) {
            __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_0_EXI | OS_INTERRUPTMASK_EXI_2_EXI);
        } else {
            __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_0_EXI | OS_INTERRUPTMASK_EXI_2_EXI);
        }
        break;
    case 1:
        if (exi->exiCallback == 0 || (exi->state & STATE_LOCKED)) {
            __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_1_EXI);
        } else {
            __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_1_EXI);
        }
        break;
    case 2:
        if (__OSGetInterruptHandler(__OS_INTERRUPT_PI_DEBUG) == 0 || (exi->state & STATE_LOCKED)) {
            __OSMaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
        } else {
            __OSUnmaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
        }
        break;
    }
}

// 0x8023C7FC
static void CompleteTransfer(s32 chan) {
    EXIControl* exi;
    u8* buf;
    u32 data;
    int i;
    int len;

    exi = &Ecb[chan];
    ASSERTLINE(361, 0 <= chan && chan < MAX_CHAN);

    if (exi->state & STATE_BUSY) {
        if (exi->state & STATE_IMM) {
            if ((len = exi->immLen) != 0) {
                buf = exi->immBuf;
                data = __EXIRegs[(chan * 5) + 4];
                for(i = 0; i < len; i++) {
                    *buf++ = data >> ((3 - i) * 8);
                }
            }
        }
        exi->state &= ~STATE_BUSY;
    }
}

// 0x8023C8D4
int EXIImm(s32 chan, void* buf, s32 len, u32 type, EXICallback callback) {
    EXIControl* exi;
    BOOL enabled;
    u32 data;
    int i;

    exi = &Ecb[chan];
    ASSERTLINE(399, exi->state & STATE_SELECTED);
    ASSERTLINE(400, 0 <= chan && chan < MAX_CHAN);
    ASSERTLINE(401, 0 < len && len <= MAX_IMM);
    ASSERTLINE(402, type < MAX_TYPE);
    enabled = OSDisableInterrupts();

    if ((exi->state & STATE_BUSY) || !(exi->state & STATE_SELECTED)) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    exi->tcCallback = callback;
    if (exi->tcCallback) {
        EXIClearInterrupts(chan, 0, 1, 0);
        __OSUnmaskInterrupts(0x200000U >> (chan * 3));
    }

    exi->state |= STATE_IMM;
    if (type != 0) {
        data = 0;
        for(i = 0; i < len; i++) {
            data |= ((u8*)buf)[i] << ((3 - i) * 8);
        }
        __EXIRegs[(chan * 5) + 4] = data;
    }

    exi->immBuf = buf;
    exi->immLen = (type != 1) ? len : 0; 
    __EXIRegs[(chan * 5) + 3] = (type << 2) | 1 | ((len - 1) << 4);
    OSRestoreInterrupts(enabled);
    return 1;
}

// 0x8023CABC
int EXIImmEx(s32 chan, void* buf, s32 len, u32 mode) {
    s32 xLen;

    while (len) {
        xLen = (len < 4) ? len : 4;
        if (EXIImm(chan, buf, xLen, mode, 0) == 0) {
            return 0;
        }
        if (EXISync(chan) == 0) {
            return 0;
        }
        ((u8*)buf) += xLen;
        len -= xLen;
    }

    return 1;
}

// 0x8023CB5C
int EXIDma(s32 chan, void* buf, s32 len, u32 type, EXICallback callback) {
    EXIControl* exi;
    BOOL enabled;

    exi = &Ecb[chan];

    ASSERTLINE(504, exi->state & STATE_SELECTED);
    ASSERTLINE(505, OFFSET(buf, 32) == 0);
    ASSERTLINE(506, 0 < len && OFFSET(len, 32) == 0);
    ASSERTLINE(508, ((u32) len & ~EXI_0LENGTH_EXILENGTH_MASK) == 0);
    ASSERTLINE(510, type == EXI_READ || type == EXI_WRITE);

    enabled = OSDisableInterrupts();
    if ((exi->state & STATE_BUSY) || !(exi->state & STATE_SELECTED)) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    exi->tcCallback = callback;
    if ((u32)exi->tcCallback) {
        EXIClearInterrupts(chan, 0, 1, 0);
        __OSUnmaskInterrupts(0x200000U >> (chan * 3));
    }

    exi->state |= STATE_DMA;
    __EXIRegs[(chan * 5) + 1] = (u32)buf & EXI_0LENGTH_EXILENGTH_MASK;
    __EXIRegs[(chan * 5) + 2] = len;
    __EXIRegs[(chan * 5) + 3] = (type * 4) | 3;

    OSRestoreInterrupts(enabled);
    return 1;
}

// 0x8023CD20
int EXISync(s32 chan) {
    EXIControl* exi;
    int rc;
    BOOL enabled;

    exi = &Ecb[chan];
    rc = 0;
    ASSERTLINE(560, 0 <= chan && chan < MAX_CHAN);

    while ((exi->state & STATE_SELECTED)) {
        if (!(__EXIRegs[(chan * 5) + 3] & 1)) {
            enabled = OSDisableInterrupts();
            if (exi->state & STATE_SELECTED) {
                CompleteTransfer(chan);
                if (__OSGetDIConfig() != 0xFF || (OSGetConsoleType() & 0xf0000000) == 0x20000000 || exi->immLen != 4 || (__EXIRegs[chan * 5] & 0x70) || (__EXIRegs[(chan * 5) + 4] != 0x01010000 && __EXIRegs[(chan * 5) + 4] != 0x05070000 && __EXIRegs[(chan * 5) + 4] != 0x04220001) || __OSDeviceCode == 0x8200) {
                    rc = 1;
                }
            }
            OSRestoreInterrupts(enabled);
            break;
        }
    }

    ASSERTLINE(588, !(exi->state & STATE_BUSY));
    return rc;
}

// 0x8023CECC
BOOL EXIClearInterrupts(s32 chan, BOOL exi, BOOL tc, BOOL ext) {
    u32 cpr;
    u32 prev;

    ASSERTLINE(609, 0 <= chan && chan < MAX_CHAN);

    cpr = prev = __EXIRegs[(chan * 5)];
    prev &= 0x7F5;

    if (exi != 0) {
        prev |= 2;
    }

    if (tc != 0) {
        prev |= 8;
    }

    if (ext != 0) {
        prev |= 0x800;
    }

    __EXIRegs[(chan * 5)] = prev;
    return cpr;
}

// 0x8023CF90
EXICallback EXISetExiCallback(s32 chan, EXICallback exiCallback) {
    EXIControl* exi;
    EXICallback prev;
    BOOL enabled;

    exi = &Ecb[chan];
    ASSERTLINE(643, 0 <= chan && chan < MAX_CHAN);
    enabled = OSDisableInterrupts();

    prev = exi->exiCallback;
    exi->exiCallback = exiCallback;
    if (chan != 2) {
        SetExiInterruptMask(chan, exi);
    } else {
        SetExiInterruptMask(0, &Ecb[0]);
    }

    OSRestoreInterrupts(enabled);
    return prev;
}

void EXIProbeReset() {
    __EXIProbeStartTime[0] = __EXIProbeStartTime[1] = 0;
    Ecb[0].idTime = Ecb[1].idTime = 0;
    __EXIProbe(0);
    __EXIProbe(1);
}

// 0x8023D09C
static int __EXIProbe(s32 chan) {
    EXIControl* exi;
    BOOL enabled;
    int rc;
    u32 cpr;
    s32 t;

    exi = &Ecb[chan];
    ASSERTLINE(698, 0 <= chan && chan < MAX_CHAN);
    if (chan == 2) {
        return 1;
    }

    rc = 1;
    enabled = OSDisableInterrupts();
    cpr = __EXIRegs[(chan * 5)];

    if (!(exi->state & STATE_ATTACHED)) {
        if (cpr & 0x800) {
            EXIClearInterrupts(chan, 0, 0, 1);
            __EXIProbeStartTime[chan] = exi->idTime = 0;
        }

        if (cpr & 0x1000) {
            t = ((s32)(OSTicksToMilliseconds(OSGetTime()) / 100) + 1);

            if (__EXIProbeStartTime[chan] == 0) {
                __EXIProbeStartTime[chan] = t;
            }

            if (t - (s32)__EXIProbeStartTime[chan] < 3) {
                rc = 0;
            }
        } else {
            __EXIProbeStartTime[chan] = exi->idTime = 0;
            rc = 0;
        }
    } else if(!(cpr & 0x1000) || (cpr & 0x800)) {
        __EXIProbeStartTime[chan] = exi->idTime = 0;
        rc = 0;
    }

    OSRestoreInterrupts(enabled);
    return rc;
}

// 0x8023D25C
int EXIProbe(s32 chan) {
    EXIControl* exi = &Ecb[chan];
    int rc;
    u32 id;
    
    rc = __EXIProbe(chan);
    if (rc && !exi->idTime) {
        rc = EXIGetID(chan, 0, &id) ? 1 : 0;
    }

    return rc;
}

// 0x8023D2DC
s32 EXIProbeEx(s32 chan) {
    if (EXIProbe(chan)) {
        return 1;
    }

    if (__EXIProbeStartTime[chan]) {
        return 0;
    }

    return -1;
}

// 0x8023D340
static int __EXIAttach(s32 chan, EXICallback extCallback) {
    EXIControl* exi;
    BOOL enabled;

    exi = &Ecb[chan];
    ASSERTLINE(803, 0 <= chan && chan < 2);
    enabled = OSDisableInterrupts();

    if ((exi->state & STATE_ATTACHED) || !__EXIProbe(chan)) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    EXIClearInterrupts(chan, 1, 0, 0);
    exi->extCallback = extCallback;
    __OSUnmaskInterrupts(0x100000U >> (chan * 3));
    exi->state |= STATE_ATTACHED;

    OSRestoreInterrupts(enabled);
    return 1;
}

// 0x8023D420
int EXIAttach(s32 chan, EXICallback extCallback) {
    EXIControl* exi;
    BOOL enabled;
    int rc;

    exi = &Ecb[chan];
    ASSERTLINE(829, 0 <= chan && chan < 2);

    EXIProbe(chan);
    enabled = OSDisableInterrupts();
    if (exi->idTime == 0) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    rc = __EXIAttach(chan, extCallback);
    OSRestoreInterrupts(enabled);
    return rc;
}

// 0x8023D4D0
int EXIDetach(s32 chan) {
    EXIControl* exi;
    BOOL enabled;

    exi = &Ecb[chan];
    ASSERTLINE(863, 0 <= chan && chan < 2);
    enabled = OSDisableInterrupts();

    if (!(exi->state & STATE_ATTACHED)) {
        OSRestoreInterrupts(enabled);
        return 1;
    }

    if ((exi->state & STATE_LOCKED) && (exi->dev == 0)) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    exi->state &= ~STATE_ATTACHED;
    __OSMaskInterrupts(0x500000U >> (chan * 3));

    OSRestoreInterrupts(enabled);
    return 1;
}

int EXISelectSD(s32 chan, u32 dev, u32 freq) {
    EXIControl* exi;
    u32 cpr;
    BOOL enabled;

    exi = &Ecb[chan];

    ASSERTLINE(908, 0 <= chan && chan < MAX_CHAN);
    ASSERTLINE(909, chan == 0 && dev < MAX_DEV || dev == 0);
    ASSERTLINE(910, freq < MAX_FREQ);
    ASSERTLINE(911, !(exi->state & STATE_SELECTED));

    enabled = OSDisableInterrupts();
    if ((exi->state & STATE_SELECTED) || ((chan != 2) && (((dev == 0) && !(exi->state & STATE_ATTACHED) && (EXIProbe(chan) == 0)) || !(exi->state & STATE_LOCKED) || (exi->dev != dev)))) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    exi->state |= STATE_SELECTED;
    cpr = __EXIRegs[(chan * 5)];
    cpr &= 0x405;
    cpr |= freq * 0x10;
    __EXIRegs[(chan * 5)] = cpr;

    if (exi->state & STATE_ATTACHED) {
        switch (chan) {
        case 0:
            __OSMaskInterrupts(0x100000U);
            break;
        case 1:
            __OSMaskInterrupts(0x20000U);
            break;
        }
    }

    OSRestoreInterrupts(enabled);
    return 1;
}

// 0x8023D5A8
int EXISelect(s32 chan, u32 dev, u32 freq) {
    EXIControl* exi;
    u32 cpr;
    BOOL enabled;

    exi = &Ecb[chan];

    ASSERTLINE(903, 0 <= chan && chan < MAX_CHAN);
    ASSERTLINE(904, chan == 0 && dev < MAX_DEV || dev == 0);
    ASSERTLINE(905, freq < MAX_FREQ);
    ASSERTLINE(906, !(exi->state & STATE_SELECTED));

    enabled = OSDisableInterrupts();
    if ((exi->state & STATE_SELECTED) || ((chan != 2) && (((dev == 0) && !(exi->state & STATE_ATTACHED) && (__EXIProbe(chan) == 0)) || !(exi->state & STATE_LOCKED) || (exi->dev != dev)))) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    exi->state |= STATE_SELECTED;
    cpr = __EXIRegs[(chan * 5)];
    cpr &= 0x405;
    cpr |= (((1 << dev) << 7) | (freq * 0x10));
    __EXIRegs[(chan * 5)] = cpr;

    if (exi->state & STATE_ATTACHED) {
        switch (chan) {
        case 0:
            __OSMaskInterrupts(0x100000U);
            break;
        case 1:
            __OSMaskInterrupts(0x20000U);
            break;
        }
    }

    OSRestoreInterrupts(enabled);
    return 1;
}

// 0x8023D778
int EXIDeselect(s32 chan) {
    EXIControl* exi;
    u32 cpr;
    BOOL enabled;

    exi = &Ecb[chan];
    ASSERTLINE(957, 0 <= chan && chan < MAX_CHAN);
    enabled = OSDisableInterrupts();

    if (!(exi->state & STATE_SELECTED)) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    exi->state &= ~STATE_SELECTED;
    cpr = __EXIRegs[(chan * 5)];
    __EXIRegs[(chan * 5)] = cpr & 0x405;

    if (exi->state & STATE_ATTACHED) {
        switch (chan) {
        case 0:
            __OSUnmaskInterrupts(0x100000U);
            break;
        case 1:
            __OSUnmaskInterrupts(0x20000U);
            break;
        }
    }

    OSRestoreInterrupts(enabled);

    if ((chan != 2) && (cpr & 0x80)) {
        if (__EXIProbe(chan) != 0) {
            return 1;
        }
        return 0;
    }

    return 1;
}

// 0x8023D8AC
static void EXIIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
    s32 chan;
    EXIControl* exi;
    EXICallback callback;

    chan = (interrupt - 9) / 3;

    ASSERTLINE(1008, 0 <= chan && chan < MAX_CHAN);
    exi = &Ecb[chan];
    EXIClearInterrupts(chan, 1, 0, 0);

    callback = exi->exiCallback;
    if (callback) {
        OSContext exceptionContext;

        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        callback(chan, context);
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
    }
}

// 0x8023D978
static void TCIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
    s32 chan;
    EXIControl* exi;
    EXICallback callback;

    chan = (interrupt - 10) / 3;

    ASSERTLINE(1044, 0 <= chan && chan < MAX_CHAN);
    exi = &Ecb[chan];
    __OSMaskInterrupts(0x80000000U >> interrupt);
    EXIClearInterrupts(chan, 0, 1, 0);

    callback = exi->tcCallback;
    if (callback) {
        OSContext exceptionContext;

        exi->tcCallback = NULL;
        CompleteTransfer(chan);

        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        callback(chan, context);
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
    }
}

// 0x8023DA64
static void EXTIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
    s32 chan;
    EXIControl* exi;
    EXICallback callback;

    chan = (interrupt - 11) / 3;

    ASSERTLINE(1084, 0 <= chan && chan < 2);
    __OSMaskInterrupts(0x500000U >> (chan * 3));
    exi = &Ecb[chan];
    callback = exi->extCallback;
    exi->state &= ~STATE_ATTACHED;

    if (callback) {
        OSContext exceptionContext;
        
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        exi->extCallback = NULL;
        callback(chan, context);
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
    }
}

void EXIInit() {
    u32 id;

    while (((REG(0, 3) & 1) == 1) || ((REG(1, 3) & 1) == 1) || ((REG(2, 3) & 1) == 1)) {}

    __OSMaskInterrupts(0x7F8000U);
    __EXIRegs[0] = 0;
    __EXIRegs[5] = 0;
    __EXIRegs[10] = 0;
    __EXIRegs[0] = 0x2000;
    __OSSetInterruptHandler(9,  EXIIntrruptHandler);
    __OSSetInterruptHandler(10, TCIntrruptHandler);
    __OSSetInterruptHandler(11, EXTIntrruptHandler);
    __OSSetInterruptHandler(12, EXIIntrruptHandler);
    __OSSetInterruptHandler(13, TCIntrruptHandler);
    __OSSetInterruptHandler(14, EXTIntrruptHandler);
    __OSSetInterruptHandler(15, EXIIntrruptHandler);
    __OSSetInterruptHandler(16, TCIntrruptHandler);

    EXIGetID(0, 2, &IDSerialPort1);

    if (__OSInIPL) {
        EXIProbeReset();
    } else if (EXIGetID(0, 0, &id) && id == 0x7010000) {
        __OSEnableBarnacle(1, 0);
    } else if (EXIGetID(1, 0, &id) && id == 0x7010000) {
        __OSEnableBarnacle(0, 2);
    }

    OSRegisterVersion(__EXIVersion);
}

// 0x8023DCEC
int EXILock(s32 chan, u32 dev, EXICallback unlockedCallback) {
    EXIControl* exi;
    BOOL enabled;
    int i;

    exi = &Ecb[chan];
    ASSERTLINE(1196, 0 <= chan && chan < MAX_CHAN);
    ASSERTLINE(1197, chan == 0 && dev < MAX_DEV || dev == 0);
    enabled = OSDisableInterrupts();

    if (exi->state & STATE_LOCKED) {
        if (unlockedCallback) {
            ASSERTLINE(1203, chan == 0 && exi->items < (MAX_DEV - 1) || exi->items == 0);
            for(i = 0; i < exi->items; i++) {
                if (exi->queue[i].dev == dev) {
                    OSRestoreInterrupts(enabled);
                    return 0;
                }
            }
            exi->queue[exi->items].callback = unlockedCallback;
            exi->queue[exi->items].dev = dev;
            exi->items++;
        }
        OSRestoreInterrupts(enabled);
        return 0;
    }

    ASSERTLINE(1219, exi->items == 0);
    exi->state |= STATE_LOCKED;
    exi->dev = dev;
    SetExiInterruptMask(chan, exi);
    OSRestoreInterrupts(enabled);
    return 1;
}

// 0x8023DE94
int EXIUnlock(s32 chan) {
    EXIControl* exi;
    BOOL enabled;
    EXICallback unlockedCallback;

    exi = &Ecb[chan];
    ASSERTLINE(1243, 0 <= chan && chan < MAX_CHAN);
    enabled = OSDisableInterrupts();

    if (!(exi->state & STATE_LOCKED)) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    exi->state &= ~STATE_LOCKED;
    SetExiInterruptMask(chan, exi);
    if (exi->items > 0) {
        unlockedCallback = exi->queue[0].callback;
        if (--exi->items > 0) {
            memmove(&exi->queue[0], &exi->queue[1], exi->items * 8);
        }
        unlockedCallback(chan, 0);
    }

    OSRestoreInterrupts(enabled);
    return 1;
}

// 0x8023DF88
u32 EXIGetState(s32 chan) {
    EXIControl* exi;

    exi = &Ecb[chan];
    ASSERTLINE(1280, 0 <= chan && chan < MAX_CHAN);
    return exi->state;
}

static void UnlockedHandler(s32 chan, OSContext* context) {
    u32 id;
    EXIGetID(chan, 0, &id);
}

// 0x8023E020
s32 EXIGetID(s32 chan, u32 dev, u32* id) {
    EXIControl* exi = &Ecb[chan];
    int err;
    u32 cmd;
    s32 startTime;
    BOOL enabled;

    ASSERTLINE(1317, 0 <= chan && chan < MAX_CHAN);
    if (chan == 0 && dev == 2 && IDSerialPort1 != 0) {
        *id = IDSerialPort1;
        return 1;
    }

    if ((chan < 2) && (dev == 0)) {
        if ((__EXIProbe(chan) == 0)) {
            return 0;
        }

        if (exi->idTime == __EXIProbeStartTime[chan]) {
            *id = exi->id;
            return exi->idTime;
        }

        if (!__EXIAttach(chan, NULL)) {
            return 0;
        }

        startTime = __EXIProbeStartTime[chan];
    }

    enabled = OSDisableInterrupts();

    err = !EXILock(chan, dev, (chan < 2 && dev == 0) ? &UnlockedHandler : NULL);
    if (err == 0) {
        err = !EXISelect(chan, dev, 0);
        if (err == 0) {
            cmd = 0;
            err |= !EXIImm(chan, &cmd, 2, 1, 0);
            err |= !EXISync(chan);
            err |= !EXIImm(chan, id, 4, 0, 0);
            err |= !EXISync(chan);
            err |= !EXIDeselect(chan);
        }

        EXIUnlock(chan);
    }

    OSRestoreInterrupts(enabled);

    if ((chan < 2) && (dev == 0)) {
        EXIDetach(chan);
        enabled = OSDisableInterrupts();
        err |= __EXIProbeStartTime[chan] != startTime;
        
        if (!err) {
            exi->id = *id;
            exi->idTime = startTime;
        }

        OSRestoreInterrupts(enabled);

        if (err) {
            return 0;
        }

        return exi->idTime;
    }

    if (err) {
        return 0;
    }

    return 1;
}
