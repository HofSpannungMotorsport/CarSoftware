#ifndef PINS_MASTER_H
#define PINS_MASTER_H


#define MASTER_BUZZER_OUT                       PA_11
#define MASTER_BRAKELIGHT_OUT                   PB_14
#define MASTER_PUMP_PWM_OUT                     PB_2
#define MASTER_PUMP_PWM_IN                      PC_9
#define MASTER_FANS_PWM_OUT                     PB_13

#define MASTER_CAN_TX                           PB_6
#define MASTER_CAN_RX                           PB_12
#define MASTER_INVERTER_CAN_TX                  PB_9
#define MASTER_INVERTER_CAN_RX                  PB_8

#define MASTER_DOUT1_IN                         PB_1
#define MASTER_DOUT2_IN                         PC_4
#define MASTER_RUN_OUT                          PC_6
#define MASTER_RFE_OUT                          PC_8


#define MASTER_SHUTDOWN_BSPD_MONITORING         PA_0
#define MASTER_SHUTDOWN_HVD_MONITORING          PC_10
#define MASTER_SHUTDOWN_MAINHOOP_MONITORING     PC_12
#define MASTER_SHUTDOWN_ACCU_MONITORING         PC_14
#define MASTER_SHUTDOWN_TSMS_MONITORING         PD_2
#define MASTER_SHUTDOWN_INVERTER_MONITORING     PC_7

#define MASTER_BRAKE_PRESSURE_SENSOR            PA_1

#define MASTER_RPM_SENSOR_REAR_RIGHT            PA_15
#define MASTER_RPM_SENSOR_REAR_LEFT             PB_7

#define MASTER_BSPD_TEST_OUT                    PC_11



#endif // PINS_MASTER_H
