///////////////////////////////////////////////////////////////////////////////
// Storage Module
// Description: This module handles SD card storage.s
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#include "config.h"

///////////////////////////////////////////////////////////////////////////////
// initialize_sd: Initialize SD card
///////////////////////////////////////////////////////////////////////////////
void initialize_sd()
{
    hspi = new SPIClass(HSPI);
    hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS);
    if (!SD.begin(HSPI_SS, *hspi))
    {
        Serial.println("Error: Failed to initialize SD card");
    }
    else
    {
        Serial.println("SD card initialized successfully");
    }
}

///////////////////////////////////////////////////////////////////////////////
// getWeekNumber: Get the week number of the year from a tm struct
///////////////////////////////////////////////////////////////////////////////
int getWeekNumber(struct tm *timeinfo)
{
    int yday = timeinfo->tm_yday;
    int wday = (timeinfo->tm_wday + 6) % 7; // make Monday=0
    return (yday - wday + 7) / 7;
}

///////////////////////////////////////////////////////////////////////////////
// save_data_to_csv: Save sensor data to CSV files on SD card
///////////////////////////////////////////////////////////////////////////////
void save_data_to_csv()
{
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    int weekNumber = getWeekNumber(timeinfo);
    int year = timeinfo->tm_year + 1900;
    if (year < 2020)
    {
        Serial.println("Error: Invalid time, cannot save data");
        return;
    }
    char filename[32];
    snprintf(filename, sizeof(filename), "/W%02d_%d_irradiance.csv", weekNumber, year);
    if (!SD.exists(filename))
    {
        File file = SD.open(filename, FILE_WRITE);
        if (!file)
        {
            Serial.println("Error: Unable to create weekly CSV file");
            return;
        }
        file.println("Timestamp,"
                     "P1_Isc,P1_Irr,P1_T,"
                     "P2_Isc,P2_Irr,P2_T,"
                     "P3_Isc,P3_Irr,P3_T,"
                     "P4_Isc,P4_Irr,P4_T,"
                     "P5_Isc,P5_Irr,P5_T,"
                     "P6_Isc,P6_Irr,P6_T,"
                     "Pavg_Isc,Pavg_Irr,Pavg_T");
        file.close();
    }
    File file = SD.open(filename, FILE_APPEND);
    if (!file)
    {
        Serial.println("Error: Failed to open CSV file for writing");
        return;
    }
    file.print(panel_avg.time_stamp);
    file.print(",");
    for (SOLAR_CELL_LIST_PTR ptr = panel_list; ptr != NULL; ptr = ptr->next)
    {
        file.printf("%.2f,%.2f,%.2f,",
                    ptr->panel->Isc,
                    ptr->panel->Irradiance,
                    ptr->panel->Temperature);
    }
    file.printf("%.2f,%.2f,%.2f\n",
                panel_avg.Isc,
                panel_avg.Irradiance,
                panel_avg.Temperature);
    file.close();
    Serial.print("Data appended to ");
    Serial.println(filename);
}
