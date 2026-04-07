/*
 * SPDX-FileCopyrightText: 2001-2003 Swedish Institute of Computer Science
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SPDX-FileContributor: 2015-2021 Espressif Systems (Shanghai) CO LTD
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include "esp_task.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "netif/dhcp_state.h"
#include "sntp/sntp_get_set_time.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Enable all Espressif-only options */

/*
   -----------------------------------------------
   ---------- Platform specific locking ----------
   -----------------------------------------------
*/
/**
 * LWIP_TCPIP_CORE_LOCKING
 * Creates a global mutex that is held during TCPIP thread operations.
 * Can be locked by client code to perform lwIP operations without changing
 * into TCPIP thread using callbacks. See LOCK_TCPIP_CORE() and
 * UNLOCK_TCPIP_CORE().
 * Your system should provide mutexes supporting priority inversion to use this.
 */
#ifdef CONFIG_LWIP_TCPIP_CORE_LOCKING
#define LWIP_TCPIP_CORE_LOCKING         1
#define LOCK_TCPIP_CORE()     do { sys_mutex_lock(&lock_tcpip_core); sys_thread_tcpip(LWIP_CORE_LOCK_MARK_HOLDER); } while(0)
#define UNLOCK_TCPIP_CORE()   do { sys_thread_tcpip(LWIP_CORE_LOCK_UNMARK_HOLDER); sys_mutex_unlock(&lock_tcpip_core);  } while(0)
#ifdef CONFIG_LWIP_CHECK_THREAD_SAFETY
#define LWIP_ASSERT_CORE_LOCKED() do { LWIP_ASSERT("Required to lock TCPIP core functionality!", sys_thread_tcpip(LWIP_CORE_LOCK_QUERY_HOLDER)); } while(0)
#endif /* CONFIG_LWIP_CHECK_THREAD_SAFETY */

#else
#define LWIP_TCPIP_CORE_LOCKING         0
#ifdef CONFIG_LWIP_CHECK_THREAD_SAFETY
#define LWIP_ASSERT_CORE_LOCKED()     do { LWIP_ASSERT("Required to run in TCPIP context!", sys_thread_tcpip(LWIP_CORE_LOCK_QUERY_HOLDER)); } while(0)
#endif /* CONFIG_LWIP_CHECK_THREAD_SAFETY */
#endif /* CONFIG_LWIP_TCPIP_CORE_LOCKING */

#define LWIP_MARK_TCPIP_THREAD() sys_thread_tcpip(LWIP_CORE_MARK_TCPIP_TASK)

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT        1

/**
 * MEMCPY: override this if you have a faster implementation at hand than the
 * one included in your C library
 */
#define MEMCPY(dst,src,len)             memcpy(dst,src,len)

/**
 * SMEMCPY: override this with care! Some compilers (e.g. gcc) can inline a
 * call to memcpy() if the length is known at compile time and is small.
 */
#define SMEMCPY(dst,src,len)            memcpy(dst,src,len)

#define LWIP_RAND       esp_random

/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
/**
 * MEM_LIBC_MALLOC==1: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#define MEM_LIBC_MALLOC        1

/**
* MEMP_MEM_MALLOC==1: Use mem_malloc/mem_free instead of the lwip pool allocator.
* Especially useful with MEM_LIBC_MALLOC but handle with care regarding execution
* speed and usage from interrupts!
*/
#define MEMP_MEM_MALLOC                 1

/**
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 * 4 byte alignment -> #define MEM_ALIGNMENT 4
 * 2 byte alignment -> #define MEM_ALIGNMENT 2
 */
#define MEM_ALIGNMENT           4

/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/

/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETCONN                CONFIG_LWIP_MAX_SOCKETS

/**
 * MEMP_NUM_RAW_PCB: Number of raw connection PCBs
 * (requires the LWIP_RAW option)
 */
#define MEMP_NUM_RAW_PCB                CONFIG_LWIP_MAX_RAW_PCBS

/**
 * MEMP_NUM_TCP_PCB: the number of simultaneously active TCP connections.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_PCB                CONFIG_LWIP_MAX_ACTIVE_TCP

/**
 * MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP connections.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_PCB_LISTEN         CONFIG_LWIP_MAX_LISTENING_TCP

/**
 * MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
 * per active UDP "connection".
 * (requires the LWIP_UDP option)
 */
#define MEMP_NUM_UDP_PCB                CONFIG_LWIP_MAX_UDP_PCBS

