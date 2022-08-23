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
//#define MASTER_PIN_TS_ON_ENABLE             PA_3 -> Removed

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

// --------- At this point unused pins
#define MASTER_PIN_RPM_RL                  PH_0
#define MASTER_PIN_RPM_RR                  PH_1

#define MASTER_PIN_SHUTDOWN_PRE_BSPD       PC_2
#define MASTER_PIN_SHUTDOWN_AFTER_BSPD     PC_3
#define MASTER_PIN_SHUTDOWN_AT_TS_ON       PB_3
//#define MASTER_PIN_SHUTDOWN_AT_HVD         PA_2 -> Removed
#define MASTER_PIN_SHUTDOWN_AT_BOTS        PC_5
#define MASTER_PIN_SHUTDOWN_ERROR_STORAGE  PB_15
#define MASTER_PIN_SHUTDOWN_TSMS_IN        PB_14

#define MASTER_PIN_IMD_OK                  PB_2
#define MASTER_PIN_BMS_OK                  PB_1

#define MASTER_PIN_TS_ON_STATE             PA_10

#define MASTER_PIN_TSAL_MC_OUT             PC_11
#define MASTER_PIN_BRAKE_FRONT             PB_0

#define MASTER_PIN_BSPD_TEST               PB_4

#endif // PINS_MASTER_H
