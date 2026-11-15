#include <stdarg.h>
#include <gcax/gcax.h>

// 0x80235D54
int gcaxSndInitEx(void* buf, int max_voice, u32 size, u32 aram_size)
{
	u32 zero;

	gcaxSndAllocWork(buf, size);
	gcax_work->size = sizeof(GCAX_WORK);
	gcax_work->master_db = 0.0f;
	gcax_work->arq_owner = 0xF;
	gcax_work->max_voice = max_voice;
	gcax_work->aram_size = aram_size;
	gcax_work->aram_base = ARAlloc(gcax_work->aram_size);
	gcax_work->aram_next = (gcax_work->aram_base + 0x1F) & ~0x1F;
	gcax_work->aram_rest = gcax_work->aram_size - (gcax_work->aram_next - gcax_work->aram_base);

	/* A silent block at the start of ARAM for samples that do not loop */
	zero = ((u32)gcax_work->zero_buf + 0x1F) & ~0x1F;
	memset((void*)zero, 0, 0x500);
	gcaxArqPostWait(zero, gcax_work->aram_next, 0x500, 0, 0);
	gcax_work->zero_adr = gcax_work->aram_next;
	gcax_work->aram_next += 0x500;
	gcax_work->aram_rest -= 0x500;

	gcax_work->tempo_scale = 1.0f;
	gcaxSndInitAram();
	gcax_work->_40 = 1;
	gcaxSndInitVoices();
	gcax_work->initialized = 1;
	return GCAX_OK;
}

// 0x80235EC8
int gcaxSndInit(void* buf)
{
	gcaxSndInitEx(buf, 0x30, 0xE1000, 0x800000);
	return GCAX_OK;
}

/* 0 for mono 1 for stereo */
// 0x80235F04
int gcaxSndSetOutput(int mode)
{
	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	switch (mode) {
	case 0:
		gcaxSndSetMono(1);
		break;
	case 1:
		gcaxSndSetMono(0);
		break;
	default:
		return GCAX_ERR_PARAM;
	}
	return GCAX_OK;
}

/* Sets the master level from 0 silent to 15 full in 3 dB steps */
// 0x80235F80
int gcaxSndSetMasterLevel(s8 level)
{
	f32 db;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	if (level < 0 || level > 15) {
		return GCAX_ERR_PARAM;
	}
	if (level == 0) {
		db = -96.0f;
	} else {
		db = -(15 - level) * 3;
	}
	gcaxSndSetMasterDb(db);
	return GCAX_OK;
}

// 0x80236040
int gcaxLoaderAlloc(GCAX_LOADER** out)
{
	GCAX_LOADER* loader;
	int i;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	loader = gcax_work->loader;
	for (i = 0; i < GCAX_LOADER_MAX; i++, loader++) {
		if (loader->used == 0) {
			goto found;
		}
	}
	return GCAX_ERR_NO_HANDLE;
found:
	memset(loader, 0, sizeof(GCAX_LOADER));
	loader->used = 1;
	*out = loader;
	return GCAX_OK;
}

// 0x802360F8
int gcaxLoaderCheck(GCAX_LOADER* loader)
{
	int i;

	if (loader == 0) {
		return GCAX_ERR_NULL;
	}
	for (i = 0; i < GCAX_LOADER_MAX; i++) {
		if (loader == &gcax_work->loader[i]) {
			if (loader->used != 0) {
				return GCAX_OK;
			}
			break;
		}
	}
	return GCAX_ERR_BAD_HANDLE;
}

// 0x8023616C
int gcaxLoaderSet(int handle, void* data, u32 size, GCAX_CALLBACK cb, void* arg)
{
	GCAX_LOADER* loader = (GCAX_LOADER*)handle;
	int err;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	err = gcaxLoaderCheck(loader);
	if (err != GCAX_OK) {
		return err;
	}
	loader->data = data;
	loader->arq = -1;
	loader->size = size;
	loader->cb = cb;
	loader->arg = arg;
	return GCAX_OK;
}

/* Frees a loader once its last transfer is done */
// 0x80236210
int gcaxLoaderFree(int handle)
{
	GCAX_LOADER* loader = (GCAX_LOADER*)handle;
	int err;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	err = gcaxLoaderCheck(loader);
	if (err != GCAX_OK) {
		return err;
	}
	if (loader->used != 0 && loader->arq >= 0) {
		while (gcax_work->arq_busy[loader->arq] != 0) {
		}
	}
	loader->used = 0;
	return GCAX_OK;
}

/* Loads each bank of an MLT bundle a "gcax" "MLT " chunk then a "gcax" "MLTM" table of 16-byte entries giving each
 * bank's type MPB or MSB slot and offset from the end of the table header */
