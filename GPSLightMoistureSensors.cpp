#include "GPSLightMoistureSensors.h"

GPSLightMoistureSensors::GPSLightMoistureSensors(PinName GPSTxPin, PinName GPSRxPin, int GPSbaudRateGPS, 
                                                PinName ligthAnalogPin,
                                                PinName moistureAnalogPin,
                                                float current_light_value, Mutex mutex) : gps(GPSTxPin, GPSRxPin,GPSbaudRateGPS),
                                                                            light(ligthAnalogPin),
                                                                            moisture(moistureAnalogPin),
                                                                            light_mutex(current_light_value),
                                                                            light_mutex(mutex)
{}

void GPSLightMoistureSensors::read_sensors_data() {
    while (true) {
        gps.read_gps();
        moisture.read_moisture_sensor();
        light.read_light_sensor();
        if (mode == 1)
            ThisThread::sleep_for(2s);
        else if (mode == 2)
            ThisThread::sleep_for(30s);
    } 
}

void GPSLightMoistureSensors::change_mode(int new_mode) {
    mode = new_mode;
    //gps.mode = new_mode;
    moisture.mode = new_mode;
    light.mode = new_mode;
}