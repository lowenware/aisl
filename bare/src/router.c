#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <aisl/aisl.h>

#include "welcome.h"
#include "router.h"

enum {
	MODULE_WELCOME,
	MODULE__COUNT
};

static struct aisl_module m_modules[MODULE__COUNT];

static void
on_stream_open(AislStream stream, const struct aisl_evt_open *evt)
{
	uint32_t modules_count = sizeof(m_modules) / sizeof(m_modules[0]);
	int ret_code = AISL_HTTP_NOT_FOUND, i;

	for (i = 0; i < modules_count; i++) {
		struct aisl_module *module = &m_modules[i];
		struct aisl_context *ctx;

		ret_code = module->on_open(module->config, stream, evt->path, evt->query, evt->http_method);

		switch (ret_code) {
		case 0: /* Found */
			ctx = aisl_get_context(stream);
			if (ctx && ctx->magic == AISL_CONTEXT_MAGIC) {
				ctx->module_id = i;
			}
			return;

		case AISL_HTTP_NOT_FOUND:
			continue;

		default:
			break;
		}
		break;
	}

	router_reply_error(stream, ret_code);
}

static struct aisl_module *
get_module_from_stream(AislStream stream)
{
	uint32_t modules_count = sizeof(m_modules) / sizeof(m_modules[0]);
	struct aisl_context *ctx = aisl_get_context(stream);

	if (ctx->magic == AISL_CONTEXT_MAGIC && ctx->module_id < modules_count) {
		return &m_modules[ ctx->module_id ];
	}

	return NULL;
}

static void
on_stream_header(AislStream stream, const struct aisl_evt_header *evt)
{
	struct aisl_module *module = get_module_from_stream(stream);

	if (module && module->on_header) {
		module->on_header(module->config, stream, evt->key, evt->value);
	}
}

static void
on_stream_input(AislStream stream, const struct aisl_evt_input *evt)
{
	struct aisl_module *module = get_module_from_stream(stream);

	if (module && module->on_input) {
		module->on_input(module->config, stream, evt->data, evt->size);
	}
}

static void
on_stream_request(AislStream stream)
{
	struct aisl_module *module = get_module_from_stream(stream);

	if (module && module->on_request) {
		module->on_request(module->config, stream);
	}
}

static void
on_stream_output(AislStream stream)
{
	struct aisl_module *module = get_module_from_stream(stream);

	if (module && module->on_output) {
		module->on_output(module->config, stream);
	}
}

static void
on_stream_close(AislStream stream)
{
	struct aisl_module *module = get_module_from_stream(stream);

	if (module && module->on_close) {
		module->on_close(module->config, stream);
	}
}

static void
on_stream_error(AislStream stream, AislStatus status)
{
	struct aisl_module *module = get_module_from_stream(stream);

	fprintf(stderr, "stream error (module: %s, status: %s)\n",
			(module ? module->name : "NULL"), aisl_status_to_string(status));
}

static void
on_server_ready(AislServer server)
{
	struct sockaddr_in addr;

	aisl_server_get_address(server, &addr);

	printf("HTTP server is ready\nOpen http://%s:%d/ in your browser\n"
			, (addr.sin_addr.s_addr ? inet_ntoa(addr.sin_addr) : "localhost")
			, ntohs(addr.sin_port));
}

static void
on_server_error(AislServer server, AislStatus status)
{
	struct sockaddr_in addr;

	aisl_server_get_address(server, &addr);

	fprintf(stderr, "failed to start server at %s:%d (%s, %s)\n"
			, inet_ntoa(addr.sin_addr)
			, ntohs(addr.sin_port)
			, aisl_status_to_string(status)
			, strerror(errno));
}

static void
on_client_event(AislClient client, AislStatus status, const char *word)
{
	struct sockaddr_in addr;

	aisl_client_get_address(client, &addr);
	if (status == AISL_IDLE)
		word = "timeout";

	printf("client %s:%d: %s\n"
			, inet_ntoa(addr.sin_addr)
			, ntohs(addr.sin_port)
			, word);
}

int
router_init(void)
{
	int ret_code;
	struct welcome_config welcome_config;

	memset(&welcome_config, 0, sizeof(welcome_config));
	welcome_config.path = "/";
	ret_code = welcome_init(&m_modules[MODULE_WELCOME], &welcome_config);

	return ret_code;
}

void
router_callback(const struct aisl_evt *evt, void *p_ctx)
{
	switch (evt->code) {
	case AISL_EVENT_SERVER_READY:
		on_server_ready((AislServer)evt->source);
		return;

	case AISL_EVENT_SERVER_ERROR:
		on_server_error((AislServer)evt->source, evt->status);
		return;

	case AISL_EVENT_CLIENT_CONNECT:
		on_client_event((AislClient)evt->source, evt->status, "connected");
		return;

	case AISL_EVENT_CLIENT_DISCONNECT:
		on_client_event((AislClient)evt->source,  evt->status, "disconnected");
		return;

	case AISL_EVENT_STREAM_OPEN:
		on_stream_open((AislStream)evt->source, (struct aisl_evt_open*) evt);
		return;

	case AISL_EVENT_STREAM_HEADER:
		on_stream_header((AislStream)evt->source, (struct aisl_evt_header*) evt);
		return;

	case AISL_EVENT_STREAM_INPUT:
		on_stream_input((AislStream)evt->source, (struct aisl_evt_input*) evt);
		return;

	case AISL_EVENT_STREAM_REQUEST:
		on_stream_request((AislStream)evt->source);
		return;

	case AISL_EVENT_STREAM_OUTPUT:
		on_stream_output((AislStream)evt->source);
		return;

	case AISL_EVENT_STREAM_CLOSE:
		on_stream_close((AislStream)evt->source);
		return;

	case AISL_EVENT_STREAM_ERROR:
		on_stream_error((AislStream)evt->source, evt->status);
		return;

	default:
		break;
	}

	fprintf(stderr, "Unhandled event (%d: %s)\n", evt->code
		, aisl_event_to_string(evt->code));
}

void
router_reply_error(AislStream stream, AislHttpResponse http_response)
{
	const char html[] = 
		"<html>"
			"<head>"
				"<title>%d - %s</title>"
			"</head>"
			"<body>"
				"<h1>%d - %s</h1>"
				"<p>Server by %s</p>"
			"</body>"
		"</html>";

	if (aisl_response(stream, http_response, AISL_AUTO_LENGTH) == AISL_SUCCESS) {
		int ret_code;
		const char *http_status = aisl_http_response_to_string(http_response);

		ret_code = aisl_printf(stream, html, http_response, http_status
				, http_response, http_status);

		if (!ret_code) {
			aisl_flush(stream);
			return;
		}
	}

	aisl_reject(stream);
}
