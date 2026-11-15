#include <gcax/gcax.h>

#pragma fp_contract off

#define GCAX_SEMITONE 1.0594630943592953 /* 2^(1/12) */
#define GCAX_FINE16 1.007246412223704    /* 2^(1/96) 16 fine steps */
#define GCAX_FINE 1.0004513695322617     /* 2^(1/1536) one fine step */

// 0x802324C4
void gcaxVoiceDropCallback(void* vpb)
{
	GCAX_VOICE* voice;

	voice = (GCAX_VOICE*)((AXVPB*)vpb)->userContext;
	AXSetVoiceState(vpb, 0);
	MIXReleaseChannel(vpb);
	voice->flags |= GCAX_VOICE_DROPPED;
	gcax_work->voice_count--;
	voice->vpb = AXAcquireVoice(0x1B, gcaxVoiceDropCallback, (u32)voice);
	if (voice->vpb != 0) {
		voice->flags &= ~GCAX_VOICE_DROPPED;
		AXSetVoiceState(voice->vpb, 0);
		gcax_work->voice_count++;
	} else {
		OSReport("AX SDLIB: AXAcquireVoice() failed in drop callback\n");
	}
}

// 0x80232588
void gcaxVoiceInit(void)
{
	int i;
	int n;

	AXSetMode(0);
	memset(gcax_work->voice, 0, sizeof(gcax_work->voice));
	for (i = 0; i < GCAX_VOICE_MAX; i++) {
		gcax_work->voice[i].index = i;
	}
	n = gcax_work->max_voice;
	if (n > GCAX_VOICE_MAX) {
		n = GCAX_VOICE_MAX;
	}
	for (i = 0; i < n; i++) {
		gcax_work->voice[i].vpb = AXAcquireVoice(0x1B, gcaxVoiceDropCallback, (u32)&gcax_work->voice[i]);
		if (gcax_work->voice[i].vpb == 0) {
			break;
		}
		gcax_work->voice_count++;
	}
}

/* Finds a free voice that holds an AX voice searching from the last one taken */
// 0x80232674
GCAX_VOICE* gcaxVoiceGet()
{
	GCAX_VOICE* voice;
	AXVPB* vpb;
	int i;

	for (i = 0; i < GCAX_VOICE_MAX; i++) {
		gcax_work->voice_rover++;
		gcax_work->voice_rover %= GCAX_VOICE_MAX;
		voice = &gcax_work->voice[gcax_work->voice_rover];
		if ((voice->flags & GCAX_VOICE_USED) && (voice->flags & GCAX_VOICE_DROPPED) &&
		    (vpb = AXAcquireVoice(0x1B, gcaxVoiceDropCallback, (u32)voice)) != 0) {
			gcax_work->voice_count++;
			voice->vpb = vpb;
			voice->flags &= ~GCAX_VOICE_DROPPED;
			return voice;
		}
		if (!(voice->flags & GCAX_VOICE_USED) && voice->vpb != 0) {
			return voice;
		}
	}
	return 0;
}

// 0x80232794
void gcaxVoiceFree(GCAX_VOICE* voice)
{
	voice->_104 = 0;
	if (voice->vpb != 0) {
		AXSetVoiceState(voice->vpb, 0);
		MIXReleaseChannel(voice->vpb);
	}
	voice->flags &= ~GCAX_VOICE_USED;
	voice->flags &= ~GCAX_VOICE_HOLD;
	voice->flags &= ~GCAX_VOICE_PLAYING;
	voice->prio = 0;
}

// 0x80232810
GCAX_VOICE* gcaxVoiceAlloc(GCAX_NOTE* note, int prio, int arg)
{
	GCAX_VOICE* voice;

	voice = gcaxVoiceGet(prio);
	if (voice != 0) {
		voice->vol = 1.0f;
		voice->note = note;
		voice->prio = prio;
		voice->arg = arg;
		voice->flags |= GCAX_VOICE_USED;
		voice->flags &= ~GCAX_VOICE_HOLD;
		voice->flags &= ~GCAX_VOICE_PLAYING;
		voice->_104 = 0;
	}
	return voice;
}

// 0x802328A8
int gcaxVoiceCountUsed(void)
{
	int n;
	int i;

	n = 0;
	if (gcax_work != 0) {
		for (i = 0; i < GCAX_VOICE_MAX; i++) {
			if (gcax_work->voice[i].flags != 0) {
				n++;
			}
		}
	}
	return n;
}

