#ifndef BONNIE_2019_PEDAL_CONF_H
#define BONNIE_2019_PEDAL_CONF_H

#include "HardwarePedal.h"
#include "HardwareAnalogSensor.h"
#include "mbed.h"

#include "../../can/can_config.h"

CAN can(CAN1_CONF);

HardwarePedal gasPedal(2,{HardwareAnalogSensor(A3),HardwareAnalogSensor(A4)});
HardwarePedal brakePedal(1,{HardwareAnalogSensor(A5)});

HardwareAnalogSensor sts_fl(A1);
HardwareAnalogSensor sts_fr(A2);

void initBoardHardware()
{
    // assign the components to CANService here
    
}

#endif