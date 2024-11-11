#include "I2CSensor.h"
#include "mbed.h"

// Accelerometer address
#define ACC_I2C_ADDRESS 0x1D << 1
#define ACC_CTRL_REG 0x2A
#define REG_OUT_X_MSB 0x01
#define REG_OUT_Y_MSB 0x03
#define REG_OUT_Z_MSB 0x05
#define UINT14_MAX 16383

// Color sensor definitions
#define TCS34725_ADDRESS 0x29 << 1
#define TCS34725_ENABLE 0x00
#define TCS34725_ENABLE_PON 0x01
#define TCS34725_ENABLE_AEN 0x02
#define TCS34725_ATIME 0x01
#define TCS34725_CONTROL 0x0F
#define TCS34725_COMMAND_BIT 0x80
#define TCS34725_CDATAL 0x14
#define TCS34725_RDATAL 0x16
#define TCS34725_GDATAL 0x18
#define TCS34725_BDATAL 0x1A
DigitalOut TCS34725_LED(D10);

// Temperature and Humidity sensor definitions
#define SI7021_ADDRESS 0x40 << 1
#define CMD_MEASURE_HUMIDITY 0xF5
#define CMD_MEASURE_TEMPERATURE 0xF3

I2CSensor::I2CSensor(PinName sda, PinName scl) : i2c(sda, scl) {}

// --- Accelerometer Functions ---
bool I2CSensor::initializeAccelerometer() {
    uint8_t data[2] = {ACC_CTRL_REG, 0x01};
    return i2c.write(ACC_I2C_ADDRESS, (char *)data, 2) == 0;
}

bool I2CSensor::readAccRegs(int addr, uint8_t *data, int len) {
    char t[1] = {(char) addr};
    if (i2c.write(ACC_I2C_ADDRESS, t, 1, true) != 0)
        return false;
    return i2c.read(ACC_I2C_ADDRESS, (char *) data, len) == 0;
}

float I2CSensor::getAccAxis(uint8_t addr) {
    int16_t axisValue;
    uint8_t res[2];
    if (!readAccRegs(addr, res, 2))
        printf("Error reading accelerometer register %d\n", addr);
    axisValue = (res[0] << 6) | (res[1] >> 2);
    if (axisValue > UINT14_MAX/2)
        axisValue -= UINT14_MAX;
    return float(axisValue) / 4096.0;
}

void I2CSensor::changeLED(int mode) {
    TCS34725_LED = mode;

}

