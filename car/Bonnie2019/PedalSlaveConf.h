#ifndef PEDALSLAVECONF_H
#define PEDALSLAVECONF_H

//#define SYNC_DEBUG
//#define CCAN_DEBUG
#include "carpi.h"

#include "hardware/Pins_Pedal_PCB.h"

// Communication
Sync syncer(DEVICE_PEDAL);
CCan canIntern(syncer, PEDAL_CAN);

// Pedals
HardwarePedal gasPedal(PEDAL_PIN_ROTATION_ANGLE_GAS_1, PEDAL_PIN_ROTATION_ANGLE_GAS_2, COMPONENT_PEDAL_GAS);
HardwarePedal brakePedal(PEDAL_PIN_ROTATION_ANGLE_BRAKE, COMPONENT_PEDAL_BRAKE);

// RPM Sensor
//HardwareRpmSensor rpmFrontLeft(PEDAL_PIN_RPM_SENSOR_FL, COMPONENT_RPM_FRONT_LEFT);
//HardwareRpmSensor rpmFrontRight(PEDAL_PIN_RPM_SENSOR_FR, COMPONENT_RPM_FRONT_RIGHT);

class Pedal : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            wait(0.2);

            syncer.addComponent((ICommunication&)gasPedal, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)brakePedal, canIntern, DEVICE_MASTER);
            //syncer.addComponent((ICommunication&)rpmFrontLeft, canIntern, DEVICE_MASTER);
            //syncer.addComponent((ICommunication&)rpmFrontRight, canIntern, DEVICE_MASTER);

            // Attach the Syncer to all components
            gasPedal.attach(syncer);
            brakePedal.attach(syncer);
            //rpmFrontLeft.attach(syncer);
            //rpmFrontRight.attach(syncer);

            wait(0.1);
        }
    
        // Called repeately after bootup
        void loop() {
            wait(0.0001);
        }
};

Pedal runtime;

#endif