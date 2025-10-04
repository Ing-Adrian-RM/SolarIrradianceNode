///////////////////////////////////////////////////////////////////////////////
// Central configuration header for Solar Irradiance Node Project
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_H
#define CONFIG_H

#define TOUCH_CS -1

///////////////////////////////////////////////////////////////////////////////
// Include necessary libraries and define global constants
///////////////////////////////////////////////////////////////////////////////
// ------------------------ Arduino Core ------------------------
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
// ------------------------ Modules ------------------------
#include "sensors_module.h"
#include "processing_module.h"
#include "storage_module.h"
#include "communication_module.h"
#include "display_module.h"
// ------------------------ Sensors ------------------------
#include <INA226.h>
#include <Adafruit_ADS1X15.h> 
// ------------------------ Display ------------------------
#include <TFT_eSPI.h> 
// ------------------------ Communication ------------------------
#include <LoRa.h>
// ------------------------ Storage ------------------------
#include <SD.h>
// ------------------------ Standard C++ ------------------------
#include <stdint.h>
#include <string>
#include <vector>
#include <ctime>


///////////////////////////////////////////////////////////////////////////////
// Global Objects
///////////////////////////////////////////////////////////////////////////////
extern TFT_eSPI tft;
extern INA226 ina226[6];
extern Adafruit_ADS1115 ads[2];

///////////////////////////////////////////////////////////////////////////////
// Global Variables and Constants
///////////////////////////////////////////////////////////////////////////////
extern uint8_t screen_id;
extern unsigned long lastSensorRead;
extern const uint8_t ADS1115_ADDR[2];

///////////////////////////////////////////////////////////////////////////////
// Global Pin Definitions
///////////////////////////////////////////////////////////////////////////////
extern const int LORA_SS;
extern const int LORA_DIO0;

#endif // CONFIG_H