/*
   --------------------------------
   ---------- ARP options -------
   --------------------------------
*/
/**
 * ARP_QUEUEING==1: Multiple outgoing packets are queued during hardware address
 * resolution. By default, only the most recent packet is queued per IP address.
 * This is sufficient for most protocols and mainly reduces TCP connection
 * startup time. Set this to 1 if you know your application sends more than one
 * packet in a row to an IP address that is not in the ARP cache.
 */
#define ARP_QUEUEING                    1

/*
   --------------------------------
   ---------- IP options ----------
   --------------------------------
*/
/**
 * IP_REASSEMBLY==1: Reassemble incoming fragmented IP4 packets. Note that
 * this option does not affect outgoing packet sizes, which can be controlled
 * via IP_FRAG.
 */
#define IP_REASSEMBLY                   CONFIG_LWIP_IP4_REASSEMBLY

/**
 * LWIP_IPV6_REASS==1: reassemble incoming IP6 packets that fragmented. Note that
 * this option does not affect outgoing packet sizes, which can be controlled
 * via LWIP_IPV6_FRAG.
 */
#define LWIP_IPV6_REASS                 CONFIG_LWIP_IP6_REASSEMBLY

/**
 * IP_FRAG==1: Fragment outgoing IP4 packets if their size exceeds MTU. Note
 * that this option does not affect incoming packet sizes, which can be
 * controlled via IP_REASSEMBLY.
 */
#define IP_FRAG                         CONFIG_LWIP_IP4_FRAG

/**
 * LWIP_IPV6_FRAG==1: Fragment outgoing IP6 packets if their size exceeds MTU. Note
 * that this option does not affect incoming packet sizes, which can be
 * controlled via IP_REASSEMBLY.
 */
#define LWIP_IPV6_FRAG                  CONFIG_LWIP_IP6_FRAG

/**
 * IP_REASS_MAXAGE: Maximum time (in multiples of IP_TMR_INTERVAL - so seconds, normally)
 * a fragmented IP packet waits for all fragments to arrive. If not all fragments arrived
 * in this time, the whole packet is discarded.
 */
#define IP_REASS_MAXAGE                 3

/**
 * IP_REASS_MAX_PBUFS: Total maximum amount of pbufs waiting to be reassembled.
 * Since the received pbufs are enqueued, be sure to configure
 * PBUF_POOL_SIZE > IP_REASS_MAX_PBUFS so that the stack is still able to receive
 * packets even if the maximum amount of fragments is enqueued for reassembly!
 */
#define IP_REASS_MAX_PBUFS              10

/**
 * IP_FORWARD==1: Enables the ability to forward IP packets across network
 * interfaces. MODIFICADO PARA NAT STARLINK PS4.
 */
#define IP_FORWARD                      1

/**
 * IP_NAPT==1: Enables IPv4 Network Address and Port Translation.
 * MODIFICADO PARA NAT STARLINK PS4.
 */
#define IP_NAPT                         1
#define IP_NAPT_MAX                     512

/*
   ----------------------------------
   ---------- ICMP options ----------
   ----------------------------------
*/
#define LWIP_ICMP  CONFIG_LWIP_ICMP

#define LWIP_BROADCAST_PING CONFIG_LWIP_BROADCAST_PING

#define LWIP_MULTICAST_PING CONFIG_LWIP_MULTICAST_PING

/*
   ---------------------------------
   ---------- RAW options ----------
   ---------------------------------
*/
/**
 * LWIP_RAW==1: Enable application layer to hook into the IP layer itself.
 */
#define LWIP_RAW                        1

/*
   ----------------------------------
   ---------- DHCP options ----------
   ----------------------------------
*/
/**
 * LWIP_DHCP==1: Enable DHCP module.
 */
#define LWIP_DHCP                       1

#define DHCP_MAXRTX                     0

/**
 * DHCP_DOES_ARP_CHECK==1: Do an ARP check on the offered address.
 */
#define DHCP_DOES_ARP_CHECK             CONFIG_LWIP_DHCP_DOES_ARP_CHECK

/**
 * LWIP_DHCP_DISABLE_CLIENT_ID==1: Do not add option 61 (client-id) to DHCP packets
 */
#define ESP_DHCP_DISABLE_CLIENT_ID      CONFIG_LWIP_DHCP_DISABLE_CLIENT_ID

#define DHCP_DEFINE_CUSTOM_TIMEOUTS     1
/* Since for embedded devices it's not that hard to miss a discover packet, so lower
 * the discover retry backoff time from (2,4,8,16,32,60,60)s to (500m,1,2,4,8,15,15)s.
 */
 #define DHCP_REQUEST_TIMEOUT_SEQUENCE(state, tries)   (state == DHCP_STATE_REQUESTING ? \
                                                       (uint16_t)(1 * 1000) : \
                                                       (uint16_t)(((tries) < 6 ? 1 << (tries) : 60) * 250))

