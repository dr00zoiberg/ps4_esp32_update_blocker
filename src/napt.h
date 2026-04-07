#ifndef _NAPT_H_
#define _NAPT_H_

#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"


#ifdef __cplusplus
extern "C" {
#endif

err_t ip_napt_init(uint16_t max_tcp, uint16_t max_udp);
err_t ip_napt_enable_no(uint8_t if_idx, uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif

