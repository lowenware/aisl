/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file src/buffer.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of struct buffer and functions
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef AISL_BUFFER_H_D60EB5DF_B70B_4D8F_AA63_7FDB569D67E9
#define AISL_BUFFER_H_D60EB5DF_B70B_4D8F_AA63_7FDB569D67E9

#include <stdint.h>
#include <stdarg.h>


struct buffer {
	char    *data;
	int32_t  size;
	int32_t  used;
};


int32_t
buffer_init(struct buffer *bs, int32_t size);


void
buffer_release(struct buffer *bs );


int32_t
buffer_insert(struct buffer *bs, int32_t offset, const char *data, int32_t size);


int32_t
buffer_append_printf(struct buffer *bs, const char *format, ...);


int32_t
buffer_append_vprintf(struct buffer *bs, const char *format, va_list args);


int32_t
buffer_append(struct buffer *bs, const char *data, int32_t size);


int32_t
buffer_shift(struct buffer *bs, int32_t size);



#endif /* !AISL_BUFFER_H */