#define DHCP_COARSE_TIMER_SECS CONFIG_LWIP_DHCP_COARSE_TIMER_SECS

static inline uint32_t timeout_from_offered(uint32_t lease, uint32_t min)
{
    uint32_t timeout = lease;
    if (timeout == 0) {
      timeout = min;
    }
    timeout = (timeout + DHCP_COARSE_TIMER_SECS - 1) / DHCP_COARSE_TIMER_SECS;
    return timeout;
}

#define DHCP_CALC_TIMEOUT_FROM_OFFERED_T0_LEASE(dhcp) \
   timeout_from_offered((dhcp)->offered_t0_lease, 120)
#define DHCP_CALC_TIMEOUT_FROM_OFFERED_T1_RENEW(dhcp) \
   timeout_from_offered((dhcp)->offered_t1_renew, (dhcp)->t0_timeout >> 1 /* 50% */)
#define DHCP_CALC_TIMEOUT_FROM_OFFERED_T2_REBIND(dhcp) \
   timeout_from_offered((dhcp)->offered_t2_rebind, ((dhcp)->t0_timeout / 8) * 7 /* 87.5% */)

/**
 * CONFIG_LWIP_DHCP_RESTORE_LAST_IP==1: Last valid IP address obtained from DHCP server
 * is restored after reset/power-up.
 */
#if CONFIG_LWIP_DHCP_RESTORE_LAST_IP

#define LWIP_DHCP_IP_ADDR_RESTORE()     dhcp_ip_addr_restore(netif)
#define LWIP_DHCP_IP_ADDR_STORE()       dhcp_ip_addr_store(netif)
#define LWIP_DHCP_IP_ADDR_ERASE(esp_netif)       dhcp_ip_addr_erase(esp_netif)

#endif

/**
 * CONFIG_LWIP_DHCP_OPTIONS_LEN: The total length of outgoing DHCP option msg. If you have many options
 * and options value is too long, you can configure the length according to your requirements
 */
#define DHCP_OPTIONS_LEN                CONFIG_LWIP_DHCP_OPTIONS_LEN

/**
 * LWIP_DHCP_DISABLE_VENDOR_CLASS_ID==1: Do not add option 60 (Vendor Class Identifier) to DHCP packets
 */
#define ESP_DHCP_DISABLE_VENDOR_CLASS_IDENTIFIER       CONFIG_LWIP_DHCP_DISABLE_VENDOR_CLASS_ID

/*
   ------------------------------------
   ---------- AUTOIP options ----------
   ------------------------------------
*/
#ifdef CONFIG_LWIP_AUTOIP
#define LWIP_AUTOIP                     1

/**
* LWIP_DHCP_AUTOIP_COOP==1: Allow DHCP and AUTOIP to be both enabled on
* the same interface at the same time.
*/
#define LWIP_DHCP_AUTOIP_COOP           1

/**
* LWIP_DHCP_AUTOIP_COOP_TRIES: Set to the number of DHCP DISCOVER probes
* that should be sent before falling back on AUTOIP. This can be set
* as low as 1 to get an AutoIP address very quickly, but you should
* be prepared to handle a changing IP address when DHCP overrides
* AutoIP.
*/
#define LWIP_DHCP_AUTOIP_COOP_TRIES     CONFIG_LWIP_AUTOIP_TRIES

#define LWIP_AUTOIP_MAX_CONFLICTS CONFIG_LWIP_AUTOIP_MAX_CONFLICTS

#define LWIP_AUTOIP_RATE_LIMIT_INTERVAL CONFIG_LWIP_AUTOIP_RATE_LIMIT_INTERVAL

#endif /* CONFIG_LWIP_AUTOIP */

/*
   ----------------------------------
   ---------- SNMP options ----------
   ----------------------------------
*/
/*
   ----------------------------------
   ---------- IGMP options ----------
   ----------------------------------
*/
/**
 * LWIP_IGMP==1: Turn on IGMP module.
 */
#define LWIP_IGMP                       1

/*
   ----------------------------------
   ---------- DNS options -----------
   ----------------------------------
*/
/**
 * LWIP_DNS==1: Turn on DNS module. UDP must be available for DNS
 * transport.
 */
#define LWIP_DNS                        1

#define DNS_MAX_SERVERS                 3
#define DNS_FALLBACK_SERVER_INDEX        (DNS_MAX_SERVERS - 1)

/*
   ---------------------------------
   ---------- UDP options ----------
   ---------------------------------
*/
/*
   ---------------------------------
   ---------- TCP options ----------
   ---------------------------------
*/


