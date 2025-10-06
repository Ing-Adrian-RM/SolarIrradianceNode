///////////////////////////////////////////////////////////////////////////////
// Sensors Module
// Description: This module handles sensor initialization and readings.
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#include "config.h"

///////////////////////////////////////////////////////////////////////////////
// initialize_sensors: Initialize INA226 and ADS1115 sensors
///////////////////////////////////////////////////////////////////////////////
void initialize_sensors()
{
    if (!ads[0].begin(ADS1115_ADDR[0]))
        Serial.println("Error: ADS1115 #1 T1,T2,T3 failed");
    else
        Serial.println("ADS1115 #1 T1,T2,T3 initialized");

    if (!ads[1].begin(ADS1115_ADDR[1]))
        Serial.println("Error: ADS1115 #2 T4,T5,T6 failed");
    else
        Serial.println("ADS1115 #2 T4,T5,T6 initialized");

    if (!ina226[0].begin())
        Serial.println("Error: INA226_P1_0x41 failed");
    else
        Serial.println("INA226_P1_0x41 initialized");

    if (!ina226[1].begin())
        Serial.println("Error: INA226_P2_0x45 failed");
    else
        Serial.println("INA226_P2_0x45 initialized");

    if (!ina226[2].begin())
        Serial.println("Error: INA226_P3_0x46 failed");
    else
        Serial.println("INA226_P3_0x46 initialized");

    if (!ina226[3].begin())
        Serial.println("Error: INA226_P4_0x40 failed");
    else
        Serial.println("INA226_P4_0x40 initialized");

    if (!ina226[4].begin())
        Serial.println("Error: INA226_P5_0x4D failed");
    else
        Serial.println("INA226_P5_0x4D initialized");

    if (!ina226[5].begin())
        Serial.println("Error: INA226_P6_0x44 failed");
    else
        Serial.println("INA226_P6_0x44 initialized");
}

///////////////////////////////////////////////////////////////////////////////
// calibrate_ina226_sensors: Calibrate INA226 sensors if needed
///////////////////////////////////////////////////////////////////////////////
void calibrate_ina226_sensors()
{

    for (int i = 0; i < 6; i++)
    {
        // Set number of averages (1024 = maximum precision)
        ina226[i].setAverage(INA226_512_SAMPLES);

        // Set conversion time for both bus and shunt (maximum precision)
        ina226[i].setBusVoltageConversionTime(INA226_8300_us);
        ina226[i].setShuntVoltageConversionTime(INA226_8300_us);

        // Continuous measurement mode for both shunt and bus voltage
        ina226[i].setModeShuntBusContinuous();

        // Configure calibration based on shunt and expected current
        ina226[i].setMaxCurrentShunt(max_expected_current, shunt_resistance);

        // Avoid I2C congestion
        delay(50);

        Serial.print("INA226 P");
        Serial.print(i + 1);
        Serial.println(" calibrated successfully.");
    }

    Serial.print("All INA226 sensors calibrated, sample time for each: 8.445056 s.");
}

///////////////////////////////////////////////////////////////////////////////
// calibrate_ina226_sensors: Calibrate INA226 sensors if needed
///////////////////////////////////////////////////////////////////////////////
void calibrate_ads1115_sensors()
{

    for (int i = 0; i < 2; i++)
    {
        // Check and correct the data rate if necessary
        int data_rate = ads[i].getDataRate();
        if (data_rate != 128)
        {
            ads[i].setDataRate(128); // Set to 128 SPS for ADS1115
            data_rate = 128;
        }
        Serial.printf("ADC %d data rate set to %d SPS.\n", i + 1, data_rate);

        // Check and correct the gain if necessary
        adsGain_t ads_gain = ads[i].getGain();
        if (ads_gain != GAIN_ONE)
        {
            ads[i].setGain(GAIN_ONE); // Set to 1x gain for ADS1115
            ads_gain = GAIN_ONE;
        }

        // Avoid I2C congestion
        delay(50);

        Serial.print("ADC ");
        Serial.print(i + 1);
        Serial.println(" calibrated successfully.");
    }

    Serial.print("All ADC's sensors calibrated, sample time for each: 15.6 ms.");
}

///////////////////////////////////////////////////////////////////////////////
// read_ina226_sensors: Read current values from INA226 sensors
///////////////////////////////////////////////////////////////////////////////
void read_ina226_sensors()
{

    int panel_index = 0;
    for (SOLAR_CELL_LIST_PTR ptr = panel_list; ptr != NULL; ptr = ptr->next)
    {
        unsigned long start_time = millis();
        while (!ptr->panel->ina226_sensor->isConversionReady())
        { // Wait until conversion is ready
            delay(10);
        }
        float current_mA = ptr->panel->ina226_sensor->getCurrent_mA();
        float irradiance = Isc_to_irradiance(current_mA, ptr->panel->Temperature);
        ptr->panel->Irradiance = irradiance;
        ptr->panel->Isc = current_mA;
        Serial.printf("Panel %d -> Isc: %.3f mA, Irradiance: %.2f W/m²\n", panel_index + 1, current_mA, irradiance);
        delay(50); // Avoid I2C congestion
        panel_index++;
    }
    panel_data_average();
}

///////////////////////////////////////////////////////////////////////////////
// read_ads1115_sensors: Read temperature values from ADS1115 sensors
///////////////////////////////////////////////////////////////////////////////
void read_ads1115_sensors()
{

    int panel_index = 0;
    for (SOLAR_CELL_LIST_PTR ptr = panel_list; ptr != NULL; ptr = ptr->next)
    {
        if (panel_index < 3)
        {
            ptr->panel->ads_sensor->readADC_SingleEnded(panel_index); // discard first sample for settling
            delay(50);                                                // Wait for settling, 5T ~50 ms for RC filter with R=10k and C=1uF.
            int16_t raw = ptr->panel->ads_sensor->readADC_SingleEnded(panel_index);
            float tempC = ads_raw_to_celsius(raw);
            ptr->panel->Temperature = tempC;
            delay(50); // Avoid I2C congestion
            Serial.printf("Panel %d Temperature: %.2f C\n", panel_index + 1, tempC);
        }
        else
        {
            int ch = panel_index - 3;
            ptr->panel->ads_sensor->readADC_SingleEnded(ch);
            delay(50);
            int16_t raw = ptr->panel->ads_sensor->readADC_SingleEnded(ch);
            float tempC = ads_raw_to_celsius(raw);
            ptr->panel->Temperature = tempC;
            Serial.printf("Panel %d Temperature: %.2f C\n", panel_index + 1, tempC);
            delay(50);
        }
        panel_index++;
    }
}
