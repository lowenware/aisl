#include <aisl/status.h>

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
const char *
aisl_status_to_string(aisl_status_t status)
{
  switch( status )
  {
    case AISL_SUCCESS:             return "success";
    case AISL_IDLE:                return "idle";
    case AISL_MALLOC_ERROR:        return "malloc error";
    case AISL_SYSCALL_ERROR:       return "system call error";
    case AISL_EXTCALL_ERROR:       return "external call error";
  }

  return "";
}

/* -------------------------------------------------------------------------- */