/**
 * TCP_QUEUE_OOSEQ==1: TCP will queue segments that arrive out of order.
 * Define to 0 if your device is low on memory.
 */
#define TCP_QUEUE_OOSEQ                 CONFIG_LWIP_TCP_QUEUE_OOSEQ

/**
 * LWIP_TCP_SACK_OUT==1: TCP will support sending selective acknowledgements (SACKs).
 */
#define LWIP_TCP_SACK_OUT               CONFIG_LWIP_TCP_SACK_OUT

/**
 * ESP_TCP_KEEP_CONNECTION_WHEN_IP_CHANGES==1: Keep TCP connection when IP changed
 * scenario happens: 192.168.0.2 -> 0.0.0.0 -> 192.168.0.2 or 192.168.0.2 -> 0.0.0.0
 */

#define ESP_TCP_KEEP_CONNECTION_WHEN_IP_CHANGES  CONFIG_LWIP_TCP_KEEP_CONNECTION_WHEN_IP_CHANGES
/*
 * LWIP_EVENT_API==1: The user defines lwip_tcp_event() to receive all
 * events (accept, sent, etc) that happen in the system.
 * LWIP_CALLBACK_API==1: The PCB callback function is called directly
 * for the event. This is the default.
*/
#define TCP_MSS                         CONFIG_LWIP_TCP_MSS

/**
 * TCP_TMR_INTERVAL: TCP timer interval
 */
#define TCP_TMR_INTERVAL                CONFIG_LWIP_TCP_TMR_INTERVAL

/**
 * TCP_MSL: The maximum segment lifetime in milliseconds
 */
#define TCP_MSL                         CONFIG_LWIP_TCP_MSL

/**
 * TCP_FIN_WAIT_TIMEOUT: The maximum FIN segment lifetime in milliseconds
 */
#define TCP_FIN_WAIT_TIMEOUT            CONFIG_LWIP_TCP_FIN_WAIT_TIMEOUT

/**
 * TCP_MAXRTX: Maximum number of retransmissions of data segments.
 */
#define TCP_MAXRTX                      CONFIG_LWIP_TCP_MAXRTX

/**
 * TCP_SYNMAXRTX: Maximum number of retransmissions of SYN segments.
 */
#define TCP_SYNMAXRTX                   CONFIG_LWIP_TCP_SYNMAXRTX

/**
 * TCP_LISTEN_BACKLOG: Enable the backlog option for tcp listen pcb.
 */
#define TCP_LISTEN_BACKLOG              1


/**
 * TCP_OVERSIZE: The maximum number of bytes that tcp_write may
 * allocate ahead of time
 */
#ifdef CONFIG_LWIP_TCP_OVERSIZE_MSS
#define TCP_OVERSIZE                    TCP_MSS
#endif
#ifdef CONFIG_LWIP_TCP_OVERSIZE_QUARTER_MSS
#define TCP_OVERSIZE                    (TCP_MSS/4)
#endif
#ifdef CONFIG_LWIP_TCP_OVERSIZE_DISABLE
#define TCP_OVERSIZE                    0
#endif
#ifndef TCP_OVERSIZE
#error "One of CONFIG_TCP_OVERSIZE_xxx options should be set by sdkconfig"
#endif

/**
 * LWIP_WND_SCALE and TCP_RCV_SCALE:
 * Set LWIP_WND_SCALE to 1 to enable window scaling.
 * Set TCP_RCV_SCALE to the desired scaling factor (shift count in the
 * range of [0..14]).
 * When LWIP_WND_SCALE is enabled but TCP_RCV_SCALE is 0, we can use a large
 * send window while having a small receive window only.
 */
#ifdef CONFIG_LWIP_WND_SCALE
#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   CONFIG_LWIP_TCP_RCV_SCALE
#endif

/**
 * LWIP_TCP_RTO_TIME: TCP rto time.
 * Default is 3 second.
 */
#define LWIP_TCP_RTO_TIME             CONFIG_LWIP_TCP_RTO_TIME

/*
   ----------------------------------
   ---------- Pbuf options ----------
   ----------------------------------
*/

/*
   ------------------------------------------------
   ---------- Network Interfaces options ----------
   ------------------------------------------------
*/

/**
 * LWIP_NETIF_HOSTNAME==1: use DHCP_OPTION_HOSTNAME with netif's hostname
 * field.
 */
#define LWIP_NETIF_HOSTNAME             1

