/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file aisl/types.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of AISL types
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef AISL_TYPES_H_86A9DBA7_C0E6_4CF4_8A64_DAAD4A81031B
#define AISL_TYPES_H_86A9DBA7_C0E6_4CF4_8A64_DAAD4A81031B

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#ifndef AISL_WITHOUT_SSL
#include <openssl/ssl.h>
#endif

#define AISL_AUTO_LENGTH (~0)

/** type casts */
#define AISL_CALLBACK(x) ((AislCallback) x)


/** @brief AISL Instance */
typedef struct aisl_instance * AislInstance;

/** @brief HTTP(s) Server */
typedef struct aisl_server * AislServer;

/** @brief HTTP(s) Client */
typedef struct aisl_client * AislClient;

/** @brief Server<->Client Stream */
typedef struct aisl_stream * AislStream;


/** status return codes */
typedef enum {
	  AISL_INPUT_ERROR    = -4
	, AISL_EXTCALL_ERROR  = -3
	, AISL_SYSCALL_ERROR  = -2
	, AISL_MALLOC_ERROR   = -1

	, AISL_SUCCESS        = 0
	, AISL_IDLE           = 1
} AislStatus;

#ifndef WITHOUT_STRINGIFIERS

/** @brief Converts #AislStatus code to a null terminated string
 *  @param status an #AislStatus code
 *  @return pointer to the string representing #AislStatus
 */
const char *
aisl_status_to_string(AislStatus status);

#endif


/** @brief HTTP version enumeration */
typedef enum {
	  AISL_HTTP_0_9 = 0x0009
	, AISL_HTTP_1_0 = 0x0100
	, AISL_HTTP_1_1 = 0x0101
	, AISL_HTTP_2_0 = 0x0200
} AislHttpVersion;


/** @brief Converts #AislHttpVersion code to a null terminated string
 *  @param version an #AislHttpVersion code
 *  @return pointer to the string representing #AislHttpVersion
 */
const char *
aisl_http_version_to_string(AislHttpVersion version);


/** HTTP method enumeration */
typedef enum {
	  AISL_HTTP_METHOD_UNKNOWN
	, AISL_HTTP_GET
	, AISL_HTTP_PUT
	, AISL_HTTP_POST
	, AISL_HTTP_HEAD
	, AISL_HTTP_TRACE
	, AISL_HTTP_DELETE
	, AISL_HTTP_OPTIONS
	, AISL_HTTP_CONNECT
	, AISL_HTTP_PRI
} AislHttpMethod;


/** @brief Converts #AislHttpMethod code to a null terminated string
 *  @param method an #AislHttpMethod code
 *  @return pointer to the string representing #AislHttpMethod
 */
const char *
aisl_http_method_to_string( AislHttpMethod method );


