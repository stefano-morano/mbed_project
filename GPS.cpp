#include <cstring>
#include <cstdlib>
#include "mbed.h"
#include "GPS.h"

// Constructor implementation
GPS::GPS(PinName txPin, PinName rxPin, int baudRateGPS) : gpsSerial(txPin, rxPin, baudRateGPS){
    num_satellites = 0;
    latitude = 0.0;
    longitude = 0.0;
    meridian = ' ';
    parallel = ' ';
    altitude = 0.0;
    measurement = ' ';
    memset(gps_time, 0, sizeof(gps_time));
}

void GPS::parse_gps_data(char* nmea_sentence) {
    if (strstr(nmea_sentence, "$GPGGA")) {          // Find GPGGA string
        char* token = strtok(nmea_sentence, ",");   // Split the string by commas
        int fieldIndex = 0;
        while (token != NULL) {
            switch (fieldIndex) {
                case 1: snprintf(gps_time, sizeof(gps_time), "%.2s:%.2s:%.2s", token, token + 2, token + 4); break; // Time
                case 2: latitude = atof(token) / 100; break;         // Latitude
                case 3: parallel = token[0]; break;                 // N/S Indicator
                case 4: longitude = atof(token) / 100; break;       // Longitude
                case 5: meridian = token[0]; break;                 // E/W Indicator
                case 7: num_satellites = atoi(token); break;        // Number of Satellites
                case 9: altitude = atof(token); break;              // Altitude
                case 10: measurement = token[0] + 32; break;        // Measurement Unit (converted to lowercase)
            }
            token = strtok(NULL, ",");
            fieldIndex++;
        }

        // Print parsed GPS data
        sprintf(gps_data, "GPS: #Sats: %d Lat(UTC): %.6f %c Long(UTC): %.6f %c Altitude: %.1f %c GPS time: %s",
            num_satellites, latitude, parallel, longitude, meridian, altitude, measurement, gps_time);
    }
}

// Reads data from the GPS serial buffer
void GPS::read_gps(void) {
    if (gpsSerial.readable()) {
        int bytesRead = gpsSerial.read(buffer, sizeof(buffer)); // Read data into buffer
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            parse_gps_data(buffer);     // Process the buffer
        }
    }
}




