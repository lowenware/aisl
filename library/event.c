#include <aisl/event.h>

__attribute__ ((visibility ("default") ))
const char *
aisl_event_get_text( aisl_event_t e_id )
{
  switch(e_id)
  {
    case AISL_SERVER_OPEN: return "AISL_SERVER_OPEN";
    case AISL_SERVER_ERROR: return "AISL_SERVER_ERROR";

    case AISL_CLIENT_CONNECT: return "AISL_CLIENT_CONNECT";
    case AISL_CLIENT_DISCONNECT: return "AISL_CLIENT_DISCONNECT";
    case AISL_CLIENT_TIMEOUT: return "AISL_CLIENT_TIMEOUT";

    case AISL_STREAM_OPEN: return "AISL_STREAM_OPEN";   
    case AISL_STREAM_INPUT: return "AISL_STREAM_INPUT"; 
    case AISL_STREAM_REQUEST: return "AISL_STREAM_REQUEST";
    case AISL_STREAM_OUTPUT: return "AISL_STREAM_OUTPUT";
    case AISL_STREAM_CLOSE: return "AISL_STREAM_CLOSE";
    case AISL_STREAM_ERROR: return "AISL_STREAM_ERROR"; 
    default:
      break;
  }
  return "AISL_CUSTOM_EVENT";
}