// --- Color Sensor Functions ---
void I2CSensor::initializeColorSensor() {
    writeColorRegister(TCS34725_ENABLE, TCS34725_ENABLE_PON);
    ThisThread::sleep_for(3ms);
    writeColorRegister(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
    writeColorRegister(TCS34725_ATIME, 0xFF);
    writeColorRegister(TCS34725_CONTROL, 0x02);
}

void I2CSensor::writeColorRegister(uint8_t reg, uint8_t value) {
    char data[2] = {(char)(TCS34725_COMMAND_BIT | reg), (char)value}; 
    i2c.write(TCS34725_ADDRESS, data, 2);
}

int I2CSensor::readColorRegister(uint8_t reg) {
    char cmd = (TCS34725_COMMAND_BIT | reg);
    char data[2];
    i2c.write(TCS34725_ADDRESS, &cmd, 1);
    i2c.read(TCS34725_ADDRESS, data, 2);
    return (data[1] << 8) | data[0];
}

void I2CSensor::readColorData(uint16_t &clear, uint16_t &red, uint16_t &green, uint16_t &blue, float current_light_value) {
    bool led_on;
    if (current_light_value < 0.16) {
        led_on = true;
        changeLED(1);
        ThisThread.sleep(20ms);
    } else
        changeLED(0);
    clear = readColorRegister(TCS34725_CDATAL);
    red = readColorRegister(TCS34725_RDATAL);
    green = readColorRegister(TCS34725_GDATAL);
    blue = readColorRegister(TCS34725_BDATAL);
    if (led_on) {
        led_on = false;
        changeLED(0);
    }
}

// --- Temperature and Humidity Sensor Functions ---
float I2CSensor::readHumidity() {
    char cmd[1] = { CMD_MEASURE_HUMIDITY };
    char data[2] = { 0 };
    i2c.write(SI7021_ADDRESS, cmd, 1);
    ThisThread::sleep_for(20ms);
    i2c.read(SI7021_ADDRESS, data, 2);
    int humidity_raw = (data[0] << 8) | data[1];
    return ((125.0 * humidity_raw) / 65536) - 6.0;
}

float I2CSensor::readTemperature() {
    char cmd[1] = { CMD_MEASURE_TEMPERATURE };
    char data[2] = { 0 };
    i2c.write(SI7021_ADDRESS, cmd, 1);
    ThisThread::sleep_for(20ms);
    i2c.read(SI7021_ADDRESS, data, 2);
    int temperature_raw = (data[0] << 8) | data[1];
    return ((175.72 * temperature_raw) / 65536) - 46.85;
}

int I2CSensor::dominant(int red, int green, int blue){
    int max = red, color = 1;
    if (green > max) {
        max = green;
        color = 2;
    }
    if (blue > max) {
        max = blue;
        color = 3;
    }
    if (mode == 2){
        counter_dominant[color-1]++;
    }
    return color;
}

void I2CSensor::reset_values(){
    min_temp = 55; 
    max_temp = 0; 
    mean_temp = 0;
    min_hum = 76;
    max_hum = 0;
    mean_hum = 0;
    count = 0;
    for (int x = 0; x<3 ; x++)
        counter_dominant[x] = 0;
    ax_min = 200;
    ax_max = -200;
    ay_min = 200;
    ay_max = -200;
    az_min = 200;
    az_max = -200;
}

void I2CSensor::read_i2c() {
    
    while (true) {
        initializeAccelerometer();
        //Read accelerometer values
        ax = getAccAxis(REG_OUT_X_MSB);
        ay = getAccAxis(REG_OUT_Y_MSB);
        az = getAccAxis(REG_OUT_Z_MSB);
        sprintf(accData, "ACCELLEROMETERS: X_axis = %.2f m/s², Y_axis = %.2f m/s², Z_axis = %.2f m/s²", ax * 9.8, ay * 9.8, az * 9.8);

        // Read color sensor values
        initializeColorSensor();
        float current_light_value;
        light_mutex.lock();
        current_light_value = light_value;
        light_mutex.unlock();
        readColorData(clear, red, green, blue, current_light_value);
        sprintf(RGBData, "COLOR SENSOR: Clear = %d, Red = %d, Green = %d, Blue = %d -- Dominant color: ", clear, red, green, blue);
        dominant_color = dominant(red, green, blue);

        // Read temperature and humidity
        humidity = readHumidity();
        temperature = readTemperature();
        sprintf(tempData, "TEMP/HUM: Temperature = %.1f°C, \t Relative Humidity = %.1f%%\n", temperature, humidity);

        if (mode == 2){
            if (humidity < min_hum)
                min_hum = humidity;
            if (humidity > max_hum)
                max_hum = humidity;
            if (temperature < min_temp)
                min_temp = temperature;
            if (temperature > max_temp)
                max_temp = temperature;
            if (ax < ax_min)
                ax_min = ax;
            if (ax > ax_max)
                ax_max = ax;
            if (ay < ay_min)
                ay_min = ay;
            if (ay > ay_max)
                ay_max = ay;
            if (az < az_min)
                az_min = az;
            if (az > az_max)
                az_max = az;
            count++;
            mean_temp = (mean_temp * (count - 1) + temperature) / count;
            mean_hum = (mean_hum * (count - 1) + humidity) / count;
        }

        if (mode == 1)
            ThisThread::sleep_for(2s);
        else if (mode == 2)
            ThisThread::sleep_for(30s);
    }
}
