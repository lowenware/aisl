#ifndef _AISL_GLOBALS_H_
#define _AISL_GLOBALS_H_

#pragma GCC diagnostic ignored "-Wuninitialized"

#include <aisl/handle.h>

/* MACOS FIX AND OTHER OS */
#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK 0
#endif

#ifndef AISL_MIN_SERVERS
#define AISL_MIN_SERVERS   1
#endif

#ifndef AISL_MIN_STREAMS
#define AISL_MIN_STREAMS   1
#endif

#ifndef AISL_MIN_CLIENTS
#define AISL_MIN_CLIENTS   32
#endif

#ifndef AISL_MIN_LISTENERS
#define AISL_MIN_LISTENERS 8
#endif

#ifndef AISL_MIN_DELAYS
#define AISL_MIN_DELAYS    8
#endif

#ifndef AISL_BUFFER_SIZE
#define AISL_BUFFER_SIZE 4096
#endif

#ifndef AISL_MIN_HEADERS
#define AISL_MIN_HEADERS 8
#endif

#ifndef AISL_MAX_CLIENT_SILENCE
#define AISL_MAX_CLIENT_SILENCE 10
#endif


extern aisl_handle_t gHandle;

#endif
