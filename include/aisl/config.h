/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file aisl/config.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of AISL configuration structures
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef AISL_CONFIG_H_DB67A89B_5CAF_4A5F_AEB1_6DB9F84827D6
#define AISL_CONFIG_H_DB67A89B_5CAF_4A5F_AEB1_6DB9F84827D6

#include <aisl/types.h>

#define AISL_CFG_DEFAULTS \
	  .client_spool_size      = 32    \
	, .initial_buffer_size    = 16536 \
	, .client_accept_limit    = 1024  \
	, .client_silence_timeout = 30    \


/** @brief Server configuration structure
 */
struct aisl_cfg_srv {
	const char *host;         /**< server IP to listen */
	uint16_t    port;         /**< server port to listen */
	bool        secure;       /**< shall server use TLS */
};


/** @brief SSL configuration structure
 */
struct aisl_cfg_ssl {
	const char *host;         /**< secure server hostname */
	const char *key_file;     /**< path to SSL key file */
	const char *crt_file;     /**< path to SSL certificate file */
};


/** @brief AISL initial configuration structure
 */
struct aisl_cfg {
	AislCallback callback;         /**< A pointer to #AislCallback event handler */
	void *p_ctx;                   /**< User defined context for #AislCallback */

	const struct aisl_cfg_srv *srv;      /**< A pointer to array of #aisl_cfg_srv */
	const struct aisl_cfg_ssl *ssl;      /**< A pointer to array of #aisl_cfg_ssl  */

	int srv_cnt;                   /**< Size of #aisl_cfg_srv array */
	int ssl_cnt;                   /**< Size of #aisl_cfg_ssl array */

	int client_spool_size;         /**< Initial size of client spool */
	int initial_buffer_size;       /**< Initial size of communication buffer */
	int client_accept_limit;       /**< Maximal number of concurent clients */
	int client_silence_timeout;    /**< Client silence timeout */
};

#endif /* !AISL_CONFIG_H */
