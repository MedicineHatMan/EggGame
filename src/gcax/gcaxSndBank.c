#include <gcax/gcax.h>

// 0x80230D7C
int gcaxNewAramBank(int bank, int size)
{
	int ret;

	ret = -1;
	size = (size + 0x1F) & ~0x1F;
	if (bank >= 0 && bank < GCAX_BANK_MAX && gcax_work->aram_rest >= size) {
		gcax_work->aram_bank[bank].adr = gcax_work->aram_next;
		gcax_work->aram_bank[bank].size = size;
		gcax_work->aram_next += size;
		gcax_work->aram_rest -= size;
		ret = 0;
	}
	if (ret != 0) {
		OSReport("AX SDLIB: gcaxNewAramBank(%d,%d) failed (should not happen\n", bank, size);
	}
	return ret;
}

/* Loads an MPB file already in memory the wave data goes to the bank's ARAM the parameters to the heap */
// 0x80230E54
int gcaxSndLoadMpb(int bank, GCAX_CHUNK* mpb, GCAX_CALLBACK cb, void* arg, int nowait)
{
	GCAX_CHUNK* chunk;
	int ret = 0;

	chunk = (GCAX_CHUNK*)((u8*)mpb + 0x10);
	if (chunk->magic != GCAX_MAGIC || chunk->type != GCAX_TYPE_MPBW) {
		return -1;
	}
	if (gcax_work->aram_bank[bank].size < chunk->size) {
		return -2;
	}
	if (cb == (GCAX_CALLBACK)-1) {
		nowait = 0;
		cb = 0;
	}
	ret = gcaxArqPostEx((u32)(chunk + 1), gcax_work->aram_bank[bank].adr, chunk->size, cb, arg, nowait);
	chunk = (GCAX_CHUNK*)((u8*)chunk + chunk->size + sizeof(GCAX_CHUNK));
	if (chunk->magic != GCAX_MAGIC || chunk->type != GCAX_TYPE_MPBP) {
		return -3;
	}
	if (gcax_work->mpbp[bank].ptr != 0) {
		gcaxSndFree(gcax_work->mpbp[bank].ptr);
	}
	gcax_work->mpbp[bank].ptr = gcaxSndMalloc(chunk->size);
	if (gcax_work->mpbp[bank].ptr == 0) {
		return -4;
	}
	gcax_work->mpbp[bank].size = chunk->size;
	memcpy(gcax_work->mpbp[bank].ptr, chunk + 1, chunk->size);
	return ret;
}

/* Loads an MSB sequence bank into the heap */
// 0x80230FF8
int gcaxSndLoadMsb(int bank, GCAX_CHUNK* msb, GCAX_CALLBACK cb, void* arg, int call)
{
	int ret = 0;
	GCAX_CHUNK* chunk = msb;

	if (chunk->magic != GCAX_MAGIC || chunk->type != GCAX_TYPE_MSB) {
		return -1;
	}
	if (cb == (GCAX_CALLBACK)-1) {
		call = 0;
		cb = 0;
	}
	if (gcax_work->msb[bank].ptr != 0) {
		gcaxSndFree(gcax_work->msb[bank].ptr);
	}
	gcax_work->msb[bank].ptr = gcaxSndMalloc(chunk->size);
	if (gcax_work->msb[bank].ptr == 0) {
		return -2;
	}
	gcax_work->msb[bank].size = chunk->size;
	memcpy(gcax_work->msb[bank].ptr, chunk + 1, chunk->size);
	if (call != 0) {
		cb(arg);
	}
	return ret;
}

/* A trace point the label is not used in this build */
// 0x8023112C
void gcaxSndTrace(const char* label)
{
	int changed = 0;

	if (gcax_work->trace_state != gcax_work->send_state) {
		gcax_work->trace_state = gcax_work->send_state;
		changed = 1;
	}
}

/* Takes an MPB file in pieces as it streams in the header then the wave chunk to ARAM and the parameter chunk to the
 * heap */
// 0x80231184
int gcaxSndSendBank(int bank, void* p, u32 size)
{
	int ret = 0;
	u8* cur;
	GCAX_CHUNK* chunk = 0;

	gcaxSndTrace("(A)");
	if (bank < 0) {
		return 0;
	}
	if (size == 0) {
		return 0;
	}
	if (bank >= GCAX_BANK_MAX) {
		return -109;
	}
	gcax_work->send_slot = bank;
	if (size < 0x20) {
		return -110;
	}
	if ((size & 0x1F) != 0) {
		return -110;
	}
	gcax_work->send_len = size;
	if (p == 0) {
		if (gcax_work->send_state == 0) {
			return 0;
		}
		if (gcax_work->mpbp[gcax_work->send_slot].ptr != 0) {
			gcaxSndFree(gcax_work->mpbp[gcax_work->send_slot].ptr);
		}
		gcax_work->mpbp[gcax_work->send_slot].ptr = 0;
		gcax_work->send_state = 0;
		return 0;
	}
	if (((u32)p & 0x1F) != 0) {
		return -111;
	}
	cur = p;
	while (gcax_work->send_len > 0 && ret == 0) {
		gcaxSndTrace("(B)");
		switch (gcax_work->send_state) {
		case 0:
			chunk = (GCAX_CHUNK*)cur;
			if (chunk->magic != GCAX_MAGIC || chunk->type != GCAX_TYPE_MPB) {
				ret = -104;
			} else {
				cur += sizeof(GCAX_CHUNK);
				gcax_work->send_len -= sizeof(GCAX_CHUNK);
				gcax_work->send_bank_rest = chunk->size;
				gcax_work->send_state = 1;
				gcax_work->send_total = 0;
			}
			break;
		case 1:
			chunk = (GCAX_CHUNK*)cur;
			if (chunk->magic != GCAX_MAGIC) {
				ret = -104;
				break;
			}
			if (chunk->type == GCAX_TYPE_MPBW) {
				if (gcax_work->aram_bank[gcax_work->send_slot].size < chunk->size) {
					OSReport("AX SDLIB: ERROR! MPB bank %d too narrow (%d/%d)\n", gcax_work->send_slot,
					         gcax_work->aram_bank[gcax_work->send_slot].size, chunk->size);
					ret = -107;
					break;
				}
				gcax_work->send_dst = gcax_work->aram_bank[gcax_work->send_slot].adr;
				gcax_work->send_to_aram = 1;
			} else if (chunk->type == GCAX_TYPE_MPBP) {
				if (gcax_work->mpbp[gcax_work->send_slot].ptr != 0) {
					gcaxSndFree(gcax_work->mpbp[gcax_work->send_slot].ptr);
				}
				gcax_work->mpbp[gcax_work->send_slot].ptr = gcaxSndMalloc(chunk->size);
				if (gcax_work->mpbp[gcax_work->send_slot].ptr == 0) {
					ret = -106;
					break;
				}
				gcax_work->mpbp[gcax_work->send_slot].size = chunk->size;
				gcax_work->send_dst = (u32)gcax_work->mpbp[gcax_work->send_slot].ptr;
				gcax_work->send_to_aram = 0;
			} else {
				ret = -104;
				break;
			}
			cur += sizeof(GCAX_CHUNK);
			gcax_work->send_len -= sizeof(GCAX_CHUNK);
			gcax_work->send_bank_rest -= sizeof(GCAX_CHUNK);
			gcax_work->send_chunk_rest = chunk->size;
			gcax_work->send_state = 2;
			break;
		case 2:
			gcax_work->send_n = gcax_work->send_len;
			if (gcax_work->send_n > gcax_work->send_chunk_rest) {
				gcax_work->send_n = gcax_work->send_chunk_rest;
			}
			switch (gcax_work->send_to_aram) {
			case 0:
				memcpy((void*)gcax_work->send_dst, cur, gcax_work->send_n);
				break;
			case 1:
				gcaxArqPostWait((u32)cur, gcax_work->send_dst, gcax_work->send_n, 0, 0);
				break;
			}
			cur += gcax_work->send_n;
			gcax_work->send_dst += gcax_work->send_n;
			gcax_work->send_chunk_rest -= gcax_work->send_n;
			gcax_work->send_bank_rest -= gcax_work->send_n;
			gcax_work->send_len -= gcax_work->send_n;
			if (gcax_work->send_chunk_rest <= 0) {
				gcax_work->send_state = 1;
			}
			if (gcax_work->send_bank_rest <= 0) {
				goto done;
			}
			break;
		}
	}
done:
	if (gcax_work->send_bank_rest <= 0) {
		gcax_work->send_state = 0;
	}
	if (ret != 0) {
		if (gcax_work->mpbp[gcax_work->send_slot].ptr != 0) {
			gcaxSndFree(gcax_work->mpbp[gcax_work->send_slot].ptr);
		}
		gcax_work->mpbp[gcax_work->send_slot].ptr = 0;
		gcax_work->send_state = 0;
	}
	gcaxSndTrace("(C)");
	gcax_work->send_total += size;
	return ret;
}

/* Allocates a cleared node and appends it to a list */
// 0x80231834
GCAX_NODE* gcaxListNew(u32 size, GCAX_NODE** list, void (*init)(GCAX_NODE*))
{
	GCAX_NODE* last = *list;
	GCAX_NODE* node = 0;

	node = gcaxSndMalloc(size);
	if (node == 0) {
		return 0;
	}
	memset(node, 0, size);
	if (*list == 0) {
		*list = node;
	} else {
		while (last->next != 0) {
			last = last->next;
		}
		last->next = node;
	}
	if (init != 0) {
		init(node);
	}
	return node;
}

/* Unlinks a node and frees it */
// 0x802318E4
void gcaxListDelete(GCAX_NODE* node, GCAX_NODE** list, void (*fini)(GCAX_NODE*))
{
	GCAX_NODE* cur;
	GCAX_NODE* next;

	cur = *list;
	if (cur == node) {
		next = cur->next;
		if (fini != 0) {
			fini(cur);
		}
		gcaxSndFree(cur);
		*list = next;
		return;
	}
	while (cur != 0) {
		next = cur->next;
		if (next == 0) {
			break;
		}
		if (next == node) {
			cur->next = next->next;
			if (fini != 0) {
				fini(next);
			}
			gcaxSndFree(next);
			return;
		}
		cur = next;
	}
}

// 0x802319A0
void gcaxListClear(GCAX_NODE** list, void (*fini)(GCAX_NODE*))
{
	while (*list != 0) {
		gcaxListDelete(*list, list, fini);
	}
}

/* Returns the first node the test returns 0 for */
// 0x802319EC
GCAX_NODE* gcaxListFind(GCAX_NODE** list, int (*test)(GCAX_NODE*))
{
	GCAX_NODE* node;

	node = *list;
	while (node != 0) {
		if (test(node) == 0) {
			break;
		}
		node = node->next;
	}
	return node;
}

// 0x80231A50
GCAX_NODE* gcaxListNext(GCAX_NODE** list, GCAX_NODE* node)
{
	if (node == 0) {
		return *list;
	}
	return node->next;
}
