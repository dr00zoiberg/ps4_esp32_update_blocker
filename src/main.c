#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

#define EXAMPLE_ESP_WIFI_SSID      "ProtectorPS4"
#define EXAMPLE_ESP_WIFI_PASS      "seguridad123"
#define EXAMPLE_ESP_WIFI_CHANNEL   6
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "ESP32_ROUTER";

// --- Lista de bloqueo con comodines (*) ---
// Ahora calculamos el número automáticamente para evitar errores
const char* blocked_domains[] = {
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
// Número automático de elementos
#define NUM_BLOCKED_DOMAINS (sizeof(blocked_domains) / sizeof(blocked_domains[0]))

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

// Esta función se llamará desde un hook de DNS (más abajo)
bool check_and_block_domain(const char *name) {
    for (int i = 0; i < NUM_BLOCKED_DOMAINS; i++) {
        if (matches_pattern(name, blocked_domains[i])) {
            ESP_LOGI(TAG, "Bloqueado: %s (coincide con %s)", name, blocked_domains[i]);
            return true; // Bloquear
        }
    }
    ESP_LOGI(TAG, "Permitido: %s", name);
    return false; // No bloquear
}

// Inicializar DNS con servidores normales (el bloqueo se hará por hook)
void initialize_dns_blocking() {
    // Configurar servidores DNS públicos
    ip_addr_t dns1, dns2;
    ipaddr_aton("8.8.8.8", &dns1);
    ipaddr_aton("8.8.4.4", &dns2);
    dns_setserver(0, &dns1);
    dns_setserver(1, &dns2);
    ESP_LOGI(TAG, "DNS configurado: 8.8.8.8 y 8.8.4.4");
}

// Inicializar NVS
void nvs_init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

// Configuración WiFi como Station + AP
void wifi_init_softap_sta(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Configuración Station (conexión a internet)
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "STARLINK",     // ← CAMBIA por el SSID de tu router
            .password = "Pauli2807", // ← CAMBIA por la contraseña de tu router
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));

    // Configuración Access Point (red que crea el ESP32)
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

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "WiFi AP (%s) y STA iniciados", EXAMPLE_ESP_WIFI_SSID);

    // Habilitar NAT (IP Forwarding)
    esp_netif_t *netif_ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    esp_netif_t *netif_sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif_ap && netif_sta) {
        esp_netif_napt_enable(netif_ap);
        ESP_LOGI(TAG, "NAT habilitado entre AP y STA");
    } else {
        ESP_LOGE(TAG, "Error: No se pudieron obtener las interfaces de red");
    }

    initialize_dns_blocking();
}

// Hook personalizado para filtrar DNS (se llama antes de resolver)
// Esta función debe ser habilitada en menuconfig (LWIP_HOOK_IP4_ROUTE o similar)
// Pero para que compile, la dejamos definida. Luego te explico cómo activarla.
void lwip_hook_dns_ext_resolve(const char *name, ip_addr_t *addr) {
    if (check_and_block_domain(name)) {
        // Si está bloqueado, devolvemos una dirección inválida (0.0.0.0)
        ip_addr_set_zero(addr);
    }
}

void app_main(void) {
    nvs_init();
    wifi_init_softap_sta();

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
