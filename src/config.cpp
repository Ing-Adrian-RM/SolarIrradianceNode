///////////////////////////////////////////////////////////////////////////////
// config.cpp: Central configuration source for Solar Irradiance Node Project
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#include "config.h"

///////////////////////////////////////////////////////////////////////////////
// Global Objects
///////////////////////////////////////////////////////////////////////////////
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
INA226 ina226[6] = {
    INA226(0x41),
    INA226(0x45),
    INA226(0x46),
    INA226(0x40),
    INA226(0x4D),
    INA226(0x44)};
Adafruit_ADS1115 ads[2];
SPIClass *hspi = NULL;

SOLAR_CELL_LIST_PTR panel_list = NULL;
SOLAR_CELL panel_1;
SOLAR_CELL panel_2;
SOLAR_CELL panel_3;
SOLAR_CELL panel_4;
SOLAR_CELL panel_5;
SOLAR_CELL panel_6;
SOLAR_AVG panel_avg;

///////////////////////////////////////////////////////////////////////////////
// Global Variables and Constants
///////////////////////////////////////////////////////////////////////////////
// ------------------------ WiFi Credentials ------------------------
const char *ssid = "AmbientesControlados";
const char *password = "Modulos@AmbCont23";
const char *ssid_P = "Adrian";
const char *password_P = "12345678";
// ------------------------ ThinkSpeak Credentials ------------------------
const char *writeAPIKey_5min = "H3NC7X8M9F15BRWZ";
const char *writeAPIKey_calibration = "90OIXJE7J7XXUJTA";
const char *writeAPIKey_15sec = "PXIGFJ6S7XHCWW0I";
const char *thingspeakServer = "http://api.thingspeak.com/update";
// ------------------------ Panel data (datasheet) ------------------------
const float Isc_ref_mA = 58.6;     // mA
const float G_ref = 1000.0;        // Irradiance W/m²
const float alpha_Isc = 0.0004526; // Relative thermal coefficient by ° C
const float T_ref = 25.0;          // °C
// ------------------------ INA226 Calibration Variables ------------------------
const float shunt_resistance = 0.5;
const float max_expected_current = 0.06;
// ------------------------ Spektron Calibration Variables ------------------------
bool calibration_mode = true;
float Spektron_reading = 0.0f;
extern const float Spektron_voltage_reference = 0.07915f;   // V for 1000 W/m²
extern const float Spektron_irradiance_reference = 1000.0f; // W/m² for calibration
float spektron_avg_irr = 0.0f;
uint16_t average_cal_count = 0;
bool cal_buffer_used = false;
// ------------------------ Linear Regression Calibration (Coefficients from Matlab Analisis) ------------------------
float calib_a[6] = {1.055414257, 1.038356658, 1.051318847, 1.033020218, 1.035471421, 1.048492097}; //Slope
float calib_b[6] = {-0.066059821, -1.020814655, -1.269966364, 0.774419482, -0.185765026, -0.553767494}; //Intercept
// ------------------------ ADS I2C directions ------------------------
const uint8_t ADS1115_ADDR[2] = {0x49, 0x48};
const float ADS1115_LSB_GAIN_ONE = 0.000125f; // V per bit
// ------------------------ Display ------------------------
uint8_t screen_id = 0;
// ------------------------ Data processing ------------------------
unsigned long lastSensorRead = 0;
unsigned long lastCalRead = 0;
// ------------------------ Thermistor constants ------------------------
const float R_PULLUP = 10000.0f;
const float R0 = 10000.0f; // Thermistor resistance at T0 (ohms)
const float BETA = 3892.0f;
const float T0_K = 298.15f; // Reference temperature 25°C in Kelvin
float VCC = 4.0959f;
// ------------------------ Display ------------------------
unsigned long last_screen_change = 0;
int current_screen = 0;
const unsigned long SCREEN_INTERVAL = 5000;
// ------------------------ Transmission ------------------------
char tx_buffer[BUFFER_SIZE] = {0};
volatile bool buffer_ready = false;
volatile bool upload_ready = false;
int rssi = 0;
float snr = 0.0f;
String url = "";