#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/prot/dns.h"
#include "dhcpserver/dhcpserver.h"
#include "esp_netif.h"
#include "lwip/lwip_napt.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"

#define EXAMPLE_ESP_WIFI_SSID      "ProtectorPS4"
#define EXAMPLE_ESP_WIFI_PASS      "seguridad123"
#define EXAMPLE_ESP_WIFI_CHANNEL   6
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "ESP32_ROUTER";

// Variables de estado para la conexión STA
static bool sta_connected = false;
static bool sta_got_ip = false;

// --- Lista de bloqueo con comodines (*) ---
const char* blocked_domains[] = {
    "*.*.*.playstation.*",
    "*.*.playstation.*",
    "*.*.playstation.*.*",
    "*.playstation.*.*",
	"ps4-system.sec.e1-np.dl.playstation.net",
    "ps4-system.sec.sp-int.dl.playstation.net",
    "ps4-system.sec.np.dl.playstation.net",
	"us.sp-int.stun.playstation.net",
	"event.api.sp-int.km.playstation.net",
	"urlconfig.sp-int.api.playstation.com",
    "gs-sec.ww.sp-int.dl.playstation.net",
    "event.api.np.km.playstation.net",
    "ps4updptl.us.sp-int.community.playstation.net",
    "f01.ps4.update.playstation.net",
    "h01.ps4.update.playstation.net",
    "update.playstation.net",
    "ps4.updatelist.playstation.net",
    "post.net.playstation.net",
    "creu.playstation.net",
    "*.playstation.*",
    "dau01.ps4.update.playstation.net",
    "dbr01.ps4.update.playstation.net",
    "dcn01.ps4.update.playstation.net",
    "deu01.ps4.update.playstation.net",
    "dhk01.ps4.update.playstation.net",
    "djp01.ps4.update.playstation.net",
    "dkr01.ps4.update.playstation.net",
    "dmx01.ps4.update.playstation.net",
    "dru01.ps4.update.playstation.net",
    "dsa01.ps4.update.playstation.net",
    "dtw01.ps4.update.playstation.net",
    "duk01.ps4.update.playstation.net",
    "dus01.ps4.update.playstation.net",
    "fau01.ps4.update.playstation.net",
    "fbr01.ps4.update.playstation.net",
    "fcn01.ps4.update.playstation.net",
    "feu01.ps4.update.playstation.net",
    "fhk01.ps4.update.playstation.net",
    "fjp01.ps4.update.playstation.net",
    "fkr01.ps4.update.playstation.net",
    "fmx01.ps4.update.playstation.net",
    "fru01.ps4.update.playstation.net",
    "fsa01.ps4.update.playstation.net",
    "ftw01.ps4.update.playstation.net",
    "fuk01.ps4.update.playstation.net",
    "fus01.ps4.update.playstation.net",
    "hau01.ps4.update.playstation.net",
    "hbr01.ps4.update.playstation.net",
    "hcn01.ps4.update.playstation.net",
    "heu01.ps4.update.playstation.net",
    "hhk01.ps4.update.playstation.net",
    "hjp01.ps4.update.playstation.net",
    "hkr01.ps4.update.playstation.net",
    "hmx01.ps4.update.playstation.net",
    "hru01.ps4.update.playstation.net",
    "hsa01.ps4.update.playstation.net",
    "htw01.ps4.update.playstation.net",
    "huk01.ps4.update.playstation.net",
    "hus01.ps4.update.playstation.net",
    "a01.cdn.update.playstation.org.edgesuite.net",
    "a192.d.akamai.net",
    "al02.cdn.update.playstation.net",
    "apicdn-p014.ribob01.net",
    "api-p014.ribob01.net",
    "artcdnsecure.ribob01.net",
    "asm.np.community.playstation.net",
    "get.net.playstation.net",
    "playstation.sony.akadns.net",
    "ps4-eb.ww.np.dl.playstation.net",
    "ps4updptl.eu.np.community.playstation.net",
    "ps4updptl.jp.sp-int.community.playstation.net",
    "ps4.updptl.sp-int.community.playstation.net",
    "sf.api.np.km.playstation.net",
    "themis.dl.playstation.net",
    "tmdb.np.dl.playstation.net",
    "psndl.net",
    "t-prof.np.community.playstation.net",
    "psn-rsc.prod.dl.playstation.net",
    "apollo.dl.playstation.net",
    "apollo2.dl.playstation.net",
    "oqesn.np.dl.playstation.net",
    "nsx.sec.np.dl.playstation.net",
    "gs2.ww.prod.dl.playstation.net",
    "duk01.ps5.update.playstation.net",
    "fuk01.ps5.update.playstation.net",
    "*.ps4.update.playstation.net",
    "crepo.ww.dl.playstation.net",
    "gs-sec.ww.np.dl.playstation.net",
    "*.dl.playstation.net",
    "ps4.np.playstation.net",
    "*.np.community.playstation.net",
    "*.np.dl.playstation.net",
    "dl.playstation.net",
    "uk.test.playstation.net",
    "d01.ps4.update.playstation.net",
    "ena.net.playstation.net",
    "feu.net.playstation.net",
    "fjp.net.playstation.net",
    "fkr.net.playstation.net",
    "fuk.net.playstation.net",
    "fus.net.playstation.net"
};
#define NUM_BLOCKED_DOMAINS (sizeof(blocked_domains) / sizeof(blocked_domains[0]))

