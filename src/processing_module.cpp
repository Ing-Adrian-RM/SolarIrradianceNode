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
// Isc_to_irradiance: Convert Isc to irradiance using temperature compensation
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
        avg_count = 0;
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
            snprintf(tx_buffer, BUFFER_SIZE, "NODE1|%s|%.2f", last_time_stamp, avg_Irradiance);
            buffer_ready = true;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// thinkspeak_buffer: Buffer averaged data for ThingSpeak upload
///////////////////////////////////////////////////////////////////////////////
void thinkspeak_url()
{
    static float panels_avg_irr[6] = {0};
    static float panel_avg_irr = 0.0F;
    static uint8_t average_count = 0;
    average_count++;
    Serial.println("Average count for Thinkspeak: " + String(average_count));

    if ((!upload_ready && average_count > AVG_COUNT_THRESHOLD))
    {
        memset(panels_avg_irr, 0, sizeof(panels_avg_irr));
        panel_avg_irr = 0.0F;
        average_count = 0;
        url = "";
        upload_ready = false;
    }

    if (!upload_ready)
    {
        int i = 0;
        for (SOLAR_CELL_LIST_PTR ptr = panel_list; ptr != NULL; ptr = ptr->next, i++)
        {
            panels_avg_irr[i] += (ptr->panel->Irradiance - panels_avg_irr[i]) / average_count; // Cumulative average for each panel
        }
        panel_avg_irr += (panel_avg.Irradiance - panel_avg_irr) / average_count; // Cumulative average for all panels
        if (average_count >= AVG_COUNT_THRESHOLD)
        {
            url = String(thingspeakServer) + "?api_key=" + String(writeAPIKey);
            for (int i = 0; i < 6; i++)
            {
                url += "&field" + String(i + 1) + "=" + String(panels_avg_irr[i], 2);
            }
            url += "&field7=" + String(panel_avg_irr, 2);

            Serial.println("Sending data to ThingSpeak:");
            Serial.println(url);
            upload_ready = true;
            upload_to_thinkspeak();
        }
    }
}