/* Splits a sample rate into octaves from 172.265625 Hz and 1/1024 steps within the octave */
// 0x80232908
int gcaxFreqToCode(int freq, int* oct, int* frac)
{
	f64 n;
	f64 base;
	int i;

	n = log10(freq / 172.265625) / log10(2.0);
	i = n;
	*oct = (i - 8) & 0xF;
	base = 172.265625 / pow(2.0, -i);
	i = 1024.0 * (freq - base) / base;
	*frac = i;
	i = (*oct << 11) | *frac;
	return i;
}

/* The playback rate of key on a sample recorded at root fine in 1/128 semitones */
// 0x80232A38
f32 gcaxKeyRatio(int root, int fine, int key)
{
	f64 r;

	r = 90316.8;
	if (fine < 0) {
		root++;
		fine += 0x80;
	}
	root -= key;
	if (root >= 0) {
		while (root >= 12) {
			root -= 12;
			r *= 0.5;
		}
		while (root > 0) {
			root--;
			r /= GCAX_SEMITONE;
		}
	} else {
		root = -root;
		while (root >= 12) {
			root -= 12;
			r *= 2.0;
		}
		while (root > 0) {
			root--;
			r *= GCAX_SEMITONE;
		}
	}
	while (fine > 16) {
		fine -= 16;
		r *= GCAX_FINE16;
	}
	while (fine > 0) {
		fine--;
		r *= GCAX_FINE;
	}
	return r;
}

/* Bends a rate by bend 0x1FFF to a semitone and gives the AX 16.16 source ratio */
// 0x80232B10
u32 gcaxBendSrcRatio(f32 ratio, int bend)
{
	f32 r;
	int neg;
	int semi;
	int fine;
	u32 src;

	r = 1.0f;
	neg = 0;
	if (bend < 0) {
		neg = 1;
		bend = -bend;
	}
	semi = bend / 0x1FFF;
	fine = bend % 0x1FFF;
	fine <<= 7;
	fine /= 0x1FFE;
	while (semi >= 12) {
		semi -= 12;
		r *= 2.0;
	}
	while (semi > 0) {
		semi--;
		r *= GCAX_SEMITONE;
	}
	while (fine > 16) {
		fine -= 16;
		r *= GCAX_FINE16;
	}
	while (fine > 0) {
		fine--;
		r *= GCAX_FINE;
	}
	if (neg) {
		r = ratio / r;
	} else {
		r = ratio * r;
	}
	src = r;
	if (src < 0x200) {
		src = 0x200;
	}
	if (src > 0x40000) {
		src = 0x3FFFF;
	}
	return src;
}

/* A pitch ratio for cent 0x2000 to a semitone */
// 0x80232C90
f32 gcaxCentToRatio(int cent)
{
	f32 r;
	int neg;
	int semi;
	int fine;

	r = 1.0f;
	neg = 0;
	if (cent < 0) {
		neg = 1;
		cent = -cent;
	}
	semi = cent / 0x2000;
	fine = cent % 0x2000;
	fine <<= 7;
	fine /= 0x2000;
	while (semi >= 12) {
		semi -= 12;
		r *= 2.0;
	}
	while (semi > 0) {
		semi--;
		r *= GCAX_SEMITONE;
	}
	while (fine > 16) {
		fine -= 16;
		r *= GCAX_FINE16;
	}
	while (fine > 0) {
		fine--;
		r *= GCAX_FINE;
	}
	if (neg) {
		r = 1.0f / r;
	}
	return r;
}

