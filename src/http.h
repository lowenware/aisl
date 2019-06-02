/******************************************************************************
 *
 *                Copyright (c) 2017-%YEAR% by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file http.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of HTTP parser module
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef AISL_HTTP_H_E2921FF1_6BD3_4E60_B599_ACA1A494CD01
#define AISL_HTTP_H_E2921FF1_6BD3_4E60_B599_ACA1A494CD01

#include <aisl/types.h>

typedef enum {
	  HTTP_PARSER_SUCCESS
	, HTTP_PARSER_READY
	, HTTP_PARSER_HUNGRY
	, HTTP_PARSER_ERROR
} ParserStatus;


ParserStatus
http_10_parse_request(char *data, int32_t *size, AislStream stream);


ParserStatus
http_10_parse_header(char *data, int32_t *size, AislStream stream);


ParserStatus
http_10_parse_body(char *data, int32_t *size, AislStream stream);


#endif /* !AISL_HTTP_H */
