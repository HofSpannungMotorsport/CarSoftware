#ifndef BONNIE_2019_PEDAL_CONF_H
#define BONNIE_2019_PEDAL_CONF_H

#include "mbed.h"
#include "hardware/Pins_Pedal.h"
#include "../../components/hardware/HardwarePedal.h"
#include "../../components/hardware/HardwareAnalogSensor.h"

HardwarePedal gasPedal(2, {HardwareAnalogSensor(PEDAL_PIN_ROTATION_ANGLE_GAS_1),HardwareAnalogSensor(PEDAL_PIN_ROTATION_ANGLE_GAS_2)});
HardwarePedal brakePedal(1, {HardwareAnalogSensor(PEDAL_PIN_ROTATION_ANGLE_BRAKE)});

HardwareAnalogSensor springTravelSensorFL(PEDAL_PIN_SPRING_TRAVEL_SENSOR_FL);
HardwareAnalogSensor springTravelSensorFR(PEDAL_PIN_SPRING_TRAVEL_SENSOR_FR);

void initBoardHardware() {
    // assign the components to CANService here
    
}

#endif