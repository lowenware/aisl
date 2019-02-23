#ifndef _AISL_STATUS_H_
#define _AISL_STATUS_H_

/* -------------------------------------------------------------------------- */

typedef enum {

  AISL_EXTCALL_ERROR  = -3,
  AISL_SYSCALL_ERROR  = -2,
  AISL_MALLOC_ERROR   = -1,

  AISL_SUCCESS        = 0,
  AISL_IDLE           = 1

} aisl_status_t;

/* -------------------------------------------------------------------------- */

const char *
aisl_status_to_string(aisl_status_t status);

/* -------------------------------------------------------------------------- */

#endif
