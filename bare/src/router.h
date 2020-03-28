#ifndef ROUTER_H_3B262BA5_8D40_4139_A302_680DAE9E29F8
#define ROUTER_H_3B262BA5_8D40_4139_A302_680DAE9E29F8

#include <aisl/aisl.h>

int
router_init(void);

void
router_callback(const struct aisl_evt *evt, void *p_ctx);

void
router_reply_error(AislStream stream, AislHttpResponse http_response);

#endif /* !ROUTER_H */