/**
 * LWIP_NETIF_TX_SINGLE_PBUF: if this is set to 1, lwIP tries to put all data
 * to be sent into one single pbuf. This is for compatibility with DMA-enabled
 * MACs that do not support scatter-gather.
 * Beware that this might involve CPU-memcpy before transmitting that would not
 * be needed without this flag! Use this only if you need to!
 *
 * @todo: TCP and IP-frag do not work with this, yet:
 */
#define LWIP_NETIF_TX_SINGLE_PBUF             1

/**
 * LWIP_NETIF_API==1: Enable usage of standard POSIX APIs in LWIP.
 */
#define LWIP_NETIF_API                      CONFIG_LWIP_NETIF_API

#define LWIP_NETIF_STATUS_CALLBACK      CONFIG_LWIP_NETIF_STATUS_CALLBACK

/*
   ------------------------------------
   ---------- LOOPIF options ----------
   ------------------------------------
*/
#ifdef CONFIG_LWIP_NETIF_LOOPBACK
/**
 * LWIP_NETIF_LOOPBACK==1: Support sending packets with a destination IP
 * address equal to the netif IP address, looping them back up the stack.
 */
#define LWIP_NETIF_LOOPBACK             1

/**
 * LWIP_LOOPBACK_MAX_PBUFS: Maximum number of pbufs on queue for loopback
 * sending for each netif (0 = disabled)
 */
#define LWIP_LOOPBACK_MAX_PBUFS         CONFIG_LWIP_LOOPBACK_MAX_PBUFS
#endif

/*
   ------------------------------------
   ---------- SLIPIF options ----------
   ------------------------------------
*/

#ifdef CONFIG_LWIP_SLIP_SUPPORT

/**
 * Enable SLIP receive from ISR functions and disable Rx thread
 *
 * This is the only supported mode of lwIP SLIP interface, so that
 * - incoming packets are queued into pbufs
 * - no thread is created from lwIP
 * meaning it is the application responsibility to read data
 * from IO driver and feed them to the slip interface
 */
#define SLIP_RX_FROM_ISR                 1
#define SLIP_USE_RX_THREAD               0

/**
 * PPP_DEBUG: Enable debugging for PPP.
 */
#define SLIP_DEBUG_ON                    CONFIG_LWIP_SLIP_DEBUG_ON

#if SLIP_DEBUG_ON
#define SLIP_DEBUG                       LWIP_DBG_ON
#else
#define SLIP_DEBUG                       LWIP_DBG_OFF
#endif


#endif

/*
   ------------------------------------
   ---------- Thread options ----------
   ------------------------------------
*/
/**
 * TCPIP_THREAD_NAME: El nombre asignado al hilo principal de tcpip.
 */
#define TCPIP_THREAD_NAME              "tiT"

/**
 * TCPIP_THREAD_STACKSIZE: El tamaño de la pila utilizado por el hilo principal de tcpip.
 */
#define TCPIP_THREAD_STACKSIZE          ESP_TASK_TCPIP_STACK

/**
 * TCPIP_THREAD_PRIO: La prioridad asignada al hilo principal de tcpip.
 */
#define TCPIP_THREAD_PRIO               ESP_TASK_TCPIP_PRIO

/**
 * TCPIP_MBOX_SIZE: El tamaño del buzón para los mensajes del hilo tcpip.
 */
#define TCPIP_MBOX_SIZE                 CONFIG_LWIP_TCPIP_RECVMBOX_SIZE

/**
 * DEFAULT_UDP_RECVMBOX_SIZE: El tamaño del buzón para los paquetes entrantes en un NETCONN_UDP.
 */
#define DEFAULT_UDP_RECVMBOX_SIZE       CONFIG_LWIP_UDP_RECVMBOX_SIZE

/**
 * DEFAULT_TCP_RECVMBOX_SIZE: El tamaño del buzón para los paquetes entrantes en un NETCONN_TCP.
 */
#define DEFAULT_TCP_RECVMBOX_SIZE       CONFIG_LWIP_TCP_RECVMBOX_SIZE

/**
 * DEFAULT_ACCEPTMBOX_SIZE: El tamaño del buzón para las conexiones entrantes.
 */
#define DEFAULT_ACCEPTMBOX_SIZE         6

/**
 * DEFAULT_THREAD_STACKSIZE: El tamaño de la pila utilizado por cualquier otro hilo de lwIP.
 */
#define DEFAULT_THREAD_STACKSIZE        TCPIP_THREAD_STACKSIZE

/**
 * DEFAULT_THREAD_PRIO: La prioridad asignada a cualquier otro hilo de lwIP.
 */
#define DEFAULT_THREAD_PRIO             TCPIP_THREAD_PRIO

