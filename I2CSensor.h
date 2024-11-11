#ifndef I2CSENSORS_H
#define I2CSENSORS_H

#include "mbed.h"
#include <cstdint>

class I2CSensor {
public:
    uint16_t clear, red, green, blue;
    float ax, ay, az, humidity, temperature;
    float ax_min = 200, ax_max = -200, ay_min = 200, ay_max = -200, az_min = 200, az_max = -200;
    int dominant_color, mode;
    float min_temp = 55, max_temp = 0, mean_temp = 0;
    float min_hum = 76, max_hum = 0, mean_hum = 0;
    int count = 0;
    char accData[128];
    char RGBData[128];
    char tempData[128];
    int counter_dominant[3];
    
    I2CSensor(PinName sda, PinName scl);    
    void reset_values();
    void read_i2c();
    void changeLED(int mode);

private:
    I2C i2c;

    bool readAccRegs(int addr, uint8_t *data, int len);
    float getAccAxis(uint8_t addr);
    void writeColorRegister(uint8_t reg, uint8_t value);
    
    //RGB sensor functions
    int readColorRegister(uint8_t reg);
    int dominant(int red, int green, int blue);
    float readHumidity();
    float readTemperature();

    // Color sensor functions
    void initializeColorSensor();
    void readColorData(uint16_t &clear, uint16_t &red, uint16_t &green, uint16_t &blue);

     // Accelerometer functions
    bool initializeAccelerometer();

};

#endif // I2CSENSORS_H
