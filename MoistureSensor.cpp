#include "mbed.h"
#include "MoistureSensor.h"

MoistureSensor::MoistureSensor(PinName analogPin) : sensor(analogPin){
    value = 0;
}

void MoistureSensor::reset_values(){
    min_soil = 101;
    max_soil = 0;
    mean_soil = 0;
    count = 0;
}


void MoistureSensor::read_moisture_sensor(){
    value = sensor.read()*100;        //read value from the sensor
    sprintf(moisture_sensor_data, "SOIL MOSTURE: %.2f%%", value);

    if (mode == 2){
        if (value < min_soil)
            min_soil = value;
        if (value > max_soil)
            max_soil = value;
        count++;
        mean_soil = (mean_soil * (count - 1) + value) / count;
    }

        
}