/** @brief HTTP response status enumeration */
typedef enum {
	  AISL_HTTP_CONTINUE = 100
	, AISL_HTTP_SWITCHING_PROTOCOLS

	, AISL_HTTP_OK = 200
	, AISL_HTTP_CREATED
	, AISL_HTTP_ACCEPTED
	, AISL_HTTP_NON_AUTHORITATIVE_INFORMATION
	, AISL_HTTP_NO_CONTENT
	, AISL_HTTP_RESET_CONTENT
	, AISL_HTTP_PARTIAL_CONTENT

	, AISL_HTTP_MULTIPLE_CHOICES = 300
	, AISL_HTTP_MOVED_PERMANENTLY
	, AISL_HTTP_FOUND
	, AISL_HTTP_SEE_OTHER
	, AISL_HTTP_NOT_MODIFIED
	, AISL_HTTP_USE_PROXY
	, AISL_HTTP_UNUSED
	, AISL_HTTP_TEMPORARY_REDIRECT

	, AISL_HTTP_BAD_REQUEST = 400
	, AISL_HTTP_UNAUTHORIZED
	, AISL_HTTP_PAYMENT_REQUIRED
	, AISL_HTTP_FORBIDDEN
	, AISL_HTTP_NOT_FOUND
	, AISL_HTTP_METHOD_NOT_ALLOWED
	, AISL_HTTP_NOT_ACCEPTABLE
	, AISL_HTTP_PROXY_AUTHENTICATION_REQUIRED
	, AISL_HTTP_REQUEST_TIMEOUT
	, AISL_HTTP_CONFLICT
	, AISL_HTTP_GONE
	, AISL_HTTP_LENGTH_REQUIRED
	, AISL_HTTP_PRECONDITION_FAILED
	, AISL_HTTP_REQUEST_ENTITY_TOO_LARGE
	, AISL_HTTP_REQUEST_URI_TOO_LONG
	, AISL_HTTP_UNSUPPORTED_MEDIA_TYPE
	, AISL_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE
	, AISL_HTTP_EXPECTATION_FAILED

	, AISL_HTTP_INTERNAL_SERVER_ERROR = 500
	, AISL_HTTP_NOT_IMPLEMENTED
	, AISL_HTTP_BAD_GATEWAY
	, AISL_HTTP_SERVICE_UNAVAILABLE
	, AISL_HTTP_GATEWAY_TIMEOUT
	, AISL_HTTP_VERSION_NOT_SUPPORTED
} AislHttpResponse;


/** @brief Converts #AislHttpResponse code to a null terminated string
 *  @param code an #AislHttpResponse code
 *  @return pointer to the string representing #AislHttpResponse
 */
const char *
aisl_http_response_to_string( AislHttpResponse code );


/** @brief AISL events enumeration */
typedef enum {
	  AISL_EVENT_SERVER_READY       = 100
	, AISL_EVENT_SERVER_ERROR       = 190

	, AISL_EVENT_CLIENT_CONNECT     = 200
	, AISL_EVENT_CLIENT_DISCONNECT  = 210

	, AISL_EVENT_STREAM_OPEN        = 300
	, AISL_EVENT_STREAM_HEADER      = 310
	, AISL_EVENT_STREAM_INPUT       = 320
	, AISL_EVENT_STREAM_REQUEST     = 330
	, AISL_EVENT_STREAM_OUTPUT      = 340
	, AISL_EVENT_STREAM_CLOSE       = 350
	, AISL_EVENT_STREAM_ERROR       = 390
} AislEvent;


/** @brief generic AISL event structure  */
struct aisl_evt {
	void         *source;    /**< Pointer to an event source: #AislServer, #AislClient, #AislStream */
	AislEvent     code;      /**< Event code */
	AislStatus    status;    /**< Event status */
};


/** @brief event handler callback definition
 *  @param evt a pointer to an #aisl_evt structure 
 *  @param ctx a pointer to a context defined by user (see #aisl_cfg)
 */
typedef void
(* AislCallback) (const struct aisl_evt *evt, void *ctx);


/** @brief AISL event structue passed on stream opening */
struct aisl_evt_open {
	struct aisl_evt   evt;           /**< generic #aisl_evt structure */
	const char       *path;          /**< HTTP request path */
	const char       *query;         /**< HTTP request query (GET params) */
	AislHttpMethod    http_method;   /**< HTTP request method */
};


/** @brief AISL event structue passed on HTTP header reception */
struct aisl_evt_header {
	struct aisl_evt   evt;           /**< generic #aisl_evt structure */
	const char       *key;           /**< low case HTTP header name */
	const char       *value;         /**< HTTP header string */
};


/** @brief AISL event structue passed on HTTP request data part received */
struct aisl_evt_input {
	struct aisl_evt   evt;           /**< generic #aisl_evt structure */
	const char       *data;          /**< a pointer to received data array */
	int32_t           size;          /**< data array size */
};


#ifndef WITHOUT_STRINGIFIERS


/** @brief Converts #AislEvent code to a null terminated string
 *  @param evt an #AislEvent code
 *  @return pointer to the string representing #AislEvent
 */
const char *
aisl_event_to_string(AislEvent evt);

#endif

#endif /* !AISL_TYPES_H */