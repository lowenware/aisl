/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file types.c
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Types stringifiers module
 *
 * @see https://lowenware.com/aisl/
 */

#include <aisl/types.h>


#if AISL_WITH_STRINGIFIERS == 1

__attribute__ ((visibility ("default") ))
const char *
aisl_status_to_string(AislStatus status)
{
	switch(status) {
  case AISL_INPUT_ERROR:   return "AISL_INPUT_ERROR";
	case AISL_EXTCALL_ERROR: return "AISL_EXTCALL_ERROR";
	case AISL_SYSCALL_ERROR: return "AISL_SYSCALL_ERROR";
	case AISL_MALLOC_ERROR:  return "AISL_MALLOC_ERROR";
	case AISL_SUCCESS:       return "AISL_SUCCESS";
	case AISL_IDLE:          return "AISL_IDLE";
	}
	return "UNKNOWN";
}


__attribute__ ((visibility ("default") ))
const char *
aisl_event_to_string(AislEvent evt_code)
{
	switch(evt_code) {
	case AISL_EVENT_SERVER_READY:        return "SERVER READY";
	case AISL_EVENT_SERVER_ERROR:        return "SERVER ERROR";
	case AISL_EVENT_CLIENT_CONNECT:      return "CLIENT CONNECT";
	case AISL_EVENT_CLIENT_DISCONNECT:   return "CLIENT DISCONNECT";
	case AISL_EVENT_STREAM_OPEN:         return "STREAM OPEN";
	case AISL_EVENT_STREAM_HEADER:       return "STREAM HEADER";
	case AISL_EVENT_STREAM_INPUT:        return "STREAM INPUT";
	case AISL_EVENT_STREAM_REQUEST:      return "STREAM REQUEST";
	case AISL_EVENT_STREAM_OUTPUT:       return "STREAM OUTPUT";
	case AISL_EVENT_STREAM_CLOSE:        return "STREAM CLOSE";
	case AISL_EVENT_STREAM_ERROR:        return "STREAM ERROR";
	}

	return "UNKNOWN";
}

#endif
