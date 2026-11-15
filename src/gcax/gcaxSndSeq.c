#include <gcax/gcax.h>

#define GCAX_SEQ_PLAYING 0x2
#define GCAX_SEQ_PAUSED 0x1

// 0x80236960
int gcaxSeqReadVar(u8** pp)
{
	u8* p;
	int n;
	int v;
	int i;

	p = *pp;
	for (n = 0; n < 4; n++) {
		if (!(p[n] & 0x80)) {
			goto found;
		}
	}
	return -1;
found:
	v = 0;
	for (i = 0; i < n + 1; i++) {
		v <<= 7;
		v += *p & 0x7F;
		p++;
	}
	*pp = p;
	return v;
}

/* Advances a port's sequence by the time since the last frame and posts the events that fall in it */
// 0x802369F4
void gcaxSeqUpdate(GCAX_PORT* port)
{
	GCAX_SEQ* seq;
	u8* p;
	u8* q;
	int delta;
	u8 status;
	f32 d;

	if (!(port->flags & GCAX_PORT_USED)) {
		return;
	}
	if (!(port->seq_flags & GCAX_SEQ_PLAYING)) {
		return;
	}
	if (port->seq_flags & GCAX_SEQ_PAUSED) {
		return;
	}
	seq = &port->seq;
	p = seq->ptr;
	if (p == 0) {
		return;
	}
	gcax_work->seq_dt = gcax_work->time - seq->time;
	if (gcax_work->seq_dt < 0) {
		gcax_work->seq_dt = -gcax_work->seq_dt;
	}

restart:
	seq->beat = 60.0f / seq->tempo;
	d = (u32)OSTicksToMicroseconds(gcax_work->seq_dt);
	d *= gcax_work->tempo_scale;
	d *= seq->speed;
	d *= seq->tempo;
	d /= 60000000.0f;
	seq->prev_tick = seq->tick;
	seq->delta = d;
	seq->tick += d;

	do {
		q = p;
		delta = gcaxSeqReadVar(&q);
		if (delta < 0) {
			p = 0;
			break;
		}
		if (seq->next + delta > seq->tick) {
			break;
		}
		p = q;
		seq->prev_next = seq->next;
		seq->next += delta;
		if (*p & 0x80) {
			status = *p++;
		} else {
			status = seq->status;
		}
		if (status == 0xFF) {
			seq->status = 0xFF;
			switch (*p) {
			case 0x51:
				seq->tempo = 60000000.0f / ((p[2] << 16) + (p[3] << 8) + p[4]);
				p += 5;
				break;
			case 0x2F:
				p = 0;
				goto done;
			}
			continue;
		}
		switch (status & 0xF0) {
		case 0x80:
		case 0x90:
		case 0xE0:
			gcaxPortPost(port, (status << 24) + (p[0] << 16) + (p[1] << 8) + (u8)seq->prio);
			p += 2;
			seq->status = status;
			break;
		case 0xB0:
			if (p[0] == 0x1F) {
				if (seq->loop_ptr == 0) {
					seq->loop_ptr = p + 2;
					seq->loop_count = p[1];
					seq->loop_next = seq->next;
					seq->loop_tick = seq->prev_tick;
					seq->loop_tempo = seq->tempo;
					p += 2;
					break;
				}
				if (seq->loop_count == 0) {
				rewind:
					p = seq->loop_ptr;
					seq->tick = seq->loop_tick;
					seq->next = seq->loop_next;
					seq->tempo = seq->loop_tempo;
					goto restart;
				}
				if (seq->loop_count <= 0) {
					break;
				}
				seq->loop_count--;
				if (seq->loop_count > 0) {
					goto rewind;
				}
				p += 2;
				seq->status = 0;
				break;
			}
			gcaxPortPost(port, (status << 24) + (p[0] << 16) + (p[1] << 8) + (u8)seq->prio);
			p += 2;
			seq->status = status;
			break;
		case 0xA0:
		case 0xC0:
		case 0xD0:
			gcaxPortPost(port, (status << 24) + (p[0] << 16));
			p += 1;
			seq->status = status;
			break;
		}
	} while (seq->next < seq->tick);

done:
	seq->ptr = p;
	seq->time = gcax_work->time;
	if (p == 0) {
		port->seq_flags &= ~GCAX_SEQ_PLAYING;
		if (port->notes != 0) {
			gcaxNoteListClear(&port->notes);
			port->notes = 0;
		}
		memset(&port->seq, 0, sizeof(GCAX_SEQ));
	}
}
