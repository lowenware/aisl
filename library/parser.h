#ifndef _AISL_PARSER_H_
#define _AISL_PARSER_H_

#include <aisl/aisl.h>
#include "client.h"


/* parser status ------------------------------------------------------------ */

typedef enum
{
  PARSER_PENDING,     /* process pending */
  PARSER_FINISHED,    /* successful finish */
  PARSER_HUNGRY,      /* not enough data in buffer */
  PARSER_FAILED       /* error happened */

} parser_status_t;


/* parse HTTP Request ------------------------------------------------------- */

parser_status_t
parse_request_method(client_t cli,  char ** b_ptr, int *b_len);

/* parse HTTP Request URI --------------------------------------------------- */

parser_status_t
parse_request_path(client_t cli, char ** b_ptr, int *b_len);

/* parse HTTP Version ------------------------------------------------------- */

parser_status_t
parse_request_protocol(client_t cli, char ** b_ptr, int *b_len);

/* parse HTTP header key ---------------------------------------------------- */

parser_status_t
parse_request_header_key(client_t cli, char ** b_ptr, int *b_len);

/* parse HTTP header value -------------------------------------------------- */

parser_status_t
parse_request_header_value(client_t cli, char ** b_ptr, int *b_len);

/* parse HTTP data ---------------------------------------------------------- */

parser_status_t
parse_request_content(client_t cli, char ** b_ptr, int *b_len);

/* -------------------------------------------------------------------------- */

#endif