/* Sets a voice up from a region and starts it */
// 0x80232D88
void gcaxVoiceStart(int bank, GCAX_VOICE* voice, GCAX_LAYER* layer, GCAX_MPB* mpb, GCAX_REGION* region, int key,
                    u8* veltab, int vel)
{
	GCAX_PORT* port;
	int ch;
	GCAX_SAMPLE* sample;
	int bend;
	u32 src;
	int oct;
	int frac;
	int i;
	u32 loop;
	u32 end;
	u32 cur;

	voice->bank = bank;
	voice->layer = layer;
	voice->mpb = mpb;
	voice->region = region;
	voice->key = key;
	voice->veltab = veltab;
	voice->vel = vel;
	port = voice->note->port;
	ch = voice->note->chan;
	sample = (GCAX_SAMPLE*)((u8*)mpb + GCAX_OFFSET(mpb->samples));
	sample += region->sample;
	veltab += region->velcurve << 7;
	if (vel < region->vel_lo) {
		vel = region->vel_lo;
	}
	if (vel > region->vel_hi) {
		vel = region->vel_hi;
	}
	vel -= port->chan[ch].vel_bias;
	if (vel < 0) {
		vel = 0;
	}
	if (vel > 0x7F) {
		vel = 0x7F;
	}
	vel = veltab[vel];
	if (vel < 0) {
		vel = 0;
	}
	if (vel > 0x7F) {
		vel = 0x7F;
	}
	voice->velocity = vel;
	voice->level = region->level;
	voice->pan = gcax_pan_map[region->pan & 0x1F];
	voice->atten = region->atten;
	voice->vol = 1.0f;
	voice->ratio = gcaxKeyRatio(region->root, region->fine, key);
	voice->ratio *= port->chan[ch].tune;
	voice->bend_up = layer->bend_up;
	voice->bend_down = layer->bend_down;
	bend = port->chan[ch].bend;
	if (bend >= 0) {
		bend *= voice->bend_up;
	} else {
		bend *= voice->bend_down;
	}
	src = gcaxBendSrcRatio(voice->ratio, bend);
	voice->fx_invert = region->fx_invert;
	voice->fx_send = region->fx_send;
	voice->src.ratioHi = src >> 16;
	voice->src.ratioLo = src;
	voice->src.currentAddressFrac = 0;
	voice->src.last_samples[0] = 0;
	voice->src.last_samples[1] = 0;
	voice->src.last_samples[2] = 0;
	voice->src.last_samples[3] = 0;

	gcaxFreqToCode((int)(0.5f + 32000.0f * (src / 65536.0f)), &oct, &frac);
	if (oct > 8) {
		oct = -(16 - oct);
	}
	if (frac & 0x200) {
		frac = 1;
	} else {
		frac = 0;
	}
	i = region->rate_bias + oct + frac + region->atk * 2;
	if (i < 0) {
		i = 0;
	}
	if (i > 0x3F) {
		i = 0x3F;
	}
	voice->atk_time = gcax_atk_time[i];
	i = region->rate_bias + oct + frac + region->dec * 2;
	if (i < 0) {
		i = 0;
	}
	if (i > 0x3F) {
		i = 0x3F;
	}
	voice->dec_time = gcax_dec_time[i];
	i = region->rate_bias + oct + frac + region->sus * 2;
	if (i < 0) {
		i = 0;
	}
	if (i > 0x3F) {
		i = 0x3F;
	}
	voice->sus_time = gcax_dec_time[i];
	i = region->rate_bias + oct + frac + region->rel * 2;
	if (i < 0) {
		i = 0;
	}
	if (i > 0x3F) {
		i = 0x3F;
	}
	voice->rel_time = gcax_dec_time[i];

	if (voice->atk_time < 5.0f) {
		voice->atk_rate = 1.0f;
	} else {
		voice->atk_rate = 5.0f / voice->atk_time;
	}
	voice->env_phase = 0;
	if (voice->atk_rate >= 1.0f) {
		voice->atk_rate = 1.0f;
		voice->env = 1.0f;
		voice->env_db = 0.0f;
		voice->env_phase = 1;
	} else {
		voice->env = 0.0f;
		voice->env_db = -96.0f;
	}
	voice->dec_rate = 5.0f / voice->dec_time;
	voice->sus_rate = 5.0f / voice->sus_time;
	voice->rel_rate = 5.0f / voice->rel_time;
	if (region->sus_level == 0) {
		voice->sus_db = 0.0f;
	} else {
		voice->sus_db = gcaxLookup(1.5f * -(f32)(31 - region->sus_level));
	}
	voice->time = 0.0f;
	MIXInitChannel(voice->vpb, 0, 0, -60, -60, 64, 0, 0);

	switch (sample->format) {
	case 0:
		loop = sample->loop_adr + sample->offset * 2 + gcax_work->aram_bank[bank].adr * 2;
		end = sample->end_adr + sample->offset * 2 + gcax_work->aram_bank[bank].adr * 2;
		cur = sample->cur_adr + sample->offset * 2 + gcax_work->aram_bank[bank].adr * 2;
		(void)(sample->loop == 0);
		voice->addr.loopFlag = sample->loop;
		voice->addr.format = 0;
		voice->addr.loopAddressHi = loop >> 16;
		voice->addr.loopAddressLo = loop;
		voice->addr.endAddressHi = end >> 16;
		voice->addr.endAddressLo = end;
		voice->addr.currentAddressHi = cur >> 16;
		voice->addr.currentAddressLo = cur;
		memcpy(voice->adpcm.a, sample->coef, sizeof(sample->coef));
		voice->adpcm.gain = sample->gain;
		voice->adpcm.pred_scale = sample->pred_scale;
		voice->adpcm.yn1 = sample->yn1;
		voice->adpcm.yn2 = sample->yn2;
		voice->adpcm_loop.loop_pred_scale = sample->loop_pred_scale;
		voice->adpcm_loop.loop_yn1 = sample->loop_yn1;
		voice->adpcm_loop.loop_yn2 = sample->loop_yn2;
		AXSetVoiceAddr(voice->vpb, &voice->addr);
		AXSetVoiceSrcType(voice->vpb, 1);
		AXSetVoiceSrc(voice->vpb, &voice->src);
		AXSetVoiceAdpcm(voice->vpb, &voice->adpcm);
		AXSetVoiceAdpcmLoop(voice->vpb, &voice->adpcm_loop);
		gcaxVoiceSetMix(voice, port);
		if (sample->length >= 3) {
			AXSetVoiceState(voice->vpb, 1);
		}
		voice->flags |= GCAX_VOICE_PLAYING;
		break;
	case 10:
		loop = sample->offset + sample->loop_adr * 2 + gcax_work->aram_bank[bank].adr;
		end = sample->offset + sample->end_adr * 2 + gcax_work->aram_bank[bank].adr;
		cur = sample->offset + sample->cur_adr * 2 + gcax_work->aram_bank[bank].adr;
		if (sample->loop == 0) {
			loop = gcax_work->zero_adr;
		}
		loop >>= 1;
		end >>= 1;
		cur >>= 1;
		voice->addr.loopFlag = sample->loop;
		voice->addr.format = 10;
		voice->addr.loopAddressHi = loop >> 16;
		voice->addr.loopAddressLo = loop;
		voice->addr.endAddressHi = end >> 16;
		voice->addr.endAddressLo = end;
		voice->addr.currentAddressHi = cur >> 16;
		voice->addr.currentAddressLo = cur;
		AXSetVoiceAddr(voice->vpb, &voice->addr);
		AXSetVoiceSrcType(voice->vpb, 1);
		AXSetVoiceSrc(voice->vpb, &voice->src);
		gcaxVoiceSetMix(voice, port);
		AXSetVoiceState(voice->vpb, 1);
		voice->flags |= GCAX_VOICE_PLAYING;
		break;
	case 0x19:
		loop = sample->loop_adr + sample->offset + gcax_work->aram_bank[bank].adr;
		end = sample->end_adr + sample->offset + gcax_work->aram_bank[bank].adr;
		cur = sample->cur_adr + sample->offset + gcax_work->aram_bank[bank].adr;
		if (sample->loop == 0) {
			loop = gcax_work->zero_adr;
		}
		voice->addr.loopFlag = sample->loop;
		voice->addr.format = 0x19;
		voice->addr.loopAddressHi = loop >> 16;
		voice->addr.loopAddressLo = loop;
		voice->addr.endAddressHi = end >> 16;
		voice->addr.endAddressLo = end;
		voice->addr.currentAddressHi = cur >> 16;
		voice->addr.currentAddressLo = cur;
		AXSetVoiceAddr(voice->vpb, &voice->addr);
		AXSetVoiceSrcType(voice->vpb, 1);
		AXSetVoiceSrc(voice->vpb, &voice->src);
		gcaxVoiceSetMix(voice, port);
		AXSetVoiceState(voice->vpb, 1);
		voice->flags |= GCAX_VOICE_PLAYING;
		break;
	}
}