// Función auxiliar para convertir modo de autenticación a cadena
static const char* auth_mode_to_string(wifi_auth_mode_t mode) {
    switch (mode) {
        case WIFI_AUTH_OPEN: return "OPEN";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA_PSK";
        case WIFI_AUTH_WPA2_PSK: return "WPA2_PSK";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA_WPA2_PSK";
        case WIFI_AUTH_WPA3_PSK: return "WPA3_PSK";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2_WPA3_PSK";
        case WIFI_AUTH_WAPI_PSK: return "WAPI_PSK";
        default: return "UNKNOWN";
    }
}

// Función para comprobar coincidencia con comodín (*)
bool matches_pattern(const char *domain, const char *pattern) {
    const char *d = domain;
    const char *p = pattern;
    while (*d && *p) {
        if (*p == '*') {
            if (*(p+1) == '\0') return true;
            const char *next_d = strchr(d, *(p+1));
            if (!next_d) return false;
            d = next_d;
            p++;
        } else if (*d != *p) {
            return false;
        } else {
            d++; p++;
        }
    }
    return (*d == '\0' && *p == '\0') ||
           (*p == '*' && *(p+1) == '\0') ||
           (*d == '\0' && *p == '\0');
}

// Función de bloqueo
bool check_and_block_domain(const char *name) {
    for (int i = 0; i < NUM_BLOCKED_DOMAINS; i++) {
        if (matches_pattern(name, blocked_domains[i])) {
            ESP_LOGI(TAG, "Bloqueado: %s (coincide con %s)", name, blocked_domains[i]);
            return true;
        }
    }
    ESP_LOGI(TAG, "Permitido: %s", name);
    return false;
}

// ==================== SERVIDOR DNS ====================
#define DNS_SERVER_PORT 53
#define DNS_UPSTREAM_IP   "8.8.8.8"
#define DNS_BUFFER_SIZE  512

