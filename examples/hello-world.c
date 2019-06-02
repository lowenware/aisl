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


#define DEFAULT_HTTP_PORT 8080 /**< Default HTTP server port */


static void
hello_world(const struct aisl_evt *evt, void *p_ctx)
{
  if (evt->code != AISL_EVENT_STREAM_REQUEST)
    return;

  AislStatus status;

  AislStream s = evt->source;

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

  status = aisl_response(s, AISL_HTTP_OK, sizeof (html)-1);

  if (status == AISL_SUCCESS)
  {
    if (aisl_write(s, html, sizeof (html)-1) != -1)
    {
      aisl_flush(s);
    }
    else
      aisl_reject(s);
  }

  (void) p_ctx;
}


int
main(int argc, char ** argv)
{
  AislInstance              aisl;    /**< AISL instance pointer */
  AislStatus       status;  /**< AISL status code */
  struct aisl_cfg     cfg = AISL_CFG_DEFAULT;
  struct aisl_cfg_srv srv = {
    .host   = "0.0.0.0",
    .port   = DEFAULT_HTTP_PORT,
    .secure = false
  };

  cfg.srv      = &srv;
  cfg.srv_cnt  = 1;
  cfg.callback = hello_world;

  /* Initialize instance */
  if ( (aisl = aisl_new(&cfg)) != NULL )
  {
    /* launch application loop */
    fprintf(stdout, "Entering main loop" );
    for(;;)
    {
      status = aisl_run_cycle(aisl);

      if ( status != AISL_SUCCESS )
        aisl_sleep(aisl, 500);
    }

    aisl_free(aisl);
  }
  else
    fprintf(stderr, "Failed to initialize AISL");

  return 0;
}