// 0x802362BC
int gcaxLoaderLoad(int handle)
{
	GCAX_LOADER* loader = (GCAX_LOADER*)handle;
	u32* mlt;
	u8* base;
	u32* mltm;
	u32* entry;
	u32 bank;
	int n;
	int err;
	u32 type;
	int ret;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	err = gcaxLoaderCheck(loader);
	if (err != GCAX_OK) {
		return err;
	}
	base = loader->data;
	mlt = (u32*)base;
	mltm = (u32*)(base + 0x10);
	entry = (u32*)(base + 0x20);
	base = (u8*)loader->data + 0x20;
	if (mlt[0] != GCAX_MAGIC || mlt[1] != GCAX_TYPE_MLT || mltm[0] != GCAX_MAGIC || mltm[1] != GCAX_TYPE_MLTM) {
		return GCAX_ERR_DATA;
	}
	n = mltm[3] >> 4;
	err = GCAX_OK;
	while (n-- > 0) {
		if (entry[1] == 0xFFFFFFFF) {
			continue;
		}
		type = (entry[0] >> 24) & 0xF;
		bank = (entry[1] >> 24) & 0xF;
		ret = -1;
		switch (type) {
		case 1:
			ret = gcaxSndLoadMpb(bank, (GCAX_CHUNK*)(base + entry[2]), loader->cb, loader->arg, gcax_work->load_nowait);
			break;
		case 4:
			ret = gcaxSndLoadMsb(bank, (GCAX_CHUNK*)(base + entry[2]), loader->cb, loader->arg, gcax_work->load_nowait);
			break;
		}
		if (ret < 0) {
			err = GCAX_ERR_LOAD;
			break;
		}
		entry += 4;
	}
	return err;
}

// 0x80236450
int gcaxPortCheck(GCAX_PORT* port)
{
	int i;

	if (port == 0) {
		return GCAX_ERR_NULL;
	}
	for (i = 0; i < GCAX_PORT_MAX; i++) {
		if (port == &gcax_work->port[i]) {
			if (port->flags != 0) {
				return GCAX_OK;
			}
			break;
		}
	}
	return GCAX_ERR_BAD_HANDLE;
}

// 0x802364C0
int gcaxSndOpenPort(GCAX_PORT** out)
{
	GCAX_PORT* port;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	port = gcaxPortAlloc();
	if (port == 0) {
		return GCAX_ERR_NO_HANDLE;
	}
	*out = port;
	return GCAX_OK;
}

// 0x8023652C
int gcaxSndClosePort(u32 handle)
{
	GCAX_PORT* port = (GCAX_PORT*)handle;
	int err;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	err = gcaxPortCheck(port);
	if (err != GCAX_OK) {
		return err;
	}
	gcaxPortFree(port);
	return GCAX_OK;
}

/* Builds a 4-byte message the status then one or two data bytes from the arguments */
// 0x802365A0
int gcaxSndMakeMsg(u32 handle, u8* msg, int status, ...)
{
	GCAX_PORT* port = (GCAX_PORT*)handle;
	va_list args;
	int err;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	err = gcaxPortCheck(port);
	if (err != GCAX_OK) {
		return err;
	}
	va_start(args, status);
	(void)status;
	msg[0] = status;
	msg[1] = va_arg(args, int);
	switch ((u8)status & 0xF0) {
	case 0x80:
	case 0x90:
	case 0xA0:
	case 0xB0:
	case 0xD0:
	case 0xE0:
		msg[2] = va_arg(args, int);
		break;
	default:
		msg[2] = 0;
		break;
	}
	msg[3] = 0;
	va_end(args);
	return GCAX_OK;
}

// 0x80236720
int gcaxSndSendMsg(u32 handle, u8* msg, u8 prio)
{
	GCAX_PORT* port = (GCAX_PORT*)handle;
	int err;
	u32 word;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	err = gcaxPortCheck(port);
	if (err != GCAX_OK) {
		return err;
	}
	word = (u8)prio | ((msg[2] << 8) | ((msg[0] << 24) | (msg[1] << 16)));
	gcaxPortPost(port, word);
	return GCAX_OK;
}

// 0x802367DC
int gcaxSndReverbOn(void)
{
	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	gcaxSndSetAux(0, 2);
	return GCAX_OK;
}

// 0x80236820
int gcaxSndReverbOff(void)
{
	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	gcaxSndSetAux(0, 0);
	return GCAX_OK;
}

// 0x80236864
int gcaxSndResetPort(u32 handle)
{
	GCAX_PORT* port = (GCAX_PORT*)handle;
	int err;
	int ch;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	err = gcaxPortCheck(port);
	if (err != GCAX_OK) {
		return err;
	}
	for (ch = 0; ch < GCAX_CHAN_MAX; ch++) {
		gcaxChanReset(port, ch);
	}
	return GCAX_OK;
}

// 0x802368F8
int gcaxSndResetAll(void)
{
	int i;

	if (gcax_work == 0) {
		return GCAX_ERR_NOT_INIT;
	}
	for (i = 0; i < GCAX_PORT_MAX; i++) {
		gcaxSndResetPort((u32)&gcax_work->port[i]);
	}
	return GCAX_OK;
}
