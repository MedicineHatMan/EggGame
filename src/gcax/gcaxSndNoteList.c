#include <gcax/gcax.h>

// 0x802321B0
GCAX_LINK* gcaxNoteListAdd(GCAX_LINK** list, GCAX_NOTE* note)
{
	GCAX_LINK* link;

	link = (GCAX_LINK*)gcaxListNew(sizeof(GCAX_LINK), (GCAX_NODE**)list, 0);
	if (link != 0) {
		link->obj = note;
	}
	return link;
}

// 0x80232204
GCAX_LINK* gcaxNoteListNext(GCAX_LINK** list, GCAX_LINK* link)
{
	return (GCAX_LINK*)gcaxListNext((GCAX_NODE**)list, (GCAX_NODE*)link);
}

// 0x80232234
void gcaxNoteListFini(GCAX_LINK* link)
{
	if (link != 0 && link->obj != 0) {
		gcaxNoteFree(link->obj);
	}
}

// 0x80232278
void gcaxNoteListDelete(GCAX_LINK** list, GCAX_LINK* link)
{
	gcaxListDelete((GCAX_NODE*)link, (GCAX_NODE**)list, (void (*)(GCAX_NODE*))gcaxNoteListFini);
}

// 0x802322B0
void gcaxNoteListClear(GCAX_LINK** list)
{
	gcaxListClear((GCAX_NODE**)list, (void (*)(GCAX_NODE*))gcaxNoteListFini);
}

// 0x802322E0
int gcaxNoteListTest(GCAX_LINK* link)
{
	if (link != 0 && link->obj == gcax_work->find_obj) {
		return 0;
	}
	return 1;
}

// 0x80232310
GCAX_LINK* gcaxNoteListFind(GCAX_LINK** list, GCAX_NOTE* note)
{
	gcax_work->find_obj = note;
	return (GCAX_LINK*)gcaxListFind((GCAX_NODE**)list, (int (*)(GCAX_NODE*))gcaxNoteListTest);
}

// 0x80232354
int gcaxNoteCountVoices(GCAX_NOTE* note)
{
	GCAX_LINK* link;
	int n;
	void* obj;

	link = 0;
	n    = 0;
	if (note != 0) {
		link = 0;
		while ((link = (GCAX_LINK*)gcaxListNext((GCAX_NODE**)&note->voices, (GCAX_NODE*)link)) != 0) {
			obj = link->obj;
			if (obj != 0) {
				n++;
			}
		}
	}
	return n;
}

/* Frees the notes on a list whose voices have all finished */
// 0x802323D8
void gcaxNoteListUpdate(GCAX_LINK** list)
{
	GCAX_LINK* link;
	int n;
	GCAX_LINK* vlink;
	GCAX_NOTE* note;
	GCAX_VOICE* voice;

restart:
	link  = 0;
	vlink = 0;
	while ((link = (GCAX_LINK*)gcaxListNext((GCAX_NODE**)list, (GCAX_NODE*)link)) != 0) {
		note = link->obj;
		n    = 0;
		if (note != 0) {
			vlink = 0;
			while ((vlink = (GCAX_LINK*)gcaxListNext((GCAX_NODE**)&note->voices, (GCAX_NODE*)vlink)) != 0) {
				voice = vlink->obj;
				if (voice != 0) {
					n++;
					if ((voice->flags & 0x80000003) == 0x80000002
					    && (voice->vpb->pb.state == 0 || (voice->env_phase == 5 && voice->env < 0.2f))) {
						n--;
					}
				}
			}
		}
		if (n == 0) {
			gcaxNoteRelease(note);
			goto restart;
		}
	}
}