/* The number of voices a key needs on a program */
// 0x80233660
int gcaxSndCountVoices(u32 bank, u32 prog, int key)
{
	u8* base;
	GCAX_MPB* mpb;
	u32* entry;
	int nlayer;
	GCAX_LAYER* layer;
	int nregion;
	GCAX_REGION* region;
	int n;
	int i;
	int j;

	n = 0;
	base = gcax_work->mpbp[bank].ptr;
	if (base != 0) {
		mpb = (GCAX_MPB*)base;
		if (GCAX_COUNT(mpb->progs) > prog) {
			entry = (u32*)(base + GCAX_OFFSET(mpb->progs));
			entry += prog;
			nlayer = GCAX_COUNT(*entry);
			layer = (GCAX_LAYER*)(base + GCAX_OFFSET(*entry));
			for (i = 0; i < nlayer; i++) {
				nregion = GCAX_COUNT(layer[i].regions);
				region = (GCAX_REGION*)(base + GCAX_OFFSET(layer[i].regions));
				for (j = 0; j < nregion; j++) {
					if (region[j].key_lo <= key && key <= region[j].key_hi) {
						n++;
					}
				}
			}
		}
	}
	return n;
}

/* Starts the voices on a list one for each region the key falls in */
// 0x80233758
void gcaxSndStartVoices(GCAX_LINK** list, u32 bank, u32 prog, int key, int vel)
{
	GCAX_LINK* link;
	GCAX_MPB* mpb;
	u32* entry;
	u8* base;
	u8* velcurves;
	int nlayer;
	GCAX_LAYER* layer;
	int nregion;
	GCAX_REGION* region;
	int i;
	int j;

	link = 0;
	(void)link;
	base = gcax_work->mpbp[bank].ptr;
	if (base != 0) {
		mpb = (GCAX_MPB*)base;
		if (GCAX_COUNT(mpb->progs) > prog) {
			entry = (u32*)(base + GCAX_OFFSET(mpb->progs));
			entry += prog;
			nlayer = GCAX_COUNT(*entry);
			layer = (GCAX_LAYER*)(base + GCAX_OFFSET(*entry));
			velcurves = base + GCAX_OFFSET(mpb->velcurves);
			for (i = 0; i < nlayer; i++) {
				nregion = GCAX_COUNT(layer[i].regions);
				region = (GCAX_REGION*)(base + GCAX_OFFSET(layer[i].regions));
				for (j = 0; j < nregion; j++) {
					if (region[j].key_lo <= key && key <= region[j].key_hi &&
					    (link = gcaxVoiceListNext(list, link)) != 0 && link->obj != 0) {
						gcaxVoiceStart(bank, link->obj, &layer[i], mpb, &region[j], key, velcurves, vel);
					}
				}
			}
		}
	}
}

