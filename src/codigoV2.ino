#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include "lwip/lwip_napt.h"
#include "lwip/err.h"
#include "esp_wifi.h"

extern "C" {
  #include <lwip/lwip_napt.h>
  err_t ip_napt_init(uint8_t max_tcp, uint8_t max_udp);
  err_t ip_napt_enable_no(uint8_t if_idx, uint8_t enable);
}
#define SOFTAP_IF ESP_IF_WIFI_AP

// ==========================================
// 1. CONFIGURACIÓN DE REDES
// ==========================================
const char* ST_SSID = "STARLINK";
const char* ST_PASS = "Pauli2807";

const char* AP_SSID = "ESP32_Protector";
const char* AP_PASS = "seguridad123";

IPAddress apIP(192, 168, 4, 1);

// DNS upstream real (Google)
const IPAddress upstreamDNS(8, 8, 8, 8);

// ==========================================
// 2. LISTA DE DOMINIOS BLOQUEADOS (la misma)
// ==========================================
const char* blocked_domains[] = {
  "f01.ps4.update.playstation.net",
  "h01.ps4.update.playstation.net",
  "update.playstation.net",
  "ps4.updatelist.playstation.net",
  "post.net.playstation.net",
  "creu.playstation.net",
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

// Número de dominios bloqueados
const int num_blocked = sizeof(blocked_domains) / sizeof(blocked_domains[0]);

// ==========================================
// 3. SERVIDOR DNS PERSONALIZADO (PROXY)
// ==========================================
DNSServer dnsServer;
WiFiUDP udp; // Para reenviar consultas

// Función auxiliar para verificar si un dominio está en la lista negra
bool isBlocked(const String& domain) {
  for (int i = 0; i < num_blocked; i++) {
    // Soporta comodines simples (ej: "*.ps4.update.playstation.net")
    String blocked = String(blocked_domains[i]);
    if (blocked.startsWith("*.")) {
      String suffix = blocked.substring(2);
      if (domain.endsWith(suffix)) return true;
    } else if (domain.equals(blocked)) {
      return true;
    }
  }
  return false;
}

// Esta función se llama cada vez que llega una consulta DNS
void handleDNSRequest() {
  // Leer el paquete DNS entrante
  int packetSize = udp.parsePacket();
  if (!packetSize) return;

  uint8_t buffer[512];
  int len = udp.read(buffer, 512);
  if (len <= 0) return;

  // Obtener el nombre de dominio desde la consulta DNS (formato estándar)
  String domain = "";
  int pos = 12; // Saltar cabecera DNS
  while (buffer[pos] != 0) {
    int labelLen = buffer[pos];
    pos++;
    for (int i = 0; i < labelLen; i++) {
      domain += (char)buffer[pos + i];
    }
    pos += labelLen;
    if (buffer[pos] != 0) domain += ".";
  }

  // Extraer el ID de transacción y flags (para reenviar igual)
  uint16_t id = (buffer[0] << 8) | buffer[1];
  bool isQuery = (buffer[2] & 0x80) == 0; // QR bit = 0

  if (!isQuery) return; // Solo procesamos consultas

  // Log de la consulta
  Serial.print("[DNS] Consulta recibida: ");
  Serial.println(domain);

  if (isBlocked(domain)) {
    // BLOQUEADO: responder con 0.0.0.0
    Serial.print("[DNS] BLOQUEADO: ");
    Serial.println(domain);
    
    // Construir respuesta DNS tipo A con 0.0.0.0
    uint8_t response[512];
    memcpy(response, buffer, len); // Copiar consulta
    // Modificar cabecera: QR=1, respuesta autoritativa, sin error
    response[2] = 0x81; // QR=1, AA=1, opcode=0, no truncado
    response[3] = 0x80; // RA=1, RCODE=0
    // Número de respuestas = 1
    response[6] = 0x00;
    response[7] = 0x01;
    // Construir la sección de respuesta (simplificado)
    int respLen = len;
    // Añadir registro A: nombre comprimido (apuntando a la consulta)
    response[respLen++] = 0xC0; // Puntero
    response[respLen++] = 0x0C; // Offset 12 (inicio del nombre)
    response[respLen++] = 0x00; // Tipo A
    response[respLen++] = 0x01;
    response[respLen++] = 0x00; // Clase IN
    response[respLen++] = 0x01;
    response[respLen++] = 0x00; // TTL (1 hora)
    response[respLen++] = 0x00;
    response[respLen++] = 0x00;
    response[respLen++] = 0xE4;
    response[respLen++] = 0x00; // Longitud de datos = 4
    response[respLen++] = 0x04;
    response[respLen++] = 0x00; // 0.0.0.0
    response[respLen++] = 0x00;
    response[respLen++] = 0x00;
    response[respLen++] = 0x00;
    
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(response, respLen);
    udp.endPacket();
  } else {
    // PERMITIDO: reenviar al upstream DNS
    Serial.print("[DNS] PERMITIDO -> reenviando a ");
    Serial.print(upstreamDNS);
    Serial.print(": ");
    Serial.println(domain);
    
    // Crear nuevo socket UDP hacia el upstream
    WiFiUDP forwardUdp;
    forwardUdp.begin(5353); // Puerto local temporal
    forwardUdp.beginPacket(upstreamDNS, 53);
    forwardUdp.write(buffer, len);
    forwardUdp.endPacket();
    
    // Esperar respuesta (con timeout de 2 segundos)
    uint32_t start = millis();
    int respLen = 0;
    uint8_t respBuffer[512];
    while (millis() - start < 2000) {
      int psize = forwardUdp.parsePacket();
      if (psize) {
        respLen = forwardUdp.read(respBuffer, 512);
        break;
      }
      delay(10);
    }
    forwardUdp.stop();
    
    if (respLen > 0) {
      // Reenviar la respuesta al cliente original
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.write(respBuffer, respLen);
      udp.endPacket();
      
      // Extraer y mostrar las IPs permitidas (solo para logging)
      Serial.print("[DNS] RESPUESTA PERMITIDA para ");
      Serial.print(domain);
      Serial.print(" -> IPs: ");
      // Buscar registros A en la respuesta
      int idx = 12;
      // Saltar el nombre de la consulta (comprimido o no)
      while (idx < respLen && respBuffer[idx] != 0) {
        if ((respBuffer[idx] & 0xC0) == 0xC0) {
          idx += 2;
          break;
        }
        idx += respBuffer[idx] + 1;
      }
      idx++; // saltar el 0 final
      idx += 4; // saltar tipo, clase, TTL, longitud (simplificado)
      if (idx + 4 <= respLen) {
        Serial.print(String(respBuffer[idx]) + "." + 
                     String(respBuffer[idx+1]) + "." + 
                     String(respBuffer[idx+2]) + "." + 
                     String(respBuffer[idx+3]));
      }
      Serial.println();
    } else {
      Serial.print("[DNS] ERROR: No hay respuesta del upstream para ");
      Serial.println(domain);
      // Opcional: enviar respuesta vacía o SERVFAIL
    }
  }
}

// ==========================================
// 4. SETUP Y LOOP PRINCIPAL
// ==========================================
void setup() {
  Serial.begin(115200);
  
  // Conectar a Internet (Starlink)
  WiFi.begin(ST_SSID, ST_PASS);
  Serial.print("Conectando a Starlink...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n¡Conectado a Internet!");
  Serial.print("IP en Starlink: ");
  Serial.println(WiFi.localIP());
  
  // Configurar punto de acceso para la PS4
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASS);

  // 3. ACTIVAR EL ENRUTAMIENTO (NAT)
  // Esto es para tener internet
  ip_napt_init(8, 8);
  ip_napt_enable_no(SOFTAP_IF, 1);
  
  Serial.print("AP creado: ");
  Serial.println(AP_SSID);
  Serial.print("IP del AP: ");
  Serial.println(apIP);
  
  // Iniciar el servidor DNS personalizado en el puerto 53
  udp.begin(53);
  Serial.println("Servidor DNS proxy iniciado en puerto 53");
  Serial.println("Logging activado. Conecta tu PS4 a 'ESP32_Protector'");
}

void loop() {
  handleDNSRequest();
  delay(1); // Pequeña pausa para evitar saturar la CPU
}
