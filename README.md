# Solar Irradiance Node

Nodo IoT basado en ESP32 (TTGO LoRa32 V2.1) para medir irradiancia solar, compensar temperatura, almacenar series de tiempo en SD y publicar promedios mediante LoRa y ThingSpeak.

## 📡 Resumen del sistema
- **Adquisición**: 6 sensores INA226 miden corriente de cortocircuito (Isc) y 6 termistores USP10982 se leen con dos ADC ADS1115 para obtener temperatura.【F:src/sensors_module.cpp†L41-L104】【F:src/processing_module.cpp†L40-L83】
- **Procesamiento**: Se corrige térmicamente la Isc, se convierte a irradiancia y se calcula un promedio general y por panel, además de promedios acumulados para transmisiones.【F:src/processing_module.cpp†L63-L164】
- **Comunicación**: Las lecturas promedio se guardan en un buffer que responde peticiones LoRa del gateway (`NODE1`) y se publican en ThingSpeak a intervalos de 15 s y 5 min (modo normal o calibración).【F:src/communication_module.cpp†L10-L107】【F:src/processing_module.cpp†L106-L197】
- **Interfaz local**: La pantalla OLED integrada rota entre pantallas de estado, datos por panel, promedios y estado de transmisión.【F:src/display_module.cpp†L12-L158】
- **Almacenamiento**: Cada 15 s se registra una fila en CSV semanal en la SD (modo normal o calibración) con sello de tiempo y lecturas por panel.【F:src/main.cpp†L29-L47】【F:src/storage_module.cpp†L24-L104】

## 🧱 Arquitectura del código
```
include/
├── communication_module.h   # API de LoRa/WiFi y ThingSpeak
├── config.h                 # Pines, constantes globales y prototipos
├── display_module.h         # Funciones de interfaz OLED
├── processing_module.h      # Procesamiento numérico y buffers
├── sensors_module.h         # Inicialización/lecturas de sensores
├── storage_module.h         # Manejo de SD y archivos CSV
└── thermistor_utils.h       # Conversión ADC → temperatura

src/
├── config.cpp               # Instancias globales y constantes
├── main.cpp                 # `setup()` y `loop()` principales
├── *.cpp                    # Implementación de cada módulo
```

Cada módulo expone funciones declaradas en `include/` y usa las variables globales definidas en `src/config.cpp` para compartir estado entre componentes.【F:src/config.cpp†L1-L91】

## 🛠️ Requisitos
- Placa **TTGO LoRa32 V2.1 (ESP32)**.
- Sensores: 6 × INA226, 2 × ADS1115, 6 × termistores Littelfuse USP10982, referencia Spektron opcional para calibración.
- Almacenamiento **microSD** conectada al bus HSPI (pines configurados en `config.h`).【F:include/config.h†L11-L36】【F:src/storage_module.cpp†L12-L23】
- Antena LoRa y conectividad WiFi para sincronización NTP y envío a ThingSpeak.

## 📦 Dependencias de software
El proyecto usa PlatformIO con framework Arduino. Las librerías se instalan automáticamente desde `platformio.ini`:

- `Adafruit ADS1X15`
- `Adafruit BusIO`
- `LoRa`
- `INA226` (robtillaart)
- `SD`
- `U8g2`

Configura el entorno ejecutando:
```bash
pio run
```
Esto descargará las dependencias y compilará el firmware para la tarjeta `ttgo-lora32-v21`.【F:platformio.ini†L1-L20】

## ⚙️ Configuración inicial
1. **Credenciales y constantes**: revisa `src/config.cpp` para ajustar SSID/contraseñas WiFi, claves de ThingSpeak, parámetros de calibración de paneles y constantes de termistor.
2. **Pines**: verifica asignaciones en `include/config.h` (SPI LoRa, HSPI SD, parámetros OLED y LoRa).
3. **Modo de calibración**: la variable global `calibration_mode` arranca en `true`; cámbiala a `false` cuando finalices la etapa de calibración para generar CSV y URLs estándar.【F:src/config.cpp†L40-L67】
4. **Zona horaria**: `configTime(-6 * 3600, ...)` está ajustado para CST (Costa Rica). Modifícalo si trabajas en otra región.【F:src/communication_module.cpp†L22-L39】

