#ifndef WELCOME_H_15339D23_C939_4719_8929_686392ED9B4A
#define WELCOME_H_15339D23_C939_4719_8929_686392ED9B4A

#include <aisl/aisl.h>

struct welcome_config {
	const char *path;
};

int
welcome_init(struct aisl_module *module, const struct welcome_config *init_config);

void
welcome_release(struct aisl_module *module);

AislHttpResponse
welcome_on_open(AislModuleConfig config, AislStream stream
		, const char *path, const char *query , AislHttpMethod method);

void
welcome_on_response(AislModuleConfig config, AislStream stream);

void
welcome_on_close(AislModuleConfig config, AislStream stream);

#endif /* !WELCOME_H */
