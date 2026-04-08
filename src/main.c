#include "lwip/opt.h"
#include "napt.h"
#include "lwip/ip.h"

#if LWIP_IPV4 && IP_NAPT

/* Importamos las funciones reales del SDK de Espressif */
extern err_t ip_napt_init(uint16_t max_tcp, uint16_t max_udp);
extern err_t ip_napt_enable_no(uint8_t if_idx, uint8_t enable);

#else

/* Si por algo no está activo el flag, dejamos las vacías para no romper la compilación */
err_t ip_napt_init(uint16_t max_tcp, uint16_t max_udp) { return ERR_OK; }
err_t ip_napt_enable_no(uint8_t if_idx, uint8_t enable) { return ERR_OK; }

#endif
