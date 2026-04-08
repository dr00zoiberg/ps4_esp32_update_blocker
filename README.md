# Propósito

Utilice su módulo ESP32 para obtener conectividad a Internet sin necesidad de actualizar su consola ni sus programas.  
*(Probado con la versión de firmware v12.50)*

## Binarios

Emplee los binarios en el siguiente sitio web:  
[https://esptool.spacehuhn.com/](https://esptool.spacehuhn.com/)

## Direcciones de memoria y archivos correspondientes

Los binarios disponibles están actualizados.

| Dirección | Archivo               |
|-----------|-----------------------|
| `0x1000`  | `bootloader.bin`      |
| `0x8000`  | `partition-table.bin` |
| `0xE000`  | `vacío`               |
| `0x10000` | `softap_sta.bin`      |

### Guía de compilación y configuración WiFi

Para compilar este proyecto, utilice el entorno de desarrollo **ESP‑IDF** disponible en el siguiente enlace oficial:

🔗 [https://idf.espressif.com/](https://idf.espressif.com/)

## Configuración necesaria

Antes de compilar, debe modificar las credenciales de la red WiFi en el código fuente. Busque las siguientes variables y sustituya los valores de ejemplo por los suyos:

`
.ssid = "nombre-wifi",
.password = "password-wifi"
`

Una vez programado el ESP32, este generará una red Wi-Fi con los siguientes parámetros.  
** **Use esta red para conectarse a su PS4.** **

- **Nombre de la red (SSID):** `ProtectorPS4`
- **Contraseña:** `seguridad123`

## Monitoreo de conexiones

Para visualizar las conexiones permitidas y bloqueadas, utilice el **monitor serie** del **Arduino IDE Legacy 1.8.19**.  
Puede descargar esta versión desde el sitio oficial:  
[https://www.arduino.cc/en/software/](https://www.arduino.cc/en/software/)

A través del monitor serie podrá observar en tiempo real qué direcciones están siendo permitidas o bloqueadas por el sistema.

## Reporte de direcciones no bloqueadas

Si durante el monitoreo detecta direcciones que deberían estar bloqueadas pero no lo están, se solicita que abra un **issue** en el repositorio para añadir dicha dirección a la lista de bloqueo.

## Aviso importante

El uso de este software es bajo su exclusiva responsabilidad. El código fuente ubicado en la carpeta "`src`" main.c .
