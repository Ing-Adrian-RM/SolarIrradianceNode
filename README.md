Solar Irradiance Node - TTGO LoRa32 V2.1 Project

This project implements an IoT solar irradiance measurement node using the TTGO LoRa32 V2.1 microcontroller (ESP32-based). The node measures the short-circuit current (Isc) of solar cells, compensates for temperature, stores data on an SD card, sends averaged measurements to a gateway via LoRa and upload data via Thingspeak Mathworks to cloud.

**Table of Contents:**

* Project Overview
* System Components
* Functional Description
* Project Structure
* Dependencies
* Setup and Configuration
* Usage
* Version Control
* License

**Project Overview:**

The Solar Irradiance Node is designed to work as a node in a distributed IoT network, communicating with a Raspberry Pi gateway via LoRa. It provides accurate solar irradiance measurements with temperature compensation, while storing data for backup and cloud upload to Thingspeak.

Key Features:

* Measures Isc using INA226 sensors.
* Reads cell temperature using ADS1115 with Littelfuse USP10982 thermistors.
* Supports six solar cell sets with independent measurement channels.
* Computes irradiance values with temperature compensation.
* Stores data every 10s in weekly CSV's format on SD card.
* Maintains a queue of files pending cloud upload (Thingspeak).
* Provides a modular dashboard on the TTGO display with multiple screens.
* Handles LoRa communication for IoT network data requests.
* Supports dual-core ESP32 processing: one core for data processing, one core for communication.

System Components:

* Microcontroller: TTGO LoRa32 V2.1_1.6 (ESP32-based)
* Solar Cells: 6 × IXOLARTM SolarBIT KXOB25-14X1F-TR solar cells
* Temperature sensors: 6 x Littelfuse USP10982 10kΩ thermistors (10kΩ SMD voltage divider)
* Current Sensors: 6 × INA226 current sensors
* ADCs: 2 × ADS1115 16-bit ADC (3 channels used per ADC)
* Data Storage and Communication:
    SD card for local backup
    LoRa antenna for node-gateway communication
    WiFi connectivity for cloud upload (Thingspeak)
    ESP32 dual-core handling processing and transmission threads
* Display: TTGO integrated TFT screen
    Multiple configurable screens (time, global irradiance, per-panel irradiance, WiFi status, gateway irradiance, etc.)

**Functional Description:**

1. Data Acquisition:
INA226 measures short-circuit current (Isc) for each solar cell set.
ADS1115 measures thermistor voltage, converting to temperature.
Each sensor module calibrated individually.

2. Data Processing:
Isc values converted to irradiance and temperature-compensated.
Data appended to weekly CSV files with 16 columns: timestamp, data_type, irra_cell_1 … irra_cell_6, temp_cell1 … temp_cell_6, average_irradiance, average_temperature.
Core 1 handles data processing; Core 2 handles transmission.

3. Transmission:
Node waits for gateway requests.
Sends a weighted average measurement over LoRa: weighted over time window since last request and weighted across six solar cell sets.

4. Cloud Backup:
Files created or modified are added to a queue (queue/pending.txt) for cloud upload.
Daily upload process checks queue and uploads files to Thingspeak.
Successfully uploaded files removed from queue; failed uploads retried later.

5. Display Management:
Modular screen system: general paint function updates the display, subfunctions define each screen.
Screens switched via a screen_id variable.

**Project Structure:**

SolarIrradianceNode/
├── include/                      # Headers (.h)
│   ├── sensors_module.h
│   ├── processing_module.h
│   ├── storage_module.h
│   ├── communication_module.h
│   ├── display_module.h
│   └── config.h                  # Definiciones globales (pines, direcciones I2C, etc.)
│
├── src/                          # Código fuente (.cpp)
│   ├── main.cpp
│   ├── sensors_module.cpp
│   ├── processing_module.cpp
│   ├── storage_module.cpp
│   ├── communication_module.cpp
│   ├── display_module.cpp
│
├── data/                         # Datos estáticos (opcional, se suben a SPIFFS/LittleFS)
│
├── logs/                         # Archivos de log en la SD (debug, errores, etc.)
│
├── results/                      # Resultados exportados (ej: CSV semanales en SD)
│
├── queue/                        # Cola de carga (ej: lista .txt de archivos pendientes para Thingspeak)
│
├── lib/                          # Librerías locales propias (si decides crear helpers reusables)
│
├── test/                         # Pruebas unitarias
│
└── platformio.ini

**Dependencies**

The project relies on several Arduino libraries (installable via PlatformIO): Adafruit INA226, Adafruit ADS1X15, SD library, SPI library, LoRa (for TTGO LoRa32), WiFi (for ESP32 cloud upload). PlatformIO will automatically handle library installation if listed in platformio.ini under lib_deps.

**Setup and Configuration**

Clone the repository: git clone https://github.com/Ing-Adrian-RM/SolarIrradianceNode.git

Initialize PlatformIO environment: pio project init --board ttgo-lora32-v21.

Configure pins, I2C addresses, and screen IDs in include/config.h.

Compile and upload: pio run --target upload.

Open Serial Monitor to debug: pio device monitor.

**Usage**

Data is sampled every 10 seconds.

CSV files are created weekly in results/.

Node responds to gateway requests via LoRa every 5 minutes (configurable).

Cloud upload process runs daily (configurable) and checks queue/pending.txt.

Display can be navigated by changing the screen_id variable.

**Version Control**

Recommended workflow using Git: git status, git add ., git commit -m "Description of changes", git push origin main.

Common commands: git pull origin main → sync with remote, git checkout -b feature-branch → create new branch, git merge feature-branch → merge branch into main, git log --oneline → see commit history.

**License**

This project is open-source under the MIT License. See LICENSE for details.