/**
 * DEFAULT_RAW_RECVMBOX_SIZE: El tamaño del buzón para los paquetes entrantes en un NETCONN_RAW.
 */
#define DEFAULT_RAW_RECVMBOX_SIZE       6

/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/

#define LWIP_NETCONN                    1

#define LWIP_NETCONN_SEM_PER_THREAD     1

#define LWIP_NETCONN_FULLDUPLEX         1

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
#define LWIP_SO_SNDTIMEO                1

#define LWIP_SO_RCVTIMEO                1

#define LWIP_TCP_KEEPALIVE              1

#define LWIP_SO_LINGER                  CONFIG_LWIP_SO_LINGER

#define LWIP_SO_RCVBUF                  CONFIG_LWIP_SO_RCVBUF

#define SO_REUSE                        CONFIG_LWIP_SO_REUSE


#define LWIP_DNS_SUPPORT_MDNS_QUERIES   CONFIG_LWIP_DNS_SUPPORT_MDNS_QUERIES

#define SO_REUSE_RXTOALL                CONFIG_LWIP_SO_REUSE_RXTOALL

#define LWIP_NETBUF_RECVINFO            CONFIG_LWIP_NETBUF_RECVINFO

/*
   ----------------------------------------
   ---------- Statistics options ----------
   ----------------------------------------
*/

#define LWIP_STATS                      CONFIG_LWIP_STATS

#if LWIP_STATS
#define LWIP_STATS_DISPLAY              CONFIG_LWIP_STATS
#endif


/*
   ---------------------------------
   ---------- PPP options ----------
   ---------------------------------
*/

#define PPP_SUPPORT                     CONFIG_LWIP_PPP_SUPPORT

#if PPP_SUPPORT

#define PPP_IPV6_SUPPORT                               CONFIG_LWIP_PPP_ENABLE_IPV6

#define PPP_NOTIFY_PHASE                CONFIG_LWIP_PPP_NOTIFY_PHASE_SUPPORT

#define PAP_SUPPORT                     CONFIG_LWIP_PPP_PAP_SUPPORT

#define CHAP_SUPPORT                    CONFIG_LWIP_PPP_CHAP_SUPPORT

#define MSCHAP_SUPPORT                  CONFIG_LWIP_PPP_MSCHAP_SUPPORT

#define MPPE_SUPPORT                    CONFIG_LWIP_PPP_MPPE_SUPPORT

#define PPP_MAXIDLEFLAG                 0

#define PPP_DEBUG_ON                    CONFIG_LWIP_PPP_DEBUG_ON

#if PPP_DEBUG_ON
#define PPP_DEBUG                       LWIP_DBG_ON
#define PRINTPKT_SUPPORT                1
#define PPP_PROTOCOLNAME                1
#else
#define PPP_DEBUG                       LWIP_DBG_OFF
#endif

#endif  /* PPP SUPPORT */

/*
   ------------------------------------
   --------- LCP Echo options ---------
   ------------------------------------
*/
#if CONFIG_LWIP_ENABLE_LCP_ECHO
#define LCP_ECHOINTERVAL                CONFIG_LWIP_LCP_ECHOINTERVAL
#define LCP_MAXECHOFAILS                CONFIG_LWIP_LCP_MAXECHOFAILS
#endif /* CONFIG_LWIP_ENABLE_LCP_ECHO */

/*
   ---------------------------------------
   ---------- IPv6 options ---------------
   ---------------------------------------
*/
#define LWIP_IPV6                       CONFIG_LWIP_IPV6

#define MEMP_NUM_ND6_QUEUE              CONFIG_LWIP_IPV6_MEMP_NUM_ND6_QUEUE

#define LWIP_ND6_NUM_NEIGHBORS          CONFIG_LWIP_IPV6_ND6_NUM_NEIGHBORS


#ifdef CONFIG_LWIP_ESP_MLDV6_REPORT
#define ESP_MLDV6_REPORT              1
#else
#define ESP_MLDV6_REPORT              0
#endif

/*
   ---------------------------------------
   ---------- Hook options ---------------
   ---------------------------------------
*/
#ifdef LWIP_HOOK_FILENAME
#warning LWIP_HOOK_FILENAME is used for IDF default hooks. Please use ESP_IDF_LWIP_HOOK_FILENAME to insert additional hook
#endif
#define LWIP_HOOK_FILENAME              "lwip_default_hooks.h"
#define LWIP_HOOK_IP4_ROUTE_SRC         ip4_route_src_hook

/*
   ---------------------------------------
   ---------- Debugging options ----------
   ---------------------------------------
*/
#ifdef CONFIG_LWIP_ETHARP_DEBUG
#define ETHARP_DEBUG                     LWIP_DBG_ON
#else
#define ETHARP_DEBUG                     LWIP_DBG_OFF
#endif


