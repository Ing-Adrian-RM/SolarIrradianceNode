///////////////////////////////////////////////////////////////////////////////
// Main Program
// Solar Irradiance Node with LoRa for IoT Applications
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#include <config.h>

///////////////////////////////////////////////////////////////////////////////
// setup: Initialization function called once at startup
///////////////////////////////////////////////////////////////////////////////
void setup()
{
    Serial.begin(115200);
    initialize_sd();
    initialize_sensors();
    initialize_display();
    initialize_communication();
    calibrate_ina226_sensors();
    calibrate_ads1115_sensors();
    initialize_panels_structs();
}

///////////////////////////////////////////////////////////////////////////////
// loop: Main loop function called repeatedly
///////////////////////////////////////////////////////////////////////////////
void loop()
{

    update_display();
    handle_lora_requests();

    unsigned long currentMillis = millis();
    if (currentMillis - lastSensorRead >= 10000)
    {
        lastSensorRead = currentMillis;
        read_ads1115_sensors();
        read_ina226_sensors();
        save_data_to_csv();
        transmission_buffer();
        thinkspeak_url();
    }
}
