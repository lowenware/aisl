/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file buffer.c
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Buffer module source file
 *
 * @see https://lowenware.com/
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "buffer.h"


static int32_t
buffer_set_size(struct buffer *buffer, int32_t new_size)
{
	if (!buffer->size || new_size != buffer->size) {
		char *data;

		if (new_size) {
			if (new_size % 4096) {
				new_size = (new_size / 4096 + 1) * 4096;
			}
		} else {
			new_size = 16*1024; 
		}

		if ((data = realloc(buffer->data, new_size)) != NULL) {
			buffer->data = data;
			buffer->size = new_size;
		} else {
			new_size = -1;
		}
	}

	return new_size;
}


int32_t
buffer_init(struct buffer *buffer, int32_t size)
{
	if ((size = buffer_set_size(buffer, size)) != -1)
		buffer->used = 0;

	return size;
}


void
buffer_release(struct buffer *buffer)
{
	if (buffer->data) {
		free(buffer->data);
		buffer->data = NULL;
	}

	buffer->used = 0;
	buffer->size = 0;
}


static int32_t
buffer_move_offset(struct buffer *buffer, int32_t offset, int32_t size)
{
	int32_t to_move = buffer->used - offset;

	if (to_move < 0) {
		return -1;
	} else if (to_move) {
		memmove(&buffer->data[offset+size], &buffer->data[offset], to_move);
	}

	return size;
}

int32_t
buffer_insert(struct buffer *buffer, int32_t offset, const char *data,
		int32_t size)
{
	int32_t result;

	DPRINTF("Buffer: %d of %d", buffer->used, buffer->size);

	if ( (result = buffer_set_size(buffer, buffer->size + size)) != -1) {
		if ((result = buffer_move_offset(buffer, offset, size)) != -1) {
			memcpy(&buffer->data[offset], data, size);
			buffer->used += result;
		}
	}

	return result;
}


int32_t
buffer_append_printf(struct buffer *buffer, const char *format, ...)
{
	int32_t result;
	va_list args;

	va_start(args, format);
	result = buffer_append_vprintf(buffer, format, args);
	va_end(args);

	return result;
}

int32_t
buffer_append_vprintf(struct buffer *buffer, const char *format, va_list args)
{
	DPRINTF("Buffer: %d of %d", buffer->used, buffer->size);
	int32_t space, result;
	va_list cp_args;

	va_copy(cp_args, args);
	space = buffer->size - buffer->used;
	result = vsnprintf(&buffer->data[buffer->used], space, format, args);

	if (result < space) { /* enough space */
		buffer->used += result;
	} else {
		result = buffer_set_size(buffer, buffer->size + result - space + 1);
		if (result != -1)
			result = buffer_append_vprintf(buffer, format, cp_args);
	}
	va_end(cp_args);

	return result;
}

int32_t
buffer_append(struct buffer *buffer, const char *data, int32_t size)
{
	DPRINTF("Buffer: %d of %d", buffer->used, buffer->size);
	int32_t used, space;
	
	used = buffer->used,
	space = buffer->size - used;

	if (size > space) { /* enough space */
		if (buffer_set_size(buffer, buffer->size + size - space) == -1)
			return -1;
	}

	memcpy(&buffer->data[used], data, size);
	buffer->used += size;

	return size;
}


int32_t
buffer_shift(struct buffer *buffer, int32_t offset)
{
	int32_t used = buffer->used - offset;

	if (offset > 0) {
		if (offset < used) {
			memmove(buffer->data, &buffer->data[offset], used);
		} else {
			used = 0;
		}
		buffer->used = used;
	}
	return used;
}