static void dns_server_task(void *pvParameters) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "No se pudo crear socket DNS");
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(DNS_SERVER_PORT),
        .sin_addr = { .s_addr = htonl(INADDR_ANY) }
    };
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "No se pudo bind socket DNS");
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Servidor DNS iniciado en puerto 53");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    uint8_t buffer[DNS_BUFFER_SIZE];

    struct sockaddr_in upstream = {
        .sin_family = AF_INET,
        .sin_port = htons(53)
    };
    inet_aton(DNS_UPSTREAM_IP, &upstream.sin_addr);

    while (1) {
        int len = recvfrom(sock, buffer, DNS_BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if (len < 0) {
            ESP_LOGE(TAG, "Error recvfrom DNS");
            continue;
        }

        // Analizar consulta DNS para extraer el nombre
        uint8_t *query = buffer;
        int qdcount = (query[4] << 8) | query[5];
        if (qdcount == 0) continue;

        uint8_t *p = query + 12;
        char name[256];
        int name_len = 0;
        while (*p != 0) {
            int label_len = *p++;
            if (label_len > 0) {
                if (name_len > 0) name[name_len++] = '.';
                memcpy(name + name_len, p, label_len);
                name_len += label_len;
                p += label_len;
            }
        }
        name[name_len] = '\0';
        p += 1; // saltar el 0 final
        p += 4; // saltar QTYPE y QCLASS

        ESP_LOGI(TAG, "Consulta DNS recibida: %s", name);

        if (check_and_block_domain(name)) {
            // Dominio bloqueado: responder con 0.0.0.0
            uint8_t response[DNS_BUFFER_SIZE];
            memcpy(response, query, len);
            response[2] |= 0x80;
            response[6] = 0;
            response[7] = 1;
            uint8_t *resp = response + len;
            uint8_t *name_ptr = query + 12;
            while (*name_ptr != 0) {
                *resp++ = *name_ptr++;
            }
            *resp++ = 0;
            *resp++ = 0; *resp++ = 1;   // TYPE A
            *resp++ = 0; *resp++ = 1;   // CLASS IN
            *resp++ = 0; *resp++ = 0; *resp++ = 1; *resp++ = 0x2c; // TTL 300
            *resp++ = 0; *resp++ = 4;   // RDLENGTH 4
            *resp++ = 0; *resp++ = 0; *resp++ = 0; *resp++ = 0; // 0.0.0.0
            int new_len = resp - response;
            sendto(sock, response, new_len, 0, (struct sockaddr*)&client_addr, client_len);
            ESP_LOGI(TAG, "Respuesta DNS bloqueado: 0.0.0.0 para %s", name);
        } else {
            // Reenviar al upstream
            int up_sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (up_sock < 0) {
                ESP_LOGE(TAG, "No se pudo crear socket upstream");
                continue;
            }
            sendto(up_sock, buffer, len, 0, (struct sockaddr*)&upstream, sizeof(upstream));
            struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
            setsockopt(up_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            uint8_t up_buffer[DNS_BUFFER_SIZE];
            int up_len = recvfrom(up_sock, up_buffer, DNS_BUFFER_SIZE, 0, NULL, NULL);
            if (up_len > 0) {
                sendto(sock, up_buffer, up_len, 0, (struct sockaddr*)&client_addr, client_len);
            } else {
                ESP_LOGW(TAG, "No se recibió respuesta upstream para %s", name);
            }
            close(up_sock);
        }
    }
    close(sock);
    vTaskDelete(NULL);
}
// ======================================================

// Inicializar NVS
void nvs_init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

// Manejador de eventos WiFi
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        sta_connected = true;
        ESP_LOGI(TAG, "STA conectado al router STARLINK");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *disconn = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGE(TAG, "STA desconectado, razón: %d", disconn->reason);
        sta_connected = false;
        sta_got_ip = false;
        ESP_LOGI(TAG, "Reintentando conexión a STARLINK...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "STA obtuvo IP: " IPSTR, IP2STR(&event->ip_info.ip));
        sta_got_ip = true;
    }
}

// Configuración WiFi como Station + AP
void wifi_init_softap_sta(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Configuración Station
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "STARLINK",
            .password = "Pauli2807",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_LOGI(TAG, "Autenticación para conectarse a STARLINK: %s", auth_mode_to_string(sta_config.sta.threshold.authmode));

    // Configuración Access Point
    wifi_config_t ap_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = EXAMPLE_MAX_STA_CONN,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_LOGI(TAG, "Autenticación para la red ProtectorPS4: %s", auth_mode_to_string(ap_config.ap.authmode));

    // Configurar DHCP del AP para entregar la IP del ESP32 como DNS
    esp_netif_t *netif_ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (netif_ap) {
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(netif_ap, &ip_info);
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4 = ip_info.ip;
        dns.ip.type = IPADDR_TYPE_V4;
        esp_netif_set_dns_info(netif_ap, ESP_NETIF_DNS_MAIN, &dns);
        ESP_LOGI(TAG, "DHCP configurado para entregar DNS: " IPSTR, IP2STR(&ip_info.ip));
    }

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "WiFi AP (%s) iniciado, conectando a STARLINK...", EXAMPLE_ESP_WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Esperar conexión STA
    int retry = 0;
    while (!sta_got_ip && retry < 30) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        retry++;
    }
    if (!sta_got_ip) {
        ESP_LOGE(TAG, "No se pudo conectar al router STARLINK. Verifique SSID y password, y que la banda sea 2.4 GHz.");
    } else {
        ESP_LOGI(TAG, "Conexión a STARLINK establecida con éxito");
    }

    // Habilitar NAT
    esp_netif_t *netif_sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif_ap && netif_sta) {
        esp_netif_napt_enable(netif_ap);
        ESP_LOGI(TAG, "NAT habilitado entre AP y STA, forwarding activado");
    } else {
        ESP_LOGE(TAG, "Error: No se pudieron obtener las interfaces de red");
    }
}

void app_main(void) {
    nvs_init();
    wifi_init_softap_sta();
    xTaskCreate(dns_server_task, "dns_server", 4096, NULL, 5, NULL);
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
