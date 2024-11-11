// GPS.h
#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H

#include "mbed.h"
#include "rtos.h"

class LightSensor {
public:
    float value;
    float min_light = 101, max_light = 0, mean_light = 0;
    int count = 0, mode;
    char light_sensor_data[256];
    AnalogIn sensor;

    LightSensor(PinName analogPin);
    void read_light_sensor();
    void reset_values();
};

#endif // LIGHTSENSOR_H
