#ifndef BONNIE_2019_MASTER_CONF_H
#define BONNIE_2019_MASTER_CONF_H

#include "mbed.h"
#include "hardware/Pins_Master.h"
#include "../../components/hardware/HardwareLed.h"
#include "../../components/software/SoftwareLed.h"
#include "../../components/hardware/HardwareAnalogSensor.h"
#include "../../can/can_config.h"

CAN can(CAN1_CONF);

SoftwareLed greenLed;
SoftwareLed yellowLed;
SoftwareLed redLed;

HardwareLed brakeLight(MASTER_PIN_BRAKE_LIGHT); // change pin

HardwareAnalogSensor springTravelSensorHL(MASTER_PIN_SPRING_TRAVEL_SENSOR_HL);
HardwareAnalogSensor springTravelSensorHR(MASTER_PIN_SPRING_TRAVEL_SENSOR_HR);

void initBoardHardware()
{
    // assign the components to CANService here
    
}

#endif