/* Works out a voice's volumes from its envelope channel and port and gives them to AX */
// 0x802338B8
void gcaxVoiceSetMix(GCAX_VOICE* voice, void* handle)
{
	GCAX_PORT* port = handle;
	int ch;
	int n;
	int i;
	int vv;
	int cv;
	int pv;
	f32 t;
	f32 db;
	f32 r;
	f32 l;
	f32 gain;
	f32 sign = 1.0f;
	f32 front2;
	f32 b;
	f32 a;
	f32 sign2 = 1.0f;
	f32 ar;
	f32 al;
	f32 g1;
	f32 g2;
	f32 pos;
	f32 front;
	f32 sr;
	f32 sl;

	if (!(port->flags & 0x80000000)) {
		return;
	}
	ch = voice->note->chan;
	voice->mix.vL = 0;
	voice->mix.vR = 0;
	voice->mix.vAuxAL = 0;
	voice->mix.vAuxAR = 0;
	voice->mix.vAuxBL = 0;
	voice->mix.vAuxBR = 0;
	voice->mix.vAuxBS = 0;
	voice->mix.vS = 0;
	voice->mix.vAuxAS = 0;
	voice->mix.vDeltaL = 0;
	voice->mix.vDeltaR = 0;
	voice->mix.vDeltaAuxAL = 0;
	voice->mix.vDeltaAuxAR = 0;
	voice->mix.vDeltaAuxBL = 0;
	voice->mix.vDeltaAuxBR = 0;
	voice->mix.vDeltaAuxBS = 0;
	voice->mix.vDeltaS = 0;
	voice->mix.vDeltaAuxAS = 0;

	n = (port->chan[ch].vol + 1) * (port->chan[ch].expr + 1) - 1;
	n /= 128;
	if (n < 0) {
		n = 0;
	}
	if (n > 0x7F) {
		n = 0x7F;
	}
	cv = n;
	n = (cv + 1) * voice->velocity;
	n *= 0x100 - voice->atten;
	n -= 1;
	n >>= 14;
	n &= 0xFF;
	vv = n;
	n = (int)(255.0f * port->vol);
	pv = n;
	n = pv + vv;
	if (n < 0) {
		n = 0;
	}
	if (n > 0xFF) {
		n = 0xFF;
	}
	n = gcax_vol_db[n] - 3;
	if (n > 0xFF) {
		n = 0xFF;
	}
	db = 0.0f;
	i = voice->level << 2;
	if (i < 0) {
		i = 0;
	}
	if (i > 0x3F) {
		i = 0x3F;
	}
	i = gcax_level_db[i];
	if (i == 0) {
		db += -96.0f;
	} else {
		db += -(15 - i) * 3;
	}
	db += gcax_work->master_db;
	if (n & 0x01) {
		db += -0.375f;
	}
	if (n & 0x02) {
		db += -0.75f;
	}
	if (n & 0x04) {
		db += -1.5f;
	}
	if (n & 0x08) {
		db += -3.0f;
	}
	if (n & 0x10) {
		db += -6.0f;
	}
	if (n & 0x20) {
		db += -12.0f;
	}
	if (n & 0x40) {
		db += -24.0f;
	}
	if (n & 0x80) {
		db += -48.0f;
	}
	db += voice->env_db;
	gain = gcaxLookup(db);
	if (gain < 0.001f) {
		gain = 0.0f;
	}

	front = port->chan[ch].dpl_front;
	n = voice->pan + port->chan[ch].pan - 0x40;
	if (n < 0) {
		n = 0;
	}
	if (n > 0x7F) {
		n = 0x7F;
	}
	pos = n;
	n = gcax_pan_idx[n];
	l = gcax_pan_gain[n];
	r = gcax_pan_gain[n ^ 0x10];
	if (gcax_work->mono) {
		t = 0.707f * (l + r);
		if (t > 1.0f) {
			t = 1.0f;
		}
		l = r = t;
	}
	voice->pan_l = l;
	voice->pan_r = r;
	l *= gain;
	r *= gain;
	port->chan[ch].out_l = l;
	port->chan[ch].out_r = r;
	if (!gcax_work->surround) {
		voice->mix.vL = 32768.0f * (voice->vol * port->chan[ch].out_l);
		voice->mix.vR = 32768.0f * (voice->vol * port->chan[ch].out_r);
		voice->mix.vAuxBL = 0;
		voice->mix.vAuxBR = 0;
	} else {
		if ((f32)port->chan[ch].dpl_pan < 0.0f) {
			a = voice->vol * port->chan[ch].out_l;
			b = voice->vol * port->chan[ch].out_r;
		} else {
			g1 = gcax_dpl_gain[0x7F - port->chan[ch].dpl_pan];
			g2 = gcax_dpl_gain[port->chan[ch].dpl_pan];
			a = g1 * (voice->vol * gain);
			b = g2 * (voice->vol * gain);
		}
		if (port->chan[ch].dpl_front <= 0x40) {
			voice->mix.vL = 32768.0f * a;
			voice->mix.vR = 32768.0f * b;
			voice->mix.vAuxBL = 0;
			voice->mix.vAuxBR = 0;
		} else {
			voice->mix.vL = 0;
			voice->mix.vR = 0;
			voice->mix.vAuxBL = 32768.0f * a;
			voice->mix.vAuxBR = 32768.0f * b;
		}
	}

	sign2 = -2.0f;
	sign = sign2;
	if (voice->fx_invert == 0) {
		sign = 1.0f;
	}
	if (voice->fx_invert == 1) {
		sign2 = 1.0f;
	}
	t = 0.0f;
	i = voice->fx_send << 2;
	if (i < 0) {
		i = 0;
	}
	if (i > 0x3F) {
		i = 0x3F;
	}
	i = gcax_level_db[i];
	if (i == 0) {
		t += -96.0f;
	} else {
		t += -(15 - i) * 3;
	}
	t = gcaxLookup(t);
	t = port->fx_send + sign * t;
	if (t < 0.0f) {
		t = 0.0f;
	}
	if (t > 1.0f) {
		t = 1.0f;
	}
	sign = t * (l + r);
	if (sign > 2.0f) {
		sign = 2.0f;
	}
	voice->mix.vAuxAL = 32768.0f * (l * t);
	voice->mix.vAuxAR = 32768.0f * (r * t);
	if (!gcax_work->surround) {
		voice->mix.vAuxAL = 32768.0f * (l * t);
		voice->mix.vAuxAR = 32768.0f * (r * t);
		voice->mix.vAuxAS = 0;
		voice->mix.vAuxBS = 0;
	} else {
		front2 = port->chan[ch].dpl_front;
		al = (127.0f - front2) / 127.0f * (l * t);
		ar = (127.0f - front2) / 127.0f * (r * t);
		voice->mix.vAuxAL = 32768.0f * al;
		voice->mix.vAuxAR = 32768.0f * ar;
		sl = front2 / 127.0f * (l * t);
		sr = front2 / 127.0f * (r * t);
		/* Both go to vAuxAS in the DOL */
		voice->mix.vAuxAS = 32768.0f * al;
		voice->mix.vAuxAS = 32768.0f * ar;
	}
	AXSetVoiceMix(voice->vpb, &voice->mix);
}

