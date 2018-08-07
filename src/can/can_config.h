#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#ifdef STM32F767xx
#define CAN1_CONF PB_8,PB_9
#define CAN2_CONF PB_12,PB_13
#define CAN3_CONF PB_3,PB_4
#endif

#ifdef STM32F446xx
#define CAN1_CONF PB_8,PB_9
#define CAN2_CONF PB_5,PB_6
#endif

#endif