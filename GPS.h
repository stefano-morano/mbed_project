// GPS.h
#ifndef GPS_H
#define GPS_H

#include "mbed.h"
#include "rtos.h"

class GPS {
public:
 
    BufferedSerial gpsSerial;

    int num_satellites;
    float latitude;
    float longitude;
    char meridian;
    char parallel;
    float altitude;
    char measurement;
    char gps_time[10];
    
    char gps_data[256];
    char buffer[256];

    // Constructor
    GPS(PinName txPin, PinName rxPin, int baudRateGPS);
    void read_gps(void);

private: 
    void parse_gps_data(char* nmea_sentence);
};

#endif // GPS_H
