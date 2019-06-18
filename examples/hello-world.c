/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file hello-world.c
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief AISL usage example: Hello World
 *
 * @see https://lowenware.com/aisl/
 */

#include <stdio.h>
#include <stdlib.h>

/* Include library meta header */
#include <aisl/aisl.h>


static void
hello_world(const struct aisl_evt *evt, void *p_ctx);


static const struct aisl_cfg_srv m_srv[] = {{
	.host   = "0.0.0.0",
	.port   = 8080,
	.secure = false
}};


static const struct aisl_cfg m_cfg = {
	  AISL_CFG_DEFAULTS
	, .srv = m_srv
	, .srv_cnt = sizeof (m_srv) / sizeof (m_srv[0])
	, .ssl = NULL
	, .ssl_cnt = 0
	, .callback = hello_world
	, .p_ctx = NULL
};


static void
hello_world(const struct aisl_evt *evt, void *p_ctx)
{
	AislStream s;

	const char html[] = 
		"<html>"
			"<head>"
				"<title>Hello World</title>"
			"</head>"
			"<body>"
				"<h1>Hello World</h1>"
				"<p>Powered by AISL</p>"
			"</body>"
		"</html>";
	
	fprintf(stdout, "Event: %s\n", aisl_event_to_string(evt->code) );

	if (evt->code != AISL_EVENT_STREAM_REQUEST)
		return;

	s = evt->source;

	if (aisl_response(s, AISL_HTTP_OK, sizeof (html)-1) == AISL_SUCCESS) {
		if (aisl_write(s, html, sizeof (html)-1) != -1) {
			aisl_flush(s);
			return;
		}
	}

	aisl_reject(s);
	(void) p_ctx;
}


int
main(int argc, char **argv)
{
	AislInstance aisl;    /**< AISL instance pointer */
	AislStatus status;  /**< AISL status code */

	/* Initialize instance */
	if ( (aisl = aisl_new(&m_cfg)) != NULL ) {
		/* launch application loop */
		fprintf(stdout, "Entering main loop\n" );
		for(;;) {
			status = aisl_run_cycle(aisl);

			if ( status != AISL_SUCCESS )
				aisl_sleep(aisl, 500);
		}

		aisl_free(aisl);
	} else {
		fprintf(stderr, "Failed to initialize AISL\n");
	}

	return 0;
}

