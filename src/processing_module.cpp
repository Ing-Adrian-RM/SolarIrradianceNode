///////////////////////////////////////////////////////////////////////////////
// Processing Module
// Description: This module processes sensor data.
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#include "config.h"

///////////////////////////////////////////////////////////////////////////////
// initialize_panels_structs: Initialize solar panel structs and linked list
///////////////////////////////////////////////////////////////////////////////
void initialize_panels_structs()
{
    // Initialize the panels & linked list
    panel_list = NULL;
    panel_1.ina226_sensor = &ina226[0];
    panel_1.ads_sensor = &ads[0];
    panel_list = insert_panel(panel_list, &panel_1);
    panel_2.ina226_sensor = &ina226[1];
    panel_2.ads_sensor = &ads[0];
    panel_list = insert_panel(panel_list, &panel_2);
    panel_3.ina226_sensor = &ina226[2];
    panel_3.ads_sensor = &ads[0];
    panel_list = insert_panel(panel_list, &panel_3);
    panel_4.ina226_sensor = &ina226[3];
    panel_4.ads_sensor = &ads[1];
    panel_list = insert_panel(panel_list, &panel_4);
    panel_5.ina226_sensor = &ina226[4];
    panel_5.ads_sensor = &ads[1];
    panel_list = insert_panel(panel_list, &panel_5);
    panel_6.ina226_sensor = &ina226[5];
    panel_6.ads_sensor = &ads[1];
    panel_list = insert_panel(panel_list, &panel_6);
}

///////////////////////////////////////////////////////////////////////////////
// insert_panel: Insert a solar panel into the linked list
///////////////////////////////////////////////////////////////////////////////
SOLAR_CELL_LIST_PTR insert_panel(SOLAR_CELL_LIST_PTR list, solar_cell *panel)
{
    SOLAR_CELL_LIST_PTR new_node = (SOLAR_CELL_LIST_PTR)malloc(sizeof(SOLAR_CELL_LIST));
    new_node->panel = panel;
    new_node->next = NULL;

    if (list == NULL)
    {
        return new_node;
    }
    else
    {
        for (SOLAR_CELL_LIST_PTR ptr = list; ptr != NULL; ptr = ptr->next)
        {
            if (ptr->next == NULL)
            {
                ptr->next = new_node;
                return list;
            }
        }
    }
    return list;
}

///////////////////////////////////////////////////////////////////////////////
// Isc_to_irradiance: Convert Isc to irradiance using temperature compensation
///////////////////////////////////////////////////////////////////////////////
float Isc_to_irradiance(float current_mA, float temperature_C)
{
    float Isc_corr = current_mA / (1 + alpha_Isc * (temperature_C - T_ref)); // Thermal compensation of Isc
    float G = G_ref * (Isc_corr / Isc_ref_mA);                               // Irradiance calculation
    return G;
}

///////////////////////////////////////////////////////////////////////////////
// panel_data_average: Average data from all panels
///////////////////////////////////////////////////////////////////////////////
void panel_data_average()
{
    float Isc_avg = 0;
    float Irradiance_avg = 0;
    float Temperature_avg = 0;
    for (SOLAR_CELL_LIST_PTR ptr = panel_list; ptr != NULL; ptr = ptr->next)
    {
        Isc_avg += ptr->panel->Isc;
        Irradiance_avg += ptr->panel->Irradiance;
        Temperature_avg += ptr->panel->Temperature;
    }
    Isc_avg /= 6.0F;
    Irradiance_avg /= 6.0F;
    Temperature_avg /= 6.0F;
    panel_avg.Isc = Isc_avg;
    panel_avg.Irradiance = Irradiance_avg;
    panel_avg.Temperature = Temperature_avg;
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    char time_stamp[20];
    strftime(time_stamp, sizeof(time_stamp), "%d/%m/%y %H:%M:%S", t);
    strncpy(panel_avg.time_stamp, time_stamp, sizeof(panel_avg.time_stamp));
    panel_avg.time_stamp[sizeof(panel_avg.time_stamp) - 1] = '\0';
}

