/* ----------------------------------------------------------------------------
 * aisl.h - header file for AISL library, part of AISLing Technology
 *
 * Copyright (c) 2017 by Löwenware Ltd. (https://lowenware.com/)
 *
 * Authors and maintainers:
 *   Ilja Kartaschoff <ik@lowenware.com>
 *
 * DOCUMENTATION
 * This file is not designed to be used as a documentation, but for looking at 
 * the precise values of constants and definitions.
 * Please, for documentation refer to web page https://lowenware.com/aisling/ or
 * file READEME.md from library source package.
 *
 * LICENSE and DISCLAIMER
 *
 * -------------------------------------------------------------------------- */

#ifndef _AISL_H_
#define _AISL_H_

/* system includes ---------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <arpa/inet.h>

/* aisl includes ------------------------------------------------------------ */

#include <aisl/status.h>
#include <aisl/event.h>
#include <aisl/stream.h>
#include <aisl/handle.h>
#include <aisl/http.h>



/* Control calls ------------------------------------------------------------ */

/* DEPRECATED, use aisl_handle_new instead
 * */
aisl_status_t
aisl_init();

/* DEPRECATED, use aisl_handle_free instead
 * */
void
aisl_release();

/* Tell library what socket should be opened. Could be called multiple times.
 * This function only save passed data. In fact, sockets are being opened only
 * inside aisl_run loop.
 * @address : host or IP to listen
 * @port    : port to listen
 * */

aisl_status_t
aisl_select(const char *address, int port);

/* Start main loop
 * @result  : exit code
 * */
aisl_status_t
aisl_run( int * flags );

/* Event calls -------------------------------------------------------------- */

/* Add callback to be executed after timeout. If callback function will return
 * true, callback will be kept in main loop and raised again, otherwise it will
 * be removed
 * @cb       : callback function:  bool callback (void * u_data)
 * @usec     : delay in milliseconds
 * @data     : user-defined data to be passed to callback
 * */
aisl_status_t
aisl_delay(aisl_callback_t cb, uint32_t msec, void *u_data);

/* Add event listener
 * @source : pointer to event source
 * @e_id   : event identifier
 * @cb     : callback to be executed
 * */
aisl_status_t
aisl_listen(void *source, aisl_event_t e_id, aisl_callback_t cb);

/* Raise event
 * @source : pointer to event source data
 * @e_id   : event identifier
 * @...    : custom event data
 * @result : true if event was handled, false otherwise
 * */
bool
aisl_raise(void *source, aisl_event_t e_id, ... );

/* input stream functions --------------------------------------------------- */


const char *
aisl_header_get(aisl_stream_t stream, const char *key);

const char *
aisl_header_get_by_index(aisl_stream_t stream, const char **key, uint32_t i);

/* -------------------------------------------------------------------------- */

#endif
