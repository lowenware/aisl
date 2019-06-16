/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file aisl/stream.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of #AislStream functions
 *
 * @see https://lowenware.com/aisl/
 */
#ifndef AISL_STREAM_H_4D8EB622_3CE0_4F1B_AC1F_B27CCB5C2EDC
#define AISL_STREAM_H_4D8EB622_3CE0_4F1B_AC1F_B27CCB5C2EDC

#include <stdint.h>
#include <stdarg.h>
#include <aisl/types.h>

#define aisl_stream_get_instance aisl_get_instance

/** @brief Gets a value of #AislStream security flag
 *  @param stream an #AislStream instance
 *  @return is true if stream is encrypted and false otherwise
 */
bool
aisl_is_secure(AislStream stream);


/** @brief Gets an #AislClient instance associated with the stream
 *  @param stream an #AislStream instance
 *  @return an #AislClient instance pointer
 */
AislClient
aisl_get_client(AislStream stream);


/** @brief Gets an #AislServer instance associated with the stream
 *  @param stream an #AislStream instance
 *  @return an #AislServer instance pointer
 */
AislServer
aisl_get_server(AislStream stream);


/** @brief Gets an #AislHttpVersion of the stream
 *  @param stream an #AislStream instance
 *  @return an #AislHttpVersion value
 */
AislHttpVersion
aisl_get_http_version(AislStream stream);


/** @brief Gets an #AislInstance pointer associated with the stream
 *  @param stream an #AislStream instance
 *  @return an #AislInstance pointer
 */
AislInstance
aisl_get_instance(AislStream s);


/** @brief Gets the stream context previously set with #aisl_set_context
 *  @param stream an #AislStream instance
 *  @return a pointer to the stream context
 */
void *
aisl_get_context(AislStream stream);


/** @brief Associate a context pointer with the stream until its lifetime.
 *  Previously allocated data should be free'd on #AISL_EVENT_STREAM_CLOSE if
 *  not needed anymore.
 *  @param stream an #AislStream instance
 *  @param context a pointer to any user-defined data
 */
void
aisl_set_context(AislStream stream, void *context);


/** @brief A call to start stream data transmission to a client
 *  @param stream an #AislStream instance
 *  @return a #AislStatus displaying if stream is ready to be proceed by teh engine
 */
AislStatus
aisl_flush(AislStream stream);


/** @brief A call to reject the stream. In HTTP 1.X it also closes client's connection
 *  @param stream an #AislStream instance
 */
void
aisl_reject(AislStream stream);


/** @brief A call to begin the HTTP response
 *  @param stream an #AislStream instance
 *  @param status_code of the HTTP response
 *  @param content_length in bytes or #AISL_AUTO_LENGTH if length is not knonw yet
 *  @return #AislStatus code
 */
AislStatus
aisl_response(AislStream       stream,
              AislHttpResponse status_code,
              uint64_t         content_length);


/** @brief Adds HTTP header to the stream buffer
 *  @param stream an #AislStream instance
 *  @param key of HTTP header
 *  @param value of HTTP header
 *  @return a length of data added to the stream buffer
 */
int
aisl_header(AislStream stream, const char *key, const char *value);


/** @brief Adds printf-like formatted HTTP header to the stream buffer
 *  @param stream an #AislStream instance
 *  @param key of HTTP header
 *  @param format of HTTP header value
 *  @return a length of data added to the stream buffer
 */
int
aisl_header_printf(AislStream  stream, 
                   const char *key,
                   const char *format,
                   ... );


/** @brief Adds vprintf-like formatted HTTP header to the stream buffer
 *  @param stream an #AislStream instance
 *  @param key of HTTP header
 *  @param format of HTTP header value
 *  @param args list for HTTP header value construction
 *  @return a length of data added to the stream buffer
 */
int
aisl_header_vprintf(AislStream  stream,
                    const char *key,
                    const char *format,
                    va_list     args );


/** @brief Adds printf-like formatted HTTP response to the stream buffer
 *  @param stream an #AislStream instance
 *  @param format of the HTTP response
 *  @return a length of data added to the stream buffer
 */
int
aisl_printf(AislStream stream, const char *format, ...);


/** @brief Adds vprintf-like formatted HTTP response to the stream buffer
 *  @param stream an #AislStream instance
 *  @param format of the HTTP response
 *  @param args list for HTTP response construction
 *  @return a length of data added to the stream buffer
 */
int
aisl_vprintf(AislStream stream, const char *format, va_list args);


/** @brief Adds data of the HTTP response to the stream buffer
 *  @param stream an #AislStream instance
 *  @param data a pointer to HTTP response data array
 *  @param d_len size of the HTTP response data array
 *  @return a length of data added to the stream buffer
 */
int
aisl_write(AislStream stream, const char *data, int d_len);


/** @brief Adds a null-terminated string to the stream buffer
 *  @param str_data the HTTP response string
 *  @param stream an #AislStream instance
 *  @return a length of data added to the stream buffer
 */
int
aisl_puts(const char *str_data, AislStream stream);


/** @brief Switches triggering of #AISL_EVENT_STREAM_OUTPUT
 *  @param stream an #AislStream instance
 *  @param value a true to enable or false to disable (default) triggering
 */
void
aisl_set_output_event(AislStream stream, bool value);


/** @brief Gets state of the #AISL_EVENT_STREAM_OUTPUT triggering
 *  @param stream an #AislStream instance
 *  @return true if triggering is enabled and flase otherwise
 */
bool
aisl_get_output_event(AislStream stream);

#endif /* !AISL_STREAM_H */
