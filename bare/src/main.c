#include <stdlib.h>
#include <stdio.h>

#include <aisl/aisl.h>

#include "router.h"

static const struct aisl_cfg_srv m_srv = {
	.host = "0.0.0.0",
	.port = 8081
};

static const struct aisl_cfg m_cfg = {
	AISL_CFG_DEFAULTS,
	.srv = &m_srv,
	.srv_cnt = 1,
	.callback = router_callback,
	.p_ctx = NULL
};

int
main(int argc, char **argv)
{
	AislInstance aisl;
	AislStatus status;

	if (!(aisl = aisl_new(&m_cfg))) {
		fprintf(stderr, "could not initialize AISL\n");
		return -1;
	}

	if (router_init()) {
		fprintf(stderr, "could not initialize router\n");
		return -1;
	}

	printf("--------------------------------------------------------------------------------\n");
	printf("Starting " TARGET_NAME "\n");

	for (;;) {
		status = aisl_run_cycle(aisl);
		if (status != AISL_SUCCESS) {
			aisl_sleep(aisl, 500);
		}
	}

	aisl_free(aisl);

	return 0;
}

