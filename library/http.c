#include <aisl/http.h>

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
const char *
aisl_http_version_to_string(aisl_http_version_t version)
{
  switch (version)
  {
    case AISL_HTTP_1_0: return "HTTP/1.0";
    case AISL_HTTP_1_1: return "HTTP/1.1";
    case AISL_HTTP_2_0: return "HTTP/2.0";
  }
  return "";
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
const char *
aisl_http_response_to_string(aisl_http_response_t code)
{
  switch (code)
  {
    /* most common for faster behavior */
    case AISL_HTTP_OK: return "OK";
    case AISL_HTTP_MOVED_PERMANENTLY: return "Moved Permanently";

    /* informational */
    case AISL_HTTP_CONTINUE: return "Continue";
    case AISL_HTTP_SWITCHING_PROTOCOLS: return "Switching Protocols";
    /* Successful */
    case AISL_HTTP_CREATED: return "Created";
    case AISL_HTTP_ACCEPTED: return "Accepted";
    case AISL_HTTP_NON_AUTHORITATIVE_INFORMATION: return "Non-Authoritative Information";
    case AISL_HTTP_NO_CONTENT: return "No Content";
    case AISL_HTTP_RESET_CONTENT: return "Reset Content";
    case AISL_HTTP_PARTIAL_CONTENT: return "Partial Content";
    /* redirection */
    case AISL_HTTP_MULTIPLE_CHOICES: return "Multiple Choices";
    case AISL_HTTP_FOUND: return "Found";
    case AISL_HTTP_SEE_OTHER: return "See other";
    case AISL_HTTP_NOT_MODIFIED: return "Not Modified";
    case AISL_HTTP_USE_PROXY: return "Use Proxy";
    case AISL_HTTP_UNUSED: return "(unused)";
    case AISL_HTTP_TEMPORARY_REDIRECT: return "Temporary Redirect";
    /* client error */
    case AISL_HTTP_BAD_REQUEST: return "Bad Request";
    case AISL_HTTP_UNAUTHORIZED: return "Unauthorized";
    case AISL_HTTP_PAYMENT_REQUIRED: return "Payment Required";
    case AISL_HTTP_FORBIDDEN: return "Forbidden";
    case AISL_HTTP_NOT_FOUND: return "Not Found";
    case AISL_HTTP_METHOD_NOT_ALLOWED: return "Method Not Allowed";
    case AISL_HTTP_NOT_ACCEPTABLE: return "Not Acceptable";
    case AISL_HTTP_PROXY_AUTHENTICATION_REQUIRED: return "Proxy Authentication Required";
    case AISL_HTTP_REQUEST_TIMEOUT: return "Request Timeout";
    case AISL_HTTP_CONFLICT: return "Conflict";
    case AISL_HTTP_GONE: return "Gone";
    case AISL_HTTP_LENGTH_REQUIRED: return "Length Required";
    case AISL_HTTP_PRECONDITION_FAILED: return "Precondition Failed";
    case AISL_HTTP_REQUEST_ENTITY_TOO_LARGE: return "Request Entity Too Large";
    case AISL_HTTP_REQUEST_URI_TOO_LONG: return "Request-URI Too Long";
    case AISL_HTTP_UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
    case AISL_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE: return "Requested Range Not Satisfiable";
    case AISL_HTTP_EXPECTATION_FAILED: return "Expectation Failed";
    /* server error */
    case AISL_HTTP_INTERNAL_SERVER_ERROR: return "Internal Server Error";
    case AISL_HTTP_NOT_IMPLEMENTED: return "Not Implemented";
    case AISL_HTTP_BAD_GATEWAY: return "Bad Gateway";
    case AISL_HTTP_SERVICE_UNAVAILABLE: return "Service Unavailable";
    case AISL_HTTP_GATEWAY_TIMEOUT: return "Gateway Timeout";
    case AISL_HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";

  }
  return "";

}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
const char *
aisl_http_secure_to_string( int is_secure )
{
  return (is_secure ? "HTTPS" : "HTTP");
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
const char *
aisl_http_method_to_string( aisl_http_method_t method )
{
  switch(method)
  {
    case AISL_HTTP_GET:     return "GET";
    case AISL_HTTP_PUT:     return "PUT";
    case AISL_HTTP_POST:    return "POST";
    case AISL_HTTP_HEAD:    return "HEAD";
    case AISL_HTTP_TRACE:   return "TRACE";
    case AISL_HTTP_DELETE:  return "DELETE";
    case AISL_HTTP_OPTIONS: return "OPTIONS";
    case AISL_HTTP_CONNECT: return "CONNECT";

    case AISL_HTTP_PRI:     return "PRI";
  }

  return "";
}

/* -------------------------------------------------------------------------- */