## 🔄 Flujo de ejecución
1. `setup()` inicializa SD, sensores, pantalla, comunicación y estructuras de paneles, seguido de calibración de INA226 y ADS1115.【F:src/main.cpp†L12-L26】
2. `loop()` refresca la pantalla, atiende solicitudes LoRa y cada 15 s lee sensores, procesa datos, guarda en CSV y actualiza buffers de transmisión/ThingSpeak.【F:src/main.cpp†L28-L47】
3. Cada solicitud LoRa con payload `NODE1` envía el paquete `NODE1|timestamp|irradiancia_promedio`. Si no hay datos listos se responde `NO_DATA`.【F:src/communication_module.cpp†L48-L97】【F:src/processing_module.cpp†L117-L148】
4. Las funciones `thinkspeak_url_15sec()` y `thinkspeak_url_5min_cal()` generan URLs con promedios acumulados (por panel y global) y disparan `upload_to_thinkspeak()` para publicar en la nube.【F:src/processing_module.cpp†L150-L197】【F:src/processing_module.cpp†L199-L224】

## 🗃️ Archivos CSV en SD
- **Modo normal**: `W<semana>_<año>_irradiance.csv` incluye Isc, irradiancia y temperatura por panel más los promedios agregados.
- **Modo calibración**: `W<semana>_<año>_calibration.csv` almacena irradiancia por panel, promedio y lectura del Spektron.

Los encabezados se crean automáticamente cuando inicia una nueva semana y se anexan filas en cada ciclo de adquisición.【F:src/storage_module.cpp†L43-L102】

## 📺 Pantallas OLED disponibles
1. Estado general (WiFi, hora/fecha).
2. Paneles 1-2 (Isc, irradiancia, temperatura).
3. Paneles 3-4.
4. Paneles 5-6.
5. Promedios globales.
6. Estado de transmisión (buffer listo, RSSI, SNR).

La pantalla cambia cada 5 s (`SCREEN_INTERVAL`). Ajusta este valor en `src/config.cpp` si necesitas otro ritmo.【F:src/display_module.cpp†L60-L154】【F:src/config.cpp†L83-L88】

## 🛰️ Calibración y Spektron
Cuando `calibration_mode` es `true`:
- Se acumulan promedios cortos (`AVG_COUNT_THINKSPEAK_THRESHOLD_CALIBRATION`) antes de enviar a ThingSpeak.
- Se lee la referencia Spektron por ADS1115 canal 3 y se calcula su irradiancia usando una conversión lineal con constantes definidas en `config.cpp`.
- El valor medio del Spektron se publica en `field8` de ThingSpeak y se guarda en CSV de calibración.【F:include/config.h†L63-L90】【F:src/processing_module.cpp†L150-L224】【F:src/processing_module.cpp†L226-L247】

## 🚀 Desarrollo y despliegue
1. Compila y sube el firmware:
   ```bash
   pio run --target upload
   ```
2. Abre el monitor serie para verificar inicialización, calibraciones y tráfico LoRa:
   ```bash
   pio device monitor
   ```
3. Durante pruebas puedes habilitar mensajes adicionales gracias a `CORE_DEBUG_LEVEL=5` configurado en `platformio.ini`.

## 📚 Recursos adicionales
- [Documentación PlatformIO](https://docs.platformio.org/)
- [TTGO LoRa32 V2.1](https://github.com/LilyGO/TTGO-T-Beam)
- [ThingSpeak](https://thingspeak.com/docs)

---
¿Tienes dudas o quieres extender el proyecto? Explora los módulos en `src/` para añadir nuevos sensores, modificar la lógica de promedios o ampliar la interfaz gráfica.
