///////////////////////////////////////////////////////////////////////////////
// Central configuration header for Solar Irradiance Node Project
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_H
#define CONFIG_H

#define TOUCH_CS -1

#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_CS    18
#define LORA_RST   23
#define LORA_DIO0  26
#define LORA_FREQ_HZ  915E6
#define LORA_BW_HZ    125E3
#define LORA_SF       7
#define LORA_CR_DEN   5

#define BUFFER_SIZE 128
#define AVG_COUNT_THRESHOLD 12
#define NO_REQUEST_THRESHOLD 30

#define HSPI_MISO   02
#define HSPI_MOSI   15
#define HSPI_SCLK   14
#define HSPI_SS     13

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3D
#define OLED_RESET     -1
///////////////////////////////////////////////////////////////////////////////
// Include necessary libraries and define global constants
///////////////////////////////////////////////////////////////////////////////
// ------------------------ Arduino Core ------------------------
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
// ------------------------ Sensors ------------------------
#include <INA226.h>
#include <Adafruit_ADS1X15.h> 
// ------------------------ Display ------------------------
#include <U8x8lib.h>
#include <U8g2lib.h>
// ------------------------ Communication ------------------------
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
// ------------------------ Storage ------------------------
#include <SD.h>
// ------------------------ Standard C++ ------------------------
#include <stdint.h>
#include <string>
#include <vector>
#include <ctime>
#include <math.h>
#include <iostream>


///////////////////////////////////////////////////////////////////////////////
// Global Objects
///////////////////////////////////////////////////////////////////////////////
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;
extern INA226 ina226[6];
extern Adafruit_ADS1115 ads[2];
extern SPIClass * hspi; // HSPI instance for SD.

typedef struct solar_cell{
    INA226 *ina226_sensor;
    Adafruit_ADS1115 *ads_sensor;
    float Isc,Irradiance,Temperature = 0.0F;
}SOLAR_CELL, *SOLAR_CELL_PTR;

typedef struct solar_avg{
    char time_stamp[20];
    float Isc,Irradiance,Temperature = 0.0F;

}SOLAR_AVG, *SOLAR_AVG_PTR;

typedef struct solar_cell_list{
    solar_cell *panel;
    struct solar_cell_list *next;

}SOLAR_CELL_LIST, *SOLAR_CELL_LIST_PTR;

extern SOLAR_CELL_LIST_PTR panel_list;
extern SOLAR_CELL panel_1;
extern SOLAR_CELL panel_2;
extern SOLAR_CELL panel_3;
extern SOLAR_CELL panel_4;
extern SOLAR_CELL panel_5;
extern SOLAR_CELL panel_6;
extern SOLAR_AVG panel_avg;

extern char tx_buffer[BUFFER_SIZE];
extern volatile bool buffer_ready;

///////////////////////////////////////////////////////////////////////////////
// Global Variables and Constants
///////////////////////////////////////////////////////////////////////////////
// ------------------------ WiFi Credentials ------------------------
extern const char* ssid;
extern const char* password;
// ------------------------ ThinkSpeak Credentials ------------------------
extern const char* writeAPIKey;
extern const char* thingspeakServer;
// ------------------------ Panel data (datasheet) ------------------------
extern const float Isc_ref_mA;
extern const float G_ref;
extern const float alpha_Isc;
extern const float T_ref;
// ------------------------ INA226 Calibration Variables ------------------------
extern const float shunt_resistance;
extern const float max_expected_current;
// ------------------------ ADS I2C directions ------------------------
extern const uint8_t ADS1115_ADDR[2];
extern const float ADS1115_LSB_GAIN_ONE;
// ------------------------ Display ------------------------
extern uint8_t screen_id;
// ------------------------ Data processing ------------------------
extern unsigned long lastSensorRead;
// ------------------------ Thermistor constants ------------------------
extern const float R_PULLUP;
extern const float R0;   
extern const float BETA;
extern const float T0_K;
extern const float VCC;
// ------------------------ Display ------------------------
extern unsigned long last_screen_change;
extern int current_screen;
extern const unsigned long SCREEN_INTERVAL;
// ------------------------ Transmission ------------------------
extern char tx_buffer[BUFFER_SIZE];
extern volatile bool buffer_ready;
extern volatile bool upload_ready;
extern int rssi;
extern float snr;
extern String url;

///////////////////////////////////////////////////////////////////////////////
// Module Includes
///////////////////////////////////////////////////////////////////////////////
// ------------------------ Modules ------------------------
#include "processing_module.h"
#include "sensors_module.h"
#include "storage_module.h"
#include "communication_module.h"
#include "display_module.h"
#include "thermistor_utils.h"

#endif // CONFIG_H
