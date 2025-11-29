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
    // Get current time for filename
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    int weekNumber = getWeekNumber(timeinfo);
    int year = timeinfo->tm_year + 1900;
    if (year < 2020)
    {
        Serial.println("Error: Invalid time, be aware of NTP configuration and possible invalid timestamp.");
        configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // Configure NTP; -6 for CST (Costa Rica)
    }

    // Create filename based on week number, year, and calibration mode
    char filename[32];
    if (calibration_mode)
    {
        // Create file if it doesn't exist and write header
        snprintf(filename, sizeof(filename), "/W%02d_%d_calibration.csv", weekNumber, year);
        if (!SD.exists(filename))
        {
            File file = SD.open(filename, FILE_WRITE);
            if (!file)
            {
                Serial.println("Error: Unable to create weekly CSV calibration file");
                return;
            }
            file.println("Timestamp,"
                         "P1 [W/m^2],P2 [W/m^2],P3 [W/m^2],P4_ [W/m^2],P5 [W/m^2],P6 [W/m^2],Pavg [W/m^2],Spektron [W/m^2]");
            file.close();
        }

        // Append data to calibration file
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
            file.printf("%.2f", ptr->panel->Irradiance);
        }
        file.printf("%.2f", panel_avg.Irradiance);
        file.printf("%.2f\n", Spektron_reading);
        file.close();
    }
    else
    {
        // Create file if it doesn't exist and write header
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
                         "P1_Isc [mA],P1_Irr [W/m^2],P1_T [ºC],"
                         "P2_Isc [mA],P2_Irr [W/m^2],P2_T [ºC],"
                         "P3_Isc [mA],P3_Irr [W/m^2],P3_T [ºC],"
                         "P4_Isc [mA],P4_Irr [W/m^2],P4_T [ºC],"
                         "P5_Isc [mA],P5_Irr [W/m^2],P5_T [ºC],"
                         "P6_Isc [mA],P6_Irr [W/m^2],P6_T [ºC],"
                         "Pavg_Isc [mA],Pavg_Irr [W/m^2],Pavg_T [ºC]");
            file.close();
        }

        // Append data to regular file
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
    }
    Serial.print("Data appended to ");
    Serial.println(filename);
}