#ifdef CONFIG_LWIP_NETIF_DEBUG
#define NETIF_DEBUG                     LWIP_DBG_ON
#else
#define NETIF_DEBUG                     LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_PBUF_DEBUG
#define PBUF_DEBUG                     LWIP_DBG_ON
#else
#define PBUF_DEBUG                     LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_API_LIB_DEBUG
#define API_LIB_DEBUG                     LWIP_DBG_ON
#else
#define API_LIB_DEBUG                     LWIP_DBG_OFF
#endif


#ifdef CONFIG_LWIP_SOCKETS_DEBUG
#define SOCKETS_DEBUG                   LWIP_DBG_ON
#else
#define SOCKETS_DEBUG                   LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_ICMP_DEBUG
#define ICMP_DEBUG                      LWIP_DBG_ON
#else
#define ICMP_DEBUG                      LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_ICMP6_DEBUG
#define ICMP6_DEBUG                      LWIP_DBG_ON
#else
#define ICMP6_DEBUG                      LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_DHCP_DEBUG
#define DHCP_DEBUG                      LWIP_DBG_ON
#else
#define DHCP_DEBUG                      LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_DHCP_STATE_DEBUG
#define ESP_DHCP_DEBUG                  LWIP_DBG_ON
#else
#define ESP_DHCP_DEBUG                  LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_IP_DEBUG
#define IP_DEBUG                        LWIP_DBG_ON
#else
#define IP_DEBUG                        LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_IP6_DEBUG
#define IP6_DEBUG                        LWIP_DBG_ON
#else
#define IP6_DEBUG                        LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_TCP_DEBUG
#define TCP_DEBUG                        LWIP_DBG_ON
#else
#define TCP_DEBUG                        LWIP_DBG_OFF
#endif

#ifdef CONFIG_LWIP_SNTP_DEBUG
#define SNTP_DEBUG                       LWIP_DBG_ON
#else
#define SNTP_DEBUG                       LWIP_DBG_OFF
#endif

#define MEMP_DEBUG                      LWIP_DBG_OFF
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
#define TCPIP_DEBUG                     LWIP_DBG_OFF
#define TCP_OOSEQ_DEBUG                 LWIP_DBG_OFF

#define ETHARP_TRUST_IP_MAC             CONFIG_LWIP_ETHARP_TRUST_IP_MAC

#define LWIP_POSIX_SOCKETS_IO_NAMES     0

#define LWIP_SOCKET_OFFSET              (FD_SETSIZE - CONFIG_LWIP_MAX_SOCKETS)

#define LWIP_IPV6_FORWARD               CONFIG_LWIP_IPV6_FORWARD

#define LWIP_IPV6_NUM_ADDRESSES         CONFIG_LWIP_IPV6_NUM_ADDRESSES

#define LWIP_ND6_RDNSS_MAX_DNS_SERVERS  CONFIG_LWIP_IPV6_RDNSS_MAX_DNS_SERVERS

#define LWIP_IPV6_DHCP6                 CONFIG_LWIP_IPV6_DHCP6

/* Enable all Espressif-only options */

#define ESP_LWIP                        1
#define ESP_LWIP_ARP                    1
#define ESP_PER_SOC_TCP_WND             0
#define ESP_THREAD_SAFE                 1
#define ESP_THREAD_SAFE_DEBUG           LWIP_DBG_OFF
#define ESP_DHCP                        1
#define ESP_DNS                         1
#define ESP_PERF                        0
#define ESP_RANDOM_TCP_PORT             1
#define ESP_IP4_ATON                    1
#define ESP_LIGHT_SLEEP                 1
#define ESP_L2_TO_L3_COPY               CONFIG_LWIP_L2_TO_L3_COPY
#define LWIP_NETIF_API                  CONFIG_LWIP_NETIF_API
#define ESP_STATS_MEM                   CONFIG_LWIP_STATS
#define ESP_STATS_DROP                  CONFIG_LWIP_STATS
#define ESP_STATS_TCP                   0
#ifdef CONFIG_LWIP_DHCPS
#define ESP_DHCPS                       1
#define ESP_DHCPS_TIMER                 1
#else
#define ESP_DHCPS                       0
#define ESP_DHCPS_TIMER                 0
#endif /* CONFIG_LWIP_DHCPS */
#define ESP_LWIP_LOGI(...)              ESP_LOGI("lwip", __VA_ARGS__)
#define ESP_PING                        1
#define ESP_HAS_SELECT                  1
#define ESP_AUTO_RECV                   1
#define ESP_GRATUITOUS_ARP              CONFIG_LWIP_ESP_GRATUITOUS_ARP
#define ESP_IP4_ROUTE                   1
#define ESP_AUTO_IP                     1
#define ESP_PBUF                        1
#define ESP_PPP                         1
#define ESP_IPV6                        LWIP_IPV6
#define ESP_SOCKET                      1
#define ESP_LWIP_SELECT                 1
#define ESP_LWIP_LOCK                   1
#define ESP_THREAD_PROTECTION           1
#define ESP_IP_FORWARD                  1

