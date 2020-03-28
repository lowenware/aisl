#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "welcome.h"

#define MODULE "welcome"

struct config {
	const char *path;
};

struct context {
	struct aisl_context parent;
};

static struct config *
_new_config(const struct welcome_config *init_config)
{
	struct config *config = calloc(1, sizeof(*config));

	if (config) {
		config->path = init_config->path;
	}
	
	return config;
}

static void
_free_config(struct config *config)
{
	if (config) {
		free(config);
	}
}

static struct context *
_new_context(void)
{
	struct context *ctx = calloc(1, sizeof(*ctx));

	if (ctx) {
		ctx->parent.magic = AISL_CONTEXT_MAGIC;
	}

	return ctx;
}

static void
_free_context(struct context *ctx)
{
	if (ctx) {
		free(ctx);
	}
}


int
welcome_init(struct aisl_module *module, const struct welcome_config *config)
{
	module->config = _new_config(config);

	if (!module->config) {
		fprintf(stderr, MODULE ": could not initialize config\n");
		return -1;
	}

	module->name = MODULE;
	module->on_open = welcome_on_open;
	module->on_header = NULL;
	module->on_input = NULL;
	module->on_request = welcome_on_response;
	module->on_output = NULL;
	module->on_close = welcome_on_close;

	return 0;
}

void
welcome_release(struct aisl_module *module)
{
	_free_config(module->config);
	memset(module, 0, sizeof(*module));
}

AislHttpResponse
welcome_on_open(AislModuleConfig config, AislStream stream
		, const char *path, const char *query , AislHttpMethod method)
{
	struct context *ctx;

	if (strcmp(path, config->path))
		return AISL_HTTP_NOT_FOUND;

	if (!(ctx = _new_context()))
		return AISL_HTTP_INTERNAL_SERVER_ERROR;

	aisl_set_context(stream, ctx);

	return 0;
}

void
welcome_on_response(AislModuleConfig config, AislStream stream)
{
	const char html[] = 
		"<html>"
			"<head>"
				"<title>Welcome World</title>"
				"<style>"
					"body {width: 100%; height: 100%}"
					"h1 {font-size: 4em}"
					".welcome-world {"
						"position: absolute;"
						"top: 50%;"
						"left: 50%;"
						"width: 640px;"
						"height:200px;"
						"margin: -100px 0 0 -320px;"
						"text-align: center;"
					"}"
				"</style>"
			"</head>"
			"<body>"
				"<div class=\"welcome-world\">"
					"<h1>Welcome World</h1>"
					"<p>I am " TARGET_NAME "</p>"
				"</div>"
			"</body>"
		"</html>";

	if (aisl_response(stream, AISL_HTTP_OK, sizeof(html) - 1) == AISL_SUCCESS) {
		if (aisl_write(stream, html, sizeof(html) - 1) != -1) {
			aisl_flush(stream);
			return;
		}
	}

	aisl_reject(stream);
}

void
welcome_on_close(AislModuleConfig config, AislStream stream)
{
	struct context *ctx = aisl_get_context(stream);
	
	if (ctx) {
		_free_context(ctx);
	}
}
