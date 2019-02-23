#ifndef _AISL_HTTP_H_
#define _AISL_HTTP_H_

/* -------------------------------------------------------------------------- */

typedef enum
{
  AISL_HTTP_1_0,
  AISL_HTTP_1_1,
  AISL_HTTP_2_0

} aisl_http_version_t;

/* -------------------------------------------------------------------------- */

typedef enum {
  AISL_HTTP_GET,
  AISL_HTTP_PUT,
  AISL_HTTP_POST,
  AISL_HTTP_HEAD,
  AISL_HTTP_TRACE,
  AISL_HTTP_DELETE,
  AISL_HTTP_OPTIONS,
  AISL_HTTP_CONNECT,

  AISL_HTTP_PRI

} aisl_http_method_t;

/* -------------------------------------------------------------------------- */

typedef enum 
{
    /* informational ------------------------------ */
    AISL_HTTP_CONTINUE                      = 100,
    AISL_HTTP_SWITCHING_PROTOCOLS,
    /* Successful --------------------------------- */
    AISL_HTTP_OK                            = 200,
    AISL_HTTP_CREATED,
    AISL_HTTP_ACCEPTED,
    AISL_HTTP_NON_AUTHORITATIVE_INFORMATION,
    AISL_HTTP_NO_CONTENT,
    AISL_HTTP_RESET_CONTENT,
    AISL_HTTP_PARTIAL_CONTENT,
    /* redirection -------------------------------- */
    AISL_HTTP_MULTIPLE_CHOICES              = 300,
    AISL_HTTP_MOVED_PERMANENTLY,
    AISL_HTTP_FOUND,
    AISL_HTTP_SEE_OTHER,
    AISL_HTTP_NOT_MODIFIED,
    AISL_HTTP_USE_PROXY,
    AISL_HTTP_UNUSED,
    AISL_HTTP_TEMPORARY_REDIRECT,
    /* client error ------------------------------- */
    AISL_HTTP_BAD_REQUEST                   = 400,
    AISL_HTTP_UNAUTHORIZED,
    AISL_HTTP_PAYMENT_REQUIRED,
    AISL_HTTP_FORBIDDEN,
    AISL_HTTP_NOT_FOUND,
    AISL_HTTP_METHOD_NOT_ALLOWED,
    AISL_HTTP_NOT_ACCEPTABLE,
    AISL_HTTP_PROXY_AUTHENTICATION_REQUIRED,
    AISL_HTTP_REQUEST_TIMEOUT,
    AISL_HTTP_CONFLICT,
    AISL_HTTP_GONE,
    AISL_HTTP_LENGTH_REQUIRED,
    AISL_HTTP_PRECONDITION_FAILED,
    AISL_HTTP_REQUEST_ENTITY_TOO_LARGE,
    AISL_HTTP_REQUEST_URI_TOO_LONG,
    AISL_HTTP_UNSUPPORTED_MEDIA_TYPE,
    AISL_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE,
    AISL_HTTP_EXPECTATION_FAILED,
    /* server error ------------------------------- */
    AISL_HTTP_INTERNAL_SERVER_ERROR          = 500,
    AISL_HTTP_NOT_IMPLEMENTED,
    AISL_HTTP_BAD_GATEWAY,
    AISL_HTTP_SERVICE_UNAVAILABLE,
    AISL_HTTP_GATEWAY_TIMEOUT,
    AISL_HTTP_VERSION_NOT_SUPPORTED

} aisl_http_response_t;
/* -------------------------------------------------------------------------- */

const char *
aisl_http_version_to_string(aisl_http_version_t version);

/* -------------------------------------------------------------------------- */

const char *
aisl_http_response_to_string(aisl_http_response_t code);

/* -------------------------------------------------------------------------- */

const char *
aisl_http_secure_to_string( int is_secure );

/* -------------------------------------------------------------------------- */

const char *
aisl_http_method_to_string( aisl_http_method_t method );

/* -------------------------------------------------------------------------- */

#endif