///////////////////////////////////////////////////////////////////////////////
// transmission_buffer: Buffer averaged data for transmission
///////////////////////////////////////////////////////////////////////////////
void transmission_buffer()
{
    static float avg_Irradiance = 0.0F;
    static uint8_t avg_count = 0;
    static char last_time_stamp[20] = {0};
    avg_count++;
    Serial.println("Average count for LoRa: " + String(avg_count));

    if ((!buffer_ready && avg_count > AVG_COUNT_THRESHOLD) || (buffer_ready && avg_count > NO_REQUEST_THRESHOLD))
    {
        avg_Irradiance = 0.0F;
        avg_count = 1;
        memset(last_time_stamp, 0, sizeof(last_time_stamp));
        memset(tx_buffer, 0, sizeof(tx_buffer));
        buffer_ready = false;
    }

    if (!buffer_ready)
    {
        avg_Irradiance += (panel_avg.Irradiance - avg_Irradiance) / avg_count; // Cumulative average
        strncpy(last_time_stamp, panel_avg.time_stamp, sizeof(last_time_stamp) - 1);
        last_time_stamp[sizeof(last_time_stamp) - 1] = '\0';

        if (avg_count >= AVG_COUNT_THRESHOLD)
        {
            snprintf(tx_buffer, BUFFER_SIZE, "%s|%s|%.2f", "NODE1", last_time_stamp, avg_Irradiance);
            buffer_ready = true;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// thinkspeak_url_5min_calibration: Prepare URL for ThingSpeak upload every 5 minutes or in calibration mode
///////////////////////////////////////////////////////////////////////////////
void thinkspeak_url_5min_cal()
{
    static float panels_avg_irr[6] = {0};
    static float panel_avg_irr = 0.0F;
    static uint8_t average_count = 0;
    static int Threshold;
    average_count++;

    Threshold = calibration_mode ? AVG_COUNT_THINKSPEAK_THRESHOLD_CALIBRATION : AVG_COUNT_THRESHOLD;
    if (average_count > Threshold)
    {
        memset(panels_avg_irr, 0, sizeof(panels_avg_irr));
        panel_avg_irr = 0.0F;
        average_count = 1;
    }

    Serial.println("Average count for Thinkspeak: " + String(average_count));
    int i = 0;
    for (SOLAR_CELL_LIST_PTR ptr = panel_list; ptr != NULL; ptr = ptr->next, i++)
    {
        panels_avg_irr[i] += (ptr->panel->Irradiance - panels_avg_irr[i]) / average_count; // Cumulative average for each panel
    }
    panel_avg_irr += (panel_avg.Irradiance - panel_avg_irr) / average_count; // Cumulative average for all panels
    Serial.printf("Average irradiance of all panels for Thinkspeak: %.2f W/m²\n", panel_avg_irr);

    if (average_count >= Threshold)
    {
        if (calibration_mode)
            url = String(thingspeakServer) + "?api_key=" + String(writeAPIKey_calibration);
        else
            url = String(thingspeakServer) + "?api_key=" + String(writeAPIKey_5min);

        for (int i = 0; i < 6; i++)
        {
            url += "&field" + String(i + 1) + "=" + String(panels_avg_irr[i], 2);
        }
        url += "&field7=" + String(panel_avg_irr, 2);

        if (calibration_mode)
        {
            url += "&field8=" + String(spektron_avg_irr, 2);
            cal_buffer_used = true;
        }

        Serial.println("Sending data to ThingSpeak:");
        Serial.println(url);
        upload_to_thinkspeak();
    }
}

///////////////////////////////////////////////////////////////////////////////
// thinkspeak_url_15_sec: Prepare URL for ThingSpeak upload every 15 seconds
///////////////////////////////////////////////////////////////////////////////
void thinkspeak_url_15sec()
{
    int i = 0;
    url = String(thingspeakServer) + "?api_key=" + String(writeAPIKey_15sec);

    for (SOLAR_CELL_LIST_PTR ptr = panel_list; ptr != NULL; ptr = ptr->next, i++)
    {
        url += "&field" + String(i + 1) + "=" + String(ptr->panel->Irradiance, 2);
    }
    url += "&field7=" + String(panel_avg.Irradiance, 2);
    
    Serial.println("Sending data to 15 sec ThingSpeak:");
    Serial.println(url);

    upload_to_thinkspeak();
}

///////////////////////////////////////////////////////////////////////////////
// calibration_average: Buffer averaged data for ThingSpeak upload in calibration mode
///////////////////////////////////////////////////////////////////////////////
void calibration_average()
{
    average_cal_count++;

    if (cal_buffer_used)
    {
        spektron_avg_irr = 0.0F;
        average_cal_count = 1;
        cal_buffer_used = false;
    }
    Serial.println("Average count for calibration Thinkspeak: " + String(average_cal_count));
    ads[0].readADC_SingleEnded(3);
    int16_t Spektron_raw = ads[0].readADC_SingleEnded(3);
    float Spektron_voltage = (float)Spektron_raw * ADS1115_LSB_GAIN_ONE;
    Spektron_reading = Spektron_voltage * (Spektron_irradiance_reference / Spektron_voltage_reference);
    Serial.printf("Spektron Irradiance: %.2f W/m^2\n", Spektron_reading);

    spektron_avg_irr += (Spektron_reading - spektron_avg_irr) / average_cal_count; // Cumulative average for Spektron
    Serial.printf("Average Spektron irradiance for Thinkspeak: %.2f W/m²\n", spektron_avg_irr);
}