#ifdef CONFIG_LWIP_IPV6_AUTOCONFIG
#define ESP_IPV6_AUTOCONFIG             CONFIG_LWIP_IPV6_AUTOCONFIG
#endif

#ifdef ESP_IRAM_ATTR
#undef ESP_IRAM_ATTR
#endif
#define ESP_IRAM_ATTR

#ifdef CONFIG_LWIP_TIMERS_ONDEMAND
#define ESP_LWIP_IGMP_TIMERS_ONDEMAND            1
#define ESP_LWIP_MLD6_TIMERS_ONDEMAND            1
#define ESP_LWIP_DHCP_FINE_TIMERS_ONDEMAND       1
#define ESP_LWIP_DNS_TIMERS_ONDEMAND             1
#if IP_REASSEMBLY
#define ESP_LWIP_IP4_REASSEMBLY_TIMERS_ONDEMAND  1
#endif /* IP_REASSEMBLY */
#if LWIP_IPV6_REASS
#define ESP_LWIP_IP6_REASSEMBLY_TIMERS_ONDEMAND  1
#endif /* LWIP_IPV6_REASS */
#else
#define ESP_LWIP_IGMP_TIMERS_ONDEMAND            0
#define ESP_LWIP_MLD6_TIMERS_ONDEMAND            0
#define ESP_LWIP_DHCP_FINE_TIMERS_ONDEMAND       0
#define ESP_LWIP_DNS_TIMERS_ONDEMAND             0
#if IP_REASSEMBLY
#define ESP_LWIP_IP4_REASSEMBLY_TIMERS_ONDEMAND  0
#endif /* IP_REASSEMBLY */
#if LWIP_IPV6_REASS
#define ESP_LWIP_IP6_REASSEMBLY_TIMERS_ONDEMAND  0
#endif /* LWIP_IPV6_REASS */
#endif

#define TCP_SND_BUF                     CONFIG_LWIP_TCP_SND_BUF_DEFAULT
#define TCP_WND                         CONFIG_LWIP_TCP_WND_DEFAULT

/**
 * LWIP_DEBUG: Enable lwip debugging in other modules.
 */
#ifdef CONFIG_LWIP_DEBUG
#define LWIP_DEBUG                      LWIP_DBG_ON
#else
#undef LWIP_DEBUG
#endif

#define CHECKSUM_CHECK_UDP              CONFIG_LWIP_CHECKSUM_CHECK_UDP
#define CHECKSUM_CHECK_IP               CONFIG_LWIP_CHECKSUM_CHECK_IP
#define CHECKSUM_CHECK_ICMP             CONFIG_LWIP_CHECKSUM_CHECK_ICMP

#define LWIP_NETCONN_FULLDUPLEX         1

#define LWIP_DHCP_MAX_NTP_SERVERS       CONFIG_LWIP_DHCP_MAX_NTP_SERVERS
#define LWIP_TIMEVAL_PRIVATE            0

/*
   --------------------------------------
   ------------ SNTP options ------------
   --------------------------------------
*/

// Max number of SNTP servers handled
#if defined CONFIG_LWIP_SNTP_MAX_SERVERS
#define SNTP_MAX_SERVERS                CONFIG_LWIP_SNTP_MAX_SERVERS
#endif

#ifdef CONFIG_LWIP_DHCP_GET_NTP_SRV
#define LWIP_DHCP_GET_NTP_SRV           CONFIG_LWIP_DHCP_GET_NTP_SRV
#endif

#define SNTP_SERVER_DNS            1

#define SNTP_SUPPRESS_DELAY_CHECK

#define SNTP_UPDATE_DELAY                 (sntp_get_sync_interval())
#define SNTP_SET_SYSTEM_TIME_US(sec, us)  (sntp_set_system_time(sec, us))
#define SNTP_GET_SYSTEM_TIME(sec, us)     (sntp_get_system_time(&(sec), &(us)))

#define SOC_SEND_LOG //printf

#ifdef __cplusplus
}
#endif

#endif /* __LWIPOPTS_H__ */
