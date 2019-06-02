/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file src/list.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief List module header file
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef AISL_LIST_H_21495B65_111D_40F7_840F_CC50D9D324A1
#define AISL_LIST_H_21495B65_111D_40F7_840F_CC50D9D324A1

#include <stdint.h>

#define LIST_INDEX(L, I) ( L.data[ I ] )


struct list {
	void   **data;
	int32_t  size;
	int32_t  count;
};


typedef void
(* list_destructor_t)(void *list_item);


int32_t
list_init(struct list *lst, int32_t size);


void
list_release(struct list *lst, list_destructor_t destructor);


int32_t
list_append(struct list *lst, void * entry);


void *
list_remove_index(struct list *lst, int32_t index);


#endif /* !AISL_LIST_H */
