#include "mbed.h"
#include "LightSensor.h"

LightSensor::LightSensor(PinName analogPin) : sensor(analogPin){
    value = 0;
}

void LightSensor::reset_values(){
    min_light = 101;
    max_light = 0;
    mean_light = 0;
    count = 0;
}

void LightSensor::read_light_sensor(){
    value = sensor.read()*100;        //read value from the sensor
    light_mutex.lock();
    light_value = value;
    light_mutex.unlock();
    
    sprintf(light_sensor_data, "LIGHT: %.2f%%", value);

    if (mode == 2){
        if (value < min_light)
            min_light = value;
        if (value > max_light)
            max_light = value;
        count++;
        mean_light = (mean_light * (count - 1) + value) / count;
    }
}
