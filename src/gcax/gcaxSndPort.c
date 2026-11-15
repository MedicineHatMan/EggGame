#pragma fp_contract off
#include <gcax/gcax.h>

// 0x802345FC
void gcaxChanReset(GCAX_PORT* port, int ch)
{
	BOOL level;

	level = OSDisableInterrupts();
	port->chan[ch].flags = 0;
	port->chan[ch].vol = 100;
	port->chan[ch].pan = 0x40;
	port->chan[ch].bend_raw = 0x2000;
	port->chan[ch].expr = 0x7F;
	port->chan[ch].dpl_front = 0;
	port->chan[ch]._28 = 0;
	port->chan[ch].dpl_pan = -1;
	port->chan[ch]._40 = 0x88888888;
	port->chan[ch]._44 = 2290649300.0f;
	port->chan[ch]._48 = 2290649300.0f;
	OSRestoreInterrupts(level);
}

// 0x802346FC
GCAX_PORT* gcaxPortAlloc(void)
{
	BOOL level;
	GCAX_PORT* port = 0;
	int i;

	level = OSDisableInterrupts();
	for (i = 0; i < GCAX_PORT_MAX; i++) {
		if (!(gcax_work->port[i].flags & GCAX_PORT_USED)) {
			goto found;
		}
	}
	OSRestoreInterrupts(level);
	return 0;
found:
	port = &gcax_work->port[i];
	memset(port, 0, sizeof(GCAX_PORT));
	port->flags |= GCAX_PORT_USED;
	port->vol = 0.0f;
	port->_0C = 0.0f;
	port->pitch = 1.0f;
	port->_28 = 0.0f;
	port->_10 = 1.0f;
	port->vol_rest = -1.0f;
	port->pitch_rest = -1.0f;
	for (i = 0; i < GCAX_CHAN_MAX; i++) {
		port->chan[i].index = i;
		gcaxChanReset(port, i);
	}
	OSRestoreInterrupts(level);
	return port;
}

// 0x80234818
void gcaxPortFree(GCAX_PORT* port)
{
	BOOL level;

	level = OSDisableInterrupts();
	if (port->notes != 0) {
		gcaxNoteListClear(&port->notes);
		port->notes = 0;
	}
	port->flags &= ~GCAX_PORT_USED;
	OSRestoreInterrupts(level);
}

/* Queues a message status in the top byte then two data bytes then a priority */
// 0x80234880
void gcaxPortPost(GCAX_PORT* port, u32 msg)
{
	BOOL level;

	level = OSDisableInterrupts();
	if (port->msg_count < GCAX_MSG_MAX) {
		port->msg[port->msg_tail] = msg;
		port->msg_count++;
		port->msg_tail++;
		port->msg_tail %= GCAX_MSG_MAX;
	}
	OSRestoreInterrupts(level);
}

// 0x80234918
void gcaxPortProcess(GCAX_PORT* port)
{
	u32 msg;
	u8 status;
	u8 ch;
	u8 d1;
	u8 d2;
	int vel;
	BOOL level;

	while (port->msg_count != 0) {
		level = OSDisableInterrupts();
		port->msg_count--;
		msg = port->msg[port->msg_head];
		port->msg_head++;
		port->msg_head %= GCAX_MSG_MAX;
		OSRestoreInterrupts(level);
		status = (msg >> 24) & 0xF0;
		ch = (msg >> 24) & 0xF;
		d1 = (msg >> 16) & 0xFF;
		d2 = (msg >> 8) & 0xFF;
		vel = (s8)(u8)msg;
		switch (status) {
		case 0x80:
			if (!(port->chan[ch].flags & GCAX_CHAN_SUSTAIN)) {
				gcaxPortNoteOff(port, ch, d1, d2);
			}
			break;
		case 0x90:
			if (d2 != 0) {
				gcaxPortNoteOn(port, ch, d1, d2, vel);
			} else {
				if (!(port->chan[ch].flags & GCAX_CHAN_SUSTAIN)) {
					gcaxPortNoteOff(port, ch, d1, d2);
				}
			}
			break;
		case 0xA0:
			break;
		case 0xB0:
			gcaxPortControl(port, ch, d1, d2);
			break;
		case 0xC0:
			gcaxPortProgram(port, ch, d1);
			break;
		case 0xD0:
			break;
		case 0xE0:
			gcaxPortBend(port, ch, ((d2 & 0x7F) << 7) + (d1 & 0x7F));
			break;
		}
	}
}

