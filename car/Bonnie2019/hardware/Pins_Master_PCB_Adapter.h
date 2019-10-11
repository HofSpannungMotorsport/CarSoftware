#ifndef PINS_MASTER_H
#define PINS_MASTER_H

// AnalogIn Spring Travel Sensor
#define MASTER_PIN_SPRING_TRAVEL_SENSOR_HL  PB_0
#define MASTER_PIN_SPRING_TRAVEL_SENSOR_HR  PA_1

// Dout Buzzer
#define MASTER_PIN_BUZZER                   PA_4

// Dout Enable Pins for Motor Controller
#define MASTER_PIN_RFE_ENABLE               PD_2
#define MASTER_PIN_RUN_ENABLE               PC_3

// Dout Brake Light
#define MASTER_PIN_BRAKE_LIGHT              PB_7

// Dout Pump enable
#define MASTER_PIN_PUMP_ENABLE              PC_2

// PWMOut Pump
#define MASTER_PIN_PUMP_PWM                 PA_0

// Dout Cooling Fans
#define MASTER_PIN_FAN                      PA_6

// Din HV Enable
#define MASTER_PIN_TSMS                     PH_1
#define MASTER_PIN_AIR                      PH_0
#define MASTER_PIN_60V_OK                   PA_15

// CAN Car intern
#define MASTER_PIN_CAR_INTERN_CAN_RD        PB_5
#define MASTER_PIN_CAR_INTERN_CAN_TD        PB_6

// CAN Motor Controller
#define MASTER_PIN_MOTOR_CONTROLLER_CAN_RD  PB_8
#define MASTER_PIN_MOTOR_CONTROLLER_CAN_TD  PB_9

// Dout LED HV Enabled
#define MASTER_PIN_LED_HV_ENABLED           PC_10 // Not Connected

// Dout LED Error
#define MASTER_PIN_LED_ERROR                PC_11 // Not Connected

// DOUT LED Extra
#define MASTER_PIN_LED_ALIVE                LED1 // PA_5 -> GREEN

// Din User Button (Normally closed)
#define MASTER_PIN_USER_BUTTON              USER_BUTTON // PC_13

// SPI SD Card
#define MASTER_PIN_SPI_SD_CS   PB_12
#define MASTER_PIN_SPI_SD_SCK  PB_13
#define MASTER_PIN_SPI_SD_MOSI PB_15
#define MASTER_PIN_SPI_SD_MISO PB_14

// Dout Microcontroller OK
#define MASTER_PIN_MICROCONTROLLER_OK       PA_8

// Din Error Pins
//#define MASTER_PIN_ERROR_BSPD               
//#define MASTER_PIN_ERROR_BMS                
//#define MASTER_PIN_ERROR_IMD                

#endif // PINS_MASTER_H