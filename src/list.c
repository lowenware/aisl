/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file list.c
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief List module source file
 *
 * @see https://lowenware.com/aisl/
 */

#include <stdlib.h>
#include "debug.h"
#include "list.h"

int32_t
list_init(struct list *list, int32_t size)
{
	if ((list->data = calloc(size, sizeof (void*))) != NULL) {
		list->size = size;
		list->count = 0;
		return 0;
	}
	return -1;
}


void
list_release(struct list *list, list_destructor_t destructor)
{
	if (list->data) {
		if (destructor) {
			int32_t i;

			for (i=0; i<list->count; i++) {
				void *ptr;

				if ((ptr = list->data[i]) != NULL)
					destructor(list->data[i]);
			}
		}
		free(list->data);
	}
}


int32_t
list_append(struct list *list, void *entry)
{
	int32_t pos = list->count;

	DPRINTF("pos = %d", pos);

	if (!(pos < list->size)) {
		DPRINTF("extending, size = %d", list->size);
		void **new_list;
		int32_t new_size = pos + 1;

		DPRINTF("extending, ptr = %p", (void*)list->data);
		if ((new_list = realloc( list->data, new_size*sizeof (void*) )) == NULL)
			return -1;

		list->data = new_list;
		list->size = new_size;
	}

	list->data[pos]=entry;
	list->count++;

	return pos;
}


void *
list_remove_index(struct list *list, int32_t index)
{
	void *result;

	if (index < list->count) {
		int32_t i, c = --list->count;

		result = list->data[index];

		for (i = index; i<c; i++) {
			list->data[i]=list->data[i+1];
		}
	}
	else
		result = NULL;

	return result;
}

