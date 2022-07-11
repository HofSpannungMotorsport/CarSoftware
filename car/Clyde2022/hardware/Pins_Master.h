#ifndef PINS_MASTER_H
#define PINS_MASTER_H

// Dout Buzzer
#define MASTER_PIN_BUZZER                   PA_8

// Dout Enable Pins for Motor Controller
#define MASTER_PIN_RFE_ENABLE               PC_12
#define MASTER_PIN_RUN_ENABLE               PC_10

// Dout Brake Light
#define MASTER_PIN_BRAKE_LIGHT              PB_10

// PWMOut Pump
#define MASTER_PIN_PUMP_PWM                 PC_7

// Dout Cooling Fans
#define MASTER_PIN_FAN                      PA_9

// Din HV Enable
#define MASTER_PIN_TSMS                     PA_1
#define MASTER_PIN_HV_ALL_READY             PD_2

// Dout TS On Enable
#define MASTER_PIN_TS_ON_ENABLE             PA_3

// Precharge Pins
#define MASTER_PIN_STOP_PRECHARGE_OUT       PB_7
#define MASTER_PIN_INVERTER_DOUT_1          PA_6
#define MASTER_PIN_INVERTER_DOUT_2          PA_7

// CAN Car intern
#define MASTER_PIN_CAR_INTERN_CAN_RD        PB_8
#define MASTER_PIN_CAR_INTERN_CAN_TD        PB_9

// CAN Motor Controller
#define MASTER_PIN_MOTOR_CONTROLLER_CAN_RD  PB_12
#define MASTER_PIN_MOTOR_CONTROLLER_CAN_TD  PB_6

// Din User Button (Normally closed)
#define MASTER_PIN_USER_BUTTON              USER_BUTTON

#endif // PINS_MASTER_H