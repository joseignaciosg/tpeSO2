#include <stdlib.h>
#include <dos.h>

#include "mtask.h"
#include "softint.h"

typedef struct SoftInt_t SoftInt_t;
struct SoftInt_t
{
	unsigned		number;
	SoftHandler_t	old_handler;
	SoftInt_t *		next;
};

static SoftInt_t *SoftInts;

static void
restore_softints(void)
{
	SoftInt_t *si;

	for ( si = SoftInts ; si ; si = si->next )
		setvect(si->number, si->old_handler);
}

SoftHandler_t
HookInterrupt(unsigned number, SoftHandler_t handler)
{
	SoftInt_t *si;

	if ( !SoftInts )
		atexit(restore_softints);
	si = Malloc(sizeof(SoftInt_t));
	si->old_handler = getvect(si->number = number);
	si->next = SoftInts;
	SoftInts = si;
	setvect(number, handler);
	return si->old_handler;
}