// 0x802341EC
void gcaxVoiceSetPitch(GCAX_VOICE* voice, int bend, f32 scale)
{
	f32 r;

	if (bend >= 0) {
		bend *= voice->bend_up;
	} else {
		bend *= voice->bend_down;
	}
	r = gcaxCentToRatio(bend);
	r *= voice->ratio;
	r /= 65536.0f;
	r *= scale;
	AXSetVoiceSrcRatio(voice->vpb, r);
}

// 0x80234280
GCAX_LINK* gcaxVoiceListAdd(GCAX_LINK** list, GCAX_VOICE* voice)
{
	GCAX_LINK* link;

	link = (GCAX_LINK*)gcaxListNew(sizeof(GCAX_LINK), (GCAX_NODE**)list, 0);
	if (link != 0) {
		link->obj = voice;
	}
	return link;
}

// 0x802342D4
GCAX_LINK* gcaxVoiceListNext(GCAX_LINK** list, GCAX_LINK* link)
{
	return (GCAX_LINK*)gcaxListNext((GCAX_NODE**)list, (GCAX_NODE*)link);
}

// 0x80234304
void gcaxVoiceListFini(GCAX_LINK* link)
{
	if (link != 0 && link->obj != 0) {
		gcaxVoiceFree(link->obj);
	}
}

