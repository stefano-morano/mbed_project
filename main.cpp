#include "GPSLightMoistureSensors.h"
#include "I2CSensor.h"
#include "mbed.h"
#define TEST 1      //0,0,1
#define NORMAL 2    //0,1,0
#define ADVANCED 4  //1,0,0
#define WHITE 0     //0,0,0
#define YELLOW 1    //0,0,1
#define PURPLE 2    //0,1,0
#define RED 3       //0,1,1
#define CYAN 4      //1,0,0
#define GREEN 5     //1,0,1
#define BLUE 6      //1,1,0
#define OFF 7       //1,1,1

#define GPS_TX_PIN PA_9
#define GPS_RX_PIN PA_10
#define LIGHT_PIN PA_0
#define MOISTURE_PIN PA_4
#define I2C_SDA PB_9
#define I2C_SCL PB_8


InterruptIn button(PB_2);
BusOut led_board(LED1, LED2, LED3);
BusOut led_rgb(PB_13, PB_14, PB_15);
BufferedSerial pc(USBTX, USBRX, 115200);

float light_value = 0;
Mutex light_mutex;

GPSLightMoistureSensors gpsLightMoisture(GPS_TX_PIN, GPS_RX_PIN, 9600, LIGHT_PIN, MOISTURE_PIN, light_value, light_mutex);
I2CSensor I2CSensor(I2C_SDA, I2C_SCL, light_value, light_mutex);

Thread i2cThread(osPriorityNormal, 1024), gpsLightMoistureThread(osPriorityNormal, 1024);
Ticker ticker, summarize_ticker;

bool button_pressed = false, print_test = false, print_summarize = false;
int mode, max_dominant;

// inline void Sleep(){
//     FLASH->ACR &= ~FLASH_ACR_SLEEP_PD;
//     SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);
//     SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
//     __WFI();
// }

void onClick(void) { button_pressed = true; }
void print_func(void) { print_test = true; }
void print_summ(void) { print_summarize = true; }

void reset_sensors(){
    gpsLightMoisture.moisture.reset_values();
    gpsLightMoisture.light.reset_values();
    I2CSensor.reset_values();
}

void change_mode(int mode){
    gpsLightMoisture.change_mode(mode);
    I2CSensor.mode = mode;
    ticker.detach();
    summarize_ticker.detach();
    led_board = mode;
    led_rgb = OFF;
}

int calculate_max_dominant(){
    int index = 0, max = I2CSensor.counter_dominant[0];
    if (I2CSensor.counter_dominant[1] > max){
        max = I2CSensor.counter_dominant[1];
        index = 1;
    }
    if (I2CSensor.counter_dominant[2] > max){
        max = I2CSensor.counter_dominant[2];
        index = 2;
    }
    return index;
}


int main(){
    button.mode(PullUp);
    button.fall(onClick);

    mode = TEST;
    led_board = TEST;

    gpsLightMoistureThread.start(callback(&gpsLightMoisture, &GPSLightMoistureSensors::read_sensors_data));
    i2cThread.start(callback(&I2CSensor, &I2CSensor::read_i2c));

    ticker.attach(print_func, 2s);

    printf("TEST MODE\n\n");
    while (true) {
        if (button_pressed) {               
            button_pressed = false;
            mode++;
            if (mode == 4) mode = 1;
            switch(mode){
                case 1: 
                    change_mode(TEST);
                    printf("TEST MODE\n\n");
                    ticker.attach(print_func, 2s);
                    break;
                case 2: 
                    change_mode(NORMAL);
                    printf("NORMAL MODE\n\n");
                    reset_sensors();
                    ticker.attach(print_func, 30s);
                    summarize_ticker.attach(print_summ, 120s);
                    break;
                case 3: 
                    change_mode(ADVANCED);
                    printf("ADVANCED MODE\n\n");
                    break;
            }
        }

        if (print_test){
            print_test = false;
            printf("%s\n", gpsLightMoisture.moisture.moisture_sensor_data);
            printf("%s\n", gpsLightMoisture.light.light_sensor_data);
            printf("%s\n", gpsLightMoisture.gps.gps_data);
            printf("%s", I2CSensor.RGBData);
            switch (I2CSensor.dominant_color){
                case 1: 
                    printf("RED\n");
                    (mode == TEST) ? led_rgb = RED : led_rgb = OFF;
                    break;
                case 2: 
                    printf("GREEN\n");
                    (mode == TEST) ? led_rgb = GREEN : led_rgb = OFF;
                    break;
                case 3: 
                    printf("BLUE\n");
                    (mode == TEST) ? led_rgb = BLUE : led_rgb = OFF;
                    break;
            }
            printf("%s\n", I2CSensor.accData);
            printf("%s\n", I2CSensor.tempData);
            printf("\n");
            if (mode == 2){                         //check test
                if (I2CSensor.temperature < -10 || I2CSensor.temperature > 50)
                    led_rgb = RED;
                if (I2CSensor.humidity < 25 || I2CSensor.humidity > 75)
                    led_rgb = WHITE;
                if (gpsLightMoisture.light.value < 0 || gpsLightMoisture.light.value > 100)
                    led_rgb = CYAN;
                if (gpsLightMoisture.moisture.value < 0 || gpsLightMoisture.moisture.value > 100)
                    led_rgb = YELLOW;
                // color, acceleration? 
            }
        }

        if (print_summarize){
            print_summarize = false;
            printf("\n\nVALUE SUMMARIZE\n\n");
            printf("LIGHT SENSOR: min = %.1f%%, max = %.1f%%, mean = %.1f%%\n", gpsLightMoisture.light.min_light, gpsLightMoisture.light.max_light, gpsLightMoisture.light.mean_light);
            printf("SOIL AND MOISTURE SENSOR: min = %.1f%%, max = %.1f%%, mean = %.1f%%\n", gpsLightMoisture.moisture.min_soil, gpsLightMoisture.moisture.max_soil, gpsLightMoisture.moisture.mean_soil);
            printf("TEMPERATURE SENSOR: min = %.1f°C, max = %.1f°C, mean = %.1f°C\n", I2CSensor.min_temp, I2CSensor.max_temp, I2CSensor.mean_temp);
            printf("HUMIDITY SENSOR: min = %.1f%%, max = %.1f%%, mean = %.1f%%\n", I2CSensor.min_hum, I2CSensor.max_hum, I2CSensor.mean_hum);
            
            max_dominant = calculate_max_dominant();
            printf("MOST READ DOMINANT COLOR: ");
            switch (max_dominant){
                case 0: 
                    printf("RED\n");
                    break;
                case 1: 
                    printf("GREEN\n");
                    break;
                case 2: 
                    printf("BLUE\n");
                    break;    
            }
            printf("ACCELLEROMETER X-AXIS: max = %.2f m/s², min = %.2f m/s²\n", I2CSensor.ax_max, I2CSensor.ax_min);
            printf("ACCELLEROMETER Y-AXIS: max = %.2f m/s², min = %.2f m/s²\n", I2CSensor.ay_max, I2CSensor.ay_min);
            printf("ACCELLEROMETER Z-AXIS: max = %.2f m/s², min = %.2f m/s²\n", I2CSensor.az_max, I2CSensor.az_min);
            printf("\n");
        }
    }
}

