# Solar Irradiance Node

IoT node built around the ESP32-based TTGO LoRa32 V2.1 board to capture solar irradiance, perform thermal compensation, log time series to a microSD card, and publish processed statistics through LoRa and ThingSpeak.

## 📡 System overview
- **Acquisition**: Six INA226 current sensors capture short-circuit current (Isc) for each panel IXOLAR KXOB25-14X1F-TR, while six Littelfuse USP10982 thermistors are digitized through two ADS1115 converters to obtain panel temperatures.
- **Processing**: Isc readings are temperature-compensated, converted into irradiance, aggregated per panel, and combined into global averages for local display, storage, and radio transmission.
- **Communication**: The node replies to LoRa gateway requests (`NODE1`) with the latest averaged packet and periodically pushes 15 s and 5 min summaries to ThingSpeak (normal or calibration mode).
- **Local interface**: The onboard OLED cycles through system status, per-panel metrics, global averages, and transmission state screens.
- **Storage**: Every 15 seconds, a new row is appended to a weekly CSV file stored on the SD card, with timestamps, Isc, irradiance, temperature, and Spektron reference (when available).

## 🧱 Code architecture
```
include/
├── communication_module.h   # LoRa/Wi-Fi and ThingSpeak interface
├── config.h                 # Pin map, global constants, forward declarations
├── display_module.h         # OLED utilities and screen rotation
├── processing_module.h      # Numerical processing pipelines
├── sensors_module.h         # Sensor initialization and sampling routines
├── storage_module.h         # SD card writer utilities
└── thermistor_utils.h       # ADC-to-temperature conversion helpers

src/
├── config.cpp               # Global instances and constant definitions
├── main.cpp                 # `setup()` and `loop()` entry points
├── *.cpp                    # Module implementations
```

Each module exposes the public interface declared in `include/` and uses the shared objects instantiated in `src/config.cpp`.

## 🛠️ Hardware requirements
- TTGO LoRa32 V2.1 (ESP32) development board.
- Six INA226 current sensors wired to the primary I²C bus.
- Two ADS1115 ADCs for thermistor and Spektron readings.
- Six Littelfuse USP10982 thermistors mounted on the panels.
- Optional Spektron reference pyranometer for calibration.
- MicroSD card connected over HSPI as defined in `include/config.h`.
- LoRa antenna and Wi-Fi connectivity for synchronization and cloud uploads.

## 📦 Software dependencies
This project targets PlatformIO with the Arduino framework. Required libraries are resolved automatically through `platformio.ini`:

- Adafruit ADS1X15
- Adafruit BusIO
- LoRa
- INA226 (Rob Tillaart)
- SD
- U8g2

Initialize the environment by running:
```bash
pio run
```
This command fetches dependencies and compiles the firmware for `ttgo-lora32-v21`.

## ⚙️ Initial configuration
1. **Credentials and calibration constants**: Inspect `src/config.cpp` to configure Wi-Fi SSID/password, ThingSpeak API keys, per-panel calibration parameters, and thermistor coefficients.
2. **Pin assignments**: Review `include/config.h` for SPI, I²C, OLED, and LoRa pin mappings.
3. **Calibration mode**: The global `calibration_mode` flag is `true` by default. Set it to `false` once calibration is completed to switch the CSV naming scheme and ThingSpeak endpoints.
4. **Timezone**: Update the `configTime` call in `src/communication_module.cpp` if the device operates outside the default CST (UTC−6) offset.

## 🔄 Execution flow
1. `setup()` initializes the SD card, sensors, display, communication stack, and data structures, followed by INA226/ADS1115 calibration routines.
2. `loop()` rotates OLED screens, handles LoRa packets, performs 15 s acquisition cycles, processes irradiance metrics, appends CSV rows, and updates ThingSpeak buffers.
3. Upon receiving the `NODE1` request, the node returns `NODE1|timestamp|avg_irradiance`; otherwise it replies `NO_DATA` when buffers are empty.
4. `thinkspeak_url_15sec()` and `thinkspeak_url_5min_cal()` prepare REST payloads and trigger `upload_to_thinkspeak()` to publish accumulated averages.

## 🗃️ SD card CSV layout
- **Normal mode**: `W<week>_<year>_irradiance.csv` stores Isc, irradiance, and temperature per panel together with aggregate statistics.
- **Calibration mode**: `W<week>_<year>_calibration.csv` includes per-panel irradiance, global averages, and Spektron reference readings.

Headers are regenerated at the start of each ISO week, and subsequent acquisition cycles append new entries.

## 📺 OLED screen rotation
1. System status (Wi-Fi, RTC timestamp).
2. Panels 1–2 (Isc, irradiance, temperature).
3. Panels 3–4.
4. Panels 5–6.
5. Global averages.
6. Transmission status (buffer readiness, RSSI, SNR).

The default rotation interval is 5 seconds; adjust `SCREEN_INTERVAL` in `src/config.cpp` if a different cadence is required.

## 🛰️ Calibration with Spektron
When `calibration_mode` is enabled:
- ThingSpeak uploads use `AVG_COUNT_THINKSPEAK_THRESHOLD_CALIBRATION` samples per batch.
- ADS1115 channel 3 captures the Spektron voltage and converts it to irradiance through linear coefficients declared in `src/config.cpp`.
- The Spektron average is published to ThingSpeak `field8` and stored in calibration CSV files.

## 🌐 External data repositories
- Google Drive project archive: _add shared link here_
- ThingSpeak 15-second channel: https://thingspeak.mathworks.com/channels/3116915
- ThingSpeak 5-minute channel: https://thingspeak.mathworks.com/channels/3100981
- ThingSpeak calibration channel: https://thingspeak.mathworks.com/channels/3100981

## 🚀 Build and deploy
1. Compile and upload the firmware:
   ```bash
   pio run --target upload
   ```
2. Open the serial monitor to verify initialization, calibration steps, and LoRa traffic:
   ```bash
   pio device monitor
   ```
3. Diagnostic logging is available through `CORE_DEBUG_LEVEL=5` configured in `platformio.ini`.

## 📚 References
- [PlatformIO Documentation](https://docs.platformio.org/)
- [TTGO LoRa32 V2.1](https://github.com/LilyGO/TTGO-T-Beam)
- [ThingSpeak Guides](https://thingspeak.com/docs)