// 0x80234348
void gcaxVoiceListClear(GCAX_LINK** list)
{
	gcaxListClear((GCAX_NODE**)list, (void (*)(GCAX_NODE*))gcaxVoiceListFini);
}

/* Steps a voice's envelope by one 5 ms frame attack decay to the sustain level sustain then release */
// 0x80234378
void gcaxVoiceEnvelope(GCAX_VOICE* voice)
{
	if (voice->flags & GCAX_VOICE_HOLD) {
		return;
	}
	switch (voice->env_phase) {
	case 0:
		voice->env += voice->atk_rate;
		voice->env_db = -(96.0f - 96.0f * voice->env);
		if (voice->env >= 1.0f) {
			voice->env = 1.0f;
			voice->env_db = -(96.0f - 96.0f * voice->env);
			voice->env_phase++;
		}
		break;
	case 1:
		if (voice->env_db <= voice->sus_db) {
			voice->env_phase++;
		} else {
			voice->env -= voice->dec_rate;
			voice->env_db = -(96.0f - 96.0f * voice->env);
			if (voice->env <= 0.0f) {
				voice->env = 0.0f;
				voice->env_db = -(96.0f - 96.0f * voice->env);
			}
			if (!(voice->env_db <= voice->sus_db)) {
				break;
			}
			voice->env_phase++;
		}
	case 2:
		(void)(voice->env > 0.0f);
		voice->env_phase++;
	case 3:
		voice->env -= voice->sus_rate;
		voice->env_db = -(96.0f - 96.0f * voice->env);
		if (voice->env <= 0.0f) {
			voice->env = 0.0f;
			voice->env_db = -(96.0f - 96.0f * voice->env);
		}
		break;
	case 4:
		(void)(voice->env > 0.0f);
		voice->env_phase++;
	case 5:
		voice->env -= voice->rel_rate;
		voice->env_db = -(96.0f - 96.0f * voice->env);
		if (voice->env <= 0.0f) {
			voice->env = 0.0f;
			voice->env_db = -(96.0f - 96.0f * voice->env);
		}
		break;
	}
	voice->time += 5.0f;
}
