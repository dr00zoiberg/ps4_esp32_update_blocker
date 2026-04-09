# Propósito

Utilice su módulo ESP32 para obtener conectividad a Internet sin necesidad de actualizar su consola ni sus programas.  
*(Probado con la versión de firmware v12.50)*

## Procedimiento de puesta en marcha

Emplee los binarios a través de la herramienta web disponible en [https://esptool.spacehuhn.com/](https://esptool.spacehuhn.com/) use la tabla para su configuracion una vez el proceso de grabación se haya completado con éxito, se procederá de la siguiente manera:

## Direcciones de memoria y archivos correspondientes

Los binarios disponibles están actualizados.

| Dirección | Archivo               |
|-----------|-----------------------|
| `0x1000`  | `bootloader.bin`      |
| `0x8000`  | `partition-table.bin` |
| `0xE000`  | `vacío`               |
| `0x10000` | `softap_sta.bin`      |

1. Cierre la página web del programador para liberar el puerto serie utilizado.
2. Presione el botón de reinicio (RESET) en la placa ESP32 o bien desconecte la alimentación durante unos segundos y vuelva a conectarla.
3. Espere unos instantes hasta que el dispositivo genere la red inalámbrica con el nombre **ProtectorPS4**.
4. Conéctese a dicha red utilizando la contraseña: **seguridad123**.
5. Abra un navegador web y acceda a la dirección **192.168.4.1**.
6. En la página que aparecerá, introduzca el nombre (SSID) y la contraseña de la red WiFi que proporcionará acceso a Internet.
7. Pulse el botón **Conectar**.

> **Nota:** Este procedimiento solo es necesario la primera vez que se utiliza el dispositivo o cuando se desea cambiar la red de salida. Los parámetros quedarán almacenados de forma persistente en la memoria flash del ESP32.

Una vez realizados estos pasos, el ESP32 dispondrá de conectividad a Internet y podrá ser utilizado para conectar su consola PlayStation 4 usando la red inalámbrica con el nombre **ProtectorPS4** y la contraseña: **seguridad123** , permitiendo la navegación y el uso de servicios en línea sin inconvenientes relacionados con actualizaciones forzadas.

## Guía de compilación y configuración WiFi

Para compilar este proyecto, utilice el entorno de desarrollo **ESP‑IDF** disponible en el siguiente enlace oficial:

👉  [https://idf.espressif.com/](https://idf.espressif.com/)

## Monitoreo de conexiones

Para visualizar las conexiones permitidas y bloqueadas, utilice el **monitor serie** del **Arduino IDE Legacy 1.8.19**.  
Puede descargar esta versión desde el sitio oficial:  
👉 [https://www.arduino.cc/en/software/](https://www.arduino.cc/en/software/)

A través del monitor serie podrá observar en tiempo real qué direcciones están siendo permitidas o bloqueadas por el sistema.

## Reporte de direcciones no bloqueadas

Si durante el monitoreo detecta direcciones que deberían estar bloqueadas pero no lo están, se solicita que abra un **issue** en el repositorio para añadir dicha dirección a la lista de bloqueo.

## Aviso importante

El uso de este software es bajo su exclusiva responsabilidad. El código fuente esta ubicado en la carpeta "`src`" main.c .
