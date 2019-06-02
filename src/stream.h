/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file dummy.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief 
 *
 * @see https://lowenware.com/
 */#ifndef AISL_STREAM_H_A9EC6601_34B2_4F3E_B631_EEDA8B6EF0D3
#define AISL_STREAM_H_A9EC6601_34B2_4F3E_B631_EEDA8B6EF0D3

#include <aisl/types.h>
#include <aisl/stream.h>
#include "buffer.h"

#define AISL_STREAM(x) ((AislStream) x)

typedef enum
{
		AISL_STREAM_STATE_IDLE

	, AISL_STREAM_STATE_WAIT_HEADER
	, AISL_STREAM_STATE_WAIT_BODY

	, AISL_STREAM_STATE_READY

	, AISL_STREAM_STATE_SEND_HEADER
	, AISL_STREAM_STATE_SEND_BODY

	, AISL_STREAM_STATE_DONE

} AislStreamState;


struct aisl_stream {
	struct buffer  buffer;
	AislClient     client;

	void          *u_ptr;

	uint64_t       content_length;
	int32_t        head_offset;
	int32_t        body_offset;
	int            id;
	int            flags;

	AislHttpResponse http_response;
	AislStreamState  state;
};


AislStream
aisl_stream_new(AislClient client, int id);


void
aisl_stream_free(AislStream stream);


int32_t
aisl_stream_get_buffer_space(AislStream stream);


int32_t
aisl_stream_get_buffer_size(AislStream stream);


char *
aisl_stream_get_data(AislStream stream, int32_t *p_length);


void
aisl_stream_shift(AislStream stream, int32_t offset);


bool
aisl_stream_is_done(AislStream stream);


AislStreamState
aisl_stream_get_state(AislStream stream);


void
aisl_stream_set_request(AislStream       stream,
                        AislHttpMethod   http_method,
                        const char      *path,
                        const char      *query );

void
aisl_stream_set_header(AislStream stream, const char *key, const char *value);


int
aisl_stream_set_end_of_headers(AislStream stream);


int
aisl_stream_set_body(AislStream stream, const char *data, int32_t size);

#endif /* !AISL_STREAM_H */
