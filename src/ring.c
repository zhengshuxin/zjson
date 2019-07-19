#include "StdAfx.h"

#include "json/ring.h"

/* ring_init - initialize ring head */

void ring_init(RING *ring)
{
	ring->pred = ring->succ = ring;
}

/* ring_append - insert entry after ring head */

void ring_append(RING *ring, RING *entry)
{
	entry->succ = ring->succ;
	entry->pred = ring;
	ring->succ->pred = entry;
	ring->succ = entry;
}

/* ring_prepend - insert new entry before ring head */

void ring_prepend(RING *ring, RING *entry)
{
	entry->pred = ring->pred;
	entry->succ = ring;
	ring->pred->succ = entry;
	ring->pred = entry;
}

/* ring_detach - remove entry from ring */

void ring_detach(RING *entry)
{
	RING *succ = entry->succ;
	RING *pred = entry->pred;

	pred->succ = succ;
	succ->pred = pred;

	entry->succ = entry->pred = 0;
}

int ring_empty(const RING *entry)
{
	return entry->succ == entry ? 1 : 0;
}

/* ring_pop_head - pop ring's head entry out from ring */

RING *ring_pop_head(RING *ring)
{
	RING   *succ;

	succ = ring->succ;
	if (succ == ring) {
		return NULL;
	}

	ring_detach(succ);
	return succ;
}

/* ring_pop_tail - pop ring's tail entry out from ring */

RING *ring_pop_tail(RING *ring)
{
	RING   *pred;

	pred = ring->pred;
	if (pred == ring) {
		return NULL;
	}

	ring_detach(pred);
	return pred;
}
