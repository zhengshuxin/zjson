#include "StdAfx.h"
#include "json/array.h"

/* array_iter_head - get the head of the array */

static void *array_iter_head(ITER *iter, struct ARRAY *a)
{
	iter->dlen = -1;
	iter->key = NULL;
	iter->klen = 0;
	iter->i = 0;
	iter->size = a->count;
	if (a->items == NULL || a->count <= 0) {
		iter->ptr = iter->data = 0;
	} else {
		iter->ptr = iter->data = a->items[0];
	}

	return iter->ptr;
}

/* array_iter_next - get the next of the array */

static void *array_iter_next(ITER *iter, struct ARRAY *a)
{
	iter->i++;
	if (iter->i >= a->count) {
		iter->data = iter->ptr = 0;
	} else {
		iter->data = iter->ptr = a->items[iter->i];
	}
	return iter->ptr;
}
 
/* array_iter_tail - get the tail of the array */

static void *array_iter_tail(ITER *iter, struct ARRAY *a)
{
	iter->dlen = -1;
	iter->key = NULL;
	iter->klen = 0;
	iter->i = a->count - 1;
	iter->size = a->count;
	if (a->items == NULL || iter->i < 0) {
		iter->ptr = iter->data = 0;
	} else {
		iter->data = iter->ptr = a->items[iter->i];
	}
	return iter->ptr;
}

/* array_iter_prev - get the prev of the array */

static void *array_iter_prev(ITER *iter, struct ARRAY *a)
{
	iter->i--;
	if (iter->i < 0) {
		iter->data = iter->ptr = 0;
	} else {
		iter->data = iter->ptr = a->items[iter->i];
	}
	return iter->ptr;
}

/* grows internal buffer to satisfy required minimal capacity */
static void array_grow(ARRAY *a, int min_capacity)
{
	int min_delta = 16;
	int delta;

	/* don't need to grow the capacity of the array */
	if(a->capacity >= min_capacity) {
		return;
	}

	delta = min_capacity;
	/* make delta a multiple of min_delta */
	delta += min_delta - 1;
	delta /= min_delta;
	delta *= min_delta;
	/* actual grow */
	if (delta <= 0) {
		return;
	}

	a->capacity += delta;

	if (a->items == NULL) {
		a->items = (void**) malloc(a->capacity * sizeof(void*));
	} else {
		a->items = (void**) realloc(a->items, a->capacity * sizeof(void*));
	}

	/* reset, just in case */
	memset(a->items + a->count, 0, (a->capacity - a->count) * sizeof(void *));
}

ARRAY *array_create(int init_size)
{
	ARRAY *a;

	a = (ARRAY *) calloc(1, sizeof(ARRAY));

	a->iter_head = array_iter_head;
	a->iter_next = array_iter_next;
	a->iter_tail = array_iter_tail;
	a->iter_prev = array_iter_prev;

	if(init_size <= 0) {
		init_size = 100;
	}

	array_grow(a, a->count + init_size);
	return a;
}

void array_free(ARRAY *a, void (*free_fn)(void *))
{
	int i;

	for (i = 0; i < a->count; i++) {
		if(free_fn != NULL && a->items[i] != NULL) {
			free_fn(a->items[i]);
		}
	}
	if (a->items) {
		free(a->items);
	}
	free(a);
}

int array_append(ARRAY *a, void *obj)
{
	if (a->count >= a->capacity) {
		array_grow(a, a->count + 16);
	}
	a->items[a->count++] = obj;
	return a->count - 1;
}

void *array_index(const ARRAY *a, int idx)
{
	if (idx < 0 || idx > a->count - 1) {
		return NULL;
	}

	return a->items[idx];
}

int array_size(const ARRAY *a)
{
	return a->count;
}
