#ifndef PINS_MASTER_H
#define PINS_MASTER_H

// AnalogIn Spring Travel Sensor
#define MASTER_PIN_SPRING_TRAVEL_SENSOR_HL  PF_8
#define MASTER_PIN_SPRING_TRAVEL_SENSOR_HR  PC_0

// Dout Buzzer
#define MASTER_PIN_BUZZER                   PG_11

// Dout Enable Pins for Motor Controller
#define MASTER_PIN_RFE_ENABLE               PC_14
#define MASTER_PIN_RUN_ENABLE               PH_0

// Dout Brake Light
#define MASTER_PIN_BRAKE_LIGHT              PD_7

// Dout Pump enable
#define MASTER_PIN_PUMP_ENABLE              PG_9

// PWMOut Pump
#define MASTER_PIN_PUMP_PWM                 PA_0

// Cooling Fans
#define MASTER_PIN_FAN                      PG_2

// HV Enable
#define MASTER_PIN_HV_ENABLED               PE_4

// CAN Car intern
#define MASTER_PIN_CAR_INTERN_CAN_RD        PD_0
#define MASTER_PIN_CAR_INTERN_CAN_TD        PD_1

// CAN Motor Controller
#define MASTER_PIN_MOTOR_CONTROLLER_CAN_RD  PB_12
#define MASTER_PIN_MOTOR_CONTROLLER_CAN_TD  PB_6

// Din RPM Sensors
#define MASTER_PIN_RPM_SENSOR_HL            PA_13
#define MASTER_PIN_RPM_SENSOR_HR            PG_10

// Dout LED HV Enabled
#define MASTER_PIN_LED_HV_ENABLED           LED1 // PB_7

// Dout LED Error
#define MASTER_PIN_LED_ERROR                LED2 // PB_0

// Din User Button (Normally closed)
#define MASTER_PIN_USER_BUTTON              USER_BUTTON // PC_13

#endif // PINS_MASTER_H