/* Releases the notes on a key or frees them at once on a port that asks for that */
// 0x80234B10
void gcaxPortNoteOff(GCAX_PORT* port, u8 ch, u8 key, u8 vel)
{
	GCAX_LINK* link;
	GCAX_LINK* vlink;
	GCAX_NOTE* note;
	GCAX_VOICE* voice;

	link = 0;
	while ((link = gcaxNoteListNext(&port->notes, link)) != 0) {
		note = link->obj;
		if (note != 0 && note->chan == ch && note->key == key) {
			if (port->flags & GCAX_PORT_KILL) {
				gcaxNoteListDelete(&port->notes, link);
			} else {
				vlink = 0;
				while ((vlink = (GCAX_LINK*)gcaxListNext((GCAX_NODE**)&note->voices, (GCAX_NODE*)vlink)) != 0) {
					voice = vlink->obj;
					if (voice != 0 && voice->env_phase < 4) {
						voice->env_phase = 4;
					}
				}
			}
		}
	}
}

// 0x80234BFC
void gcaxPortNoteOn(GCAX_PORT* port, u8 ch, u8 key, u8 vel, int prio)
{
	GCAX_LINK* link;
	GCAX_NOTE* note;

	port->chan[ch].tune = 1.0f;
	port->chan[ch].vel_bias = 0;
	link = gcaxNoteListAdd(&port->notes, 0);
	if (link != 0) {
		gcaxChanRandomize(port, ch, key, vel, prio);
		note = gcaxNoteOn(port, ch, key, vel, prio);
		if (note == 0) {
			gcaxNoteListDelete(&port->notes, link);
		} else {
			note->port = port;
			note->key = key;
			note->chan = ch;
			link->obj = note;
		}
	}
}

// 0x80234CD8
void gcaxPortControl(GCAX_PORT* port, u8 ch, u8 cc, u8 val)
{
	int dpl;
	int front;
	int pan;

	val &= 0x7F;
	switch (cc) {
	case 0x00:
	case 0x20:
		port->chan[ch].bank = val;
		break;
	case 0x07:
		port->chan[ch].vol = val;
		gcaxChanUpdate(port, ch);
		break;
	case 0x0A:
		port->chan[ch].pan = val;
		port->chan[ch].dpl_pan = -1;
		gcaxChanUpdate(port, ch);
		break;
	case 0x0B:
		port->chan[ch].expr = val;
		gcaxChanUpdate(port, ch);
		break;
	case 0x3F:
		port->chan[ch].dpl_front = val;
		gcaxChanUpdate(port, ch);
		break;
	case 0x3E:
		val &= 0x7F;
		if (val <= 0x40) {
			dpl = val << 1;
			if (dpl > 0x7F) {
				dpl = 0x7F;
			}
			pan = dpl;
			front = 0;
		} else {
			dpl = (0x7F - val) << 1;
			pan = dpl;
			front = 0x7F;
		}
		port->chan[ch].dpl_pan = dpl;
		port->chan[ch].dpl_front = front;
		if (!gcax_work->surround) {
			port->chan[ch].pan = pan;
		}
		gcaxChanUpdate(port, ch);
		break;
	case 0x40:
		if (val >= 0x40) {
			port->chan[ch].flags |= GCAX_CHAN_SUSTAIN;
		} else {
			port->chan[ch].flags &= ~GCAX_CHAN_SUSTAIN;
		}
		break;
	case 0x79:
		gcaxChanReset(port, ch);
		gcaxChanUpdate(port, ch);
		break;
	}
}

// 0x80234F34
void gcaxPortProgram(GCAX_PORT* port, u8 ch, u8 prog)
{
	port->chan[ch].prog = (u8)prog & 0x7F;
}

// 0x80234F50
void gcaxPortBend(GCAX_PORT* port, u8 ch, u16 bend)
{
	port->chan[ch].bend_raw = bend & 0x3FFF;
	gcaxChanSetBend(port, ch);
}

// 0x80234FAC
void gcaxChanUpdate(GCAX_PORT* port, int ch)
{
}

// 0x80234FB0
void gcaxChanSetBend(GCAX_PORT* port, int ch)
{
	port->chan[ch].bend = port->chan[ch].bend_raw - 0x2000;
}

// 0x80234FD0
void gcaxPortUpdateChans(GCAX_PORT* port)
{
	int ch;

	if (port->flags & GCAX_PORT_USED) {
		for (ch = 0; ch < GCAX_CHAN_MAX; ch++) {
			gcaxChanUpdate(port, ch);
			gcaxChanSetBend(port, ch);
		}
	}
}

// 0x8023503C
void gcaxPortUpdateVoices(GCAX_PORT* port)
{
	GCAX_LINK* link;
	GCAX_LINK* vlink;
	int ch;

	if (port->flags & GCAX_PORT_USED) {
		link = 0;
		while ((link = gcaxNoteListNext(&port->notes, link)) != 0) {
			ch = ((GCAX_NOTE*)link->obj)->chan;
			vlink = 0;
			while ((vlink = gcaxVoiceListNext(&((GCAX_NOTE*)link->obj)->voices, vlink)) != 0) {
				gcaxVoiceEnvelope(vlink->obj);
				gcaxVoiceSetMix(vlink->obj, port);
				gcaxVoiceSetPitch(vlink->obj, port->chan[ch].bend, port->pitch);
			}
		}
	}
}

// 0x80235104
void gcaxSndUpdateChans(void)
{
	BOOL level;
	int i;
	int ch;

	level = OSDisableInterrupts();
	for (i = 0; i < GCAX_PORT_MAX; i++) {
		for (ch = 0; ch < GCAX_CHAN_MAX; ch++) {
			gcaxChanUpdate(&gcax_work->port[i], ch);
		}
	}
	OSRestoreInterrupts(level);
}

// 0x80235188
void gcaxPortVolFadeSet(GCAX_PORT* port)
{
	if (port->vol_rest <= 0.0f) {
		port->vol = port->vol_target;
		port->vol_time = 0.0f;
		port->vol_rest = -1.0f;
	} else {
		port->vol = port->vol_start + (port->vol_target - port->vol_start) * (1.0f - port->vol_rest / port->vol_time);
	}
}

// 0x802351EC
void gcaxPortVolFade(GCAX_PORT* port)
{
	if (port->vol_rest >= 0.0f) {
		port->vol_rest -= 5.0f;
		gcaxPortVolFadeSet(port);
	}
}

// 0x80235240
void gcaxPortPitchFadeSet(GCAX_PORT* port)
{
	if (port->pitch_rest <= 0.0f) {
		port->pitch = port->pitch_target;
		port->pitch_time = 0.0f;
		port->pitch_rest = -1.0f;
	} else {
		port->pitch = port->pitch_start +
		              (port->pitch_target - port->pitch_start) * (1.0f - port->pitch_rest / port->pitch_time);
	}
}

// 0x802352A4
void gcaxPortPitchFade(GCAX_PORT* port)
{
	if (port->pitch_rest >= 0.0f) {
		port->pitch_rest -= 5.0f;
		gcaxPortPitchFadeSet(port);
	}
}

/* Bank 2 programs 0 1 and 23 to 30 get a random detune and velocity drop on each note */
// 0x802352F8
void gcaxChanRandomize(GCAX_PORT* port, u8 ch, u8 key, u8 vel, int prio)
{
	if ((int)port->chan[ch].bank == 2) {
		switch (port->chan[ch].prog) {
		case 0:
		case 1:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
			port->chan[ch].tune = (gcaxRand() & 0xFFFF) / 200000.0f;
			port->chan[ch].tune += 0.84000003f; /* 0x3F570A3E one step above 0.84f */
			port->chan[ch].vel_bias = (gcaxRand() & 0xFFFF) / 2000;
			break;
		}
	}
}

/* Runs one 5 ms frame sequences messages fades envelopes and mixes on every port */
// 0x802353F8
void gcaxSndFrame(void)
{
	BOOL level;
	BOOL level2;
	BOOL level3;
	int i;

	if (gcax_work == 0 || gcax_work->initialized == 0) {
		return;
	}
	level = OSDisableInterrupts();
	gcax_work->busy++;
	OSRestoreInterrupts(level);
	if (gcax_work->busy != 1) {
		gcax_work->overruns++;
		level2 = OSDisableInterrupts();
		gcax_work->busy--;
		OSRestoreInterrupts(level2);
	} else {
		gcax_work->prev_time = gcax_work->time;
		gcax_work->time = OSGetTime();
		gcax_work->notes_used = gcaxNoteCount();
		gcax_work->voices_used = gcaxVoiceCountUsed();
		for (i = 0; i < GCAX_PORT_MAX; i++) {
			if (gcax_work->port[i].flags & GCAX_PORT_USED) {
				gcaxSeqUpdate(&gcax_work->port[i]);
				gcaxPortProcess(&gcax_work->port[i]);
			}
			gcaxPortVolFade(&gcax_work->port[i]);
			gcaxPortPitchFade(&gcax_work->port[i]);
			gcaxPortUpdateChans(&gcax_work->port[i]);
			gcaxPortUpdateVoices(&gcax_work->port[i]);
			gcaxNoteListUpdate(&gcax_work->port[i].notes);
		}
		gcax_work->notes_used = gcaxNoteCount();
		gcax_work->voices_used = gcaxVoiceCountUsed();
		level3 = OSDisableInterrupts();
		gcax_work->busy--;
		OSRestoreInterrupts(level3);
	}
}

// 0x802355F0
void gcaxSndSetMono(int mono)
{
	gcax_work->mono = mono ? TRUE : FALSE;
}

// 0x80235628
void gcaxSndSetSurround(int surround)
{
	gcax_work->surround = surround ? TRUE : FALSE;
	if (surround != 0) {
		AXSetMode(2);
	} else {
		AXSetMode(0);
	}
}
