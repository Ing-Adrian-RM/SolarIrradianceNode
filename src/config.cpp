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
const float G_ref = 1000.0;        // W/m²
const float alpha_Isc = 0.0004526; // Relative thermal coefficient by ° C
const float T_ref = 25.0;          // °C

// ------------------------ INA226 Calibration Variables ------------------------
const float shunt_resistance = 0.5;      // Ohms
const float max_expected_current = 0.06; // Amperes (60 mA)

// ------------------------ Spektron Calibration Variables ------------------------
bool calibration_mode = false;                              // Set true for calibration mode
float Spektron_reading = 0.0f;                              // Raw voltage reading
extern const float Spektron_voltage_reference = 0.07915f;   // V at 1000 W/m²
extern const float Spektron_irradiance_reference = 1000.0f; // W/m²
float spektron_avg_irr = 0.0f;                              // Acumulated average irradiance
uint16_t average_cal_count = 0;                             // Number of samples accumulated
bool cal_buffer_used = false;                               // Flag to indicate last buffer has been used

// ------------------------ Linear Regression Calibration ------------------------
// ------------------------ (Coefficients from Matlab Analisis) ------------------------
float calib_a[6] = {1.055414257, 1.038356658, 1.051318847, 1.033020218, 1.035471421, 1.048492097};      // Slope
float calib_b[6] = {-0.066059821, -1.020814655, -1.269966364, 0.774419482, -0.185765026, -0.553767494}; // Intercept

// ------------------------ ADS Calibration Variables ------------------------
const uint8_t ADS1115_ADDR[2] = {0x49, 0x48}; // ADS1115 I2C addresses
const float ADS1115_LSB_GAIN_ONE = 0.000125f; // V per bit

// ------------------------ Data processing ------------------------
unsigned long lastSensorRead = 0;
unsigned long lastCalRead = 0;

// ------------------------ Thermistor constants ------------------------
const float R_PULLUP = 10000.0f; // Voltage divider resistor value (ohms)
const float R0 = 10000.0f;       // Thermistor resistance at T0 (ohms)
const float BETA = 3892.0f;      // Beta parameter (K)
const float T0_K = 298.15f;      // Reference temperature 25°C in Kelvin
float VCC = 4.0959f;             // Supply voltage (V)

// ------------------------ Display ------------------------
uint8_t screen_id = 0;
unsigned long last_screen_change = 0;
int current_screen = 0;
const unsigned long SCREEN_INTERVAL = 5000;
// ------------------------ Transmission ------------------------
char tx_buffer[BUFFER_SIZE] = {0};  // Transmission buffer
volatile bool buffer_ready = false; // Flag to indicate buffer is ready for transmission
int rssi = 0;                       // Received Signal Strength Indicator
float snr = 0.0f;                   // Signal-to-Noise Ratio
String url = "";                    // URL for HTTP requests
const uint8_t NODE_ADDR = 22;       // TTGO
const uint8_t GATEWAY_ADDR = 30;    // Gateway
uint8_t g_msg_id = 0;               // Package ID
uint32_t timeout_ms = 50;           // Transmission time_out in ms