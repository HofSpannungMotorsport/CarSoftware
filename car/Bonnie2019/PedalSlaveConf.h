#ifndef PEDALSLAVECONF_H
#define PEDALSLAVECONF_H

//#define SYNC_DEBUG
//#define CCAN_DEBUG
#include "carpi.h"

#ifdef NEW_SMALL
    #include "hardware/Pins_Pedal_NEW_PCB.h"
#else
    #include "hardware/Pins_Pedal_PCB.h"
#endif

// Communication
Sync syncer(DEVICE_PEDAL);
CCan canIntern(syncer, PEDAL_CAN);

// Pedals
HardwarePedal gasPedal(PEDAL_PIN_ROTATION_ANGLE_GAS_1, PEDAL_PIN_ROTATION_ANGLE_GAS_2, COMPONENT_PEDAL_GAS);
HardwarePedal brakePedal(PEDAL_PIN_ROTATION_ANGLE_BRAKE, COMPONENT_PEDAL_BRAKE);

// RPM Sensor
//HardwareRpmSensor rpmFrontLeft(PEDAL_PIN_RPM_SENSOR_FL, COMPONENT_RPM_FRONT_LEFT);
//HardwareRpmSensor rpmFrontRight(PEDAL_PIN_RPM_SENSOR_FR, COMPONENT_RPM_FRONT_RIGHT);

// Alive
HardwareAlive alive(COMPONENT_ALIVE_PEDAL, LED2);

class Pedal : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            syncer.addComponent((ICommunication&)gasPedal, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)brakePedal, canIntern, DEVICE_MASTER);
            //syncer.addComponent((ICommunication&)rpmFrontLeft, canIntern, DEVICE_MASTER);
            //syncer.addComponent((ICommunication&)rpmFrontRight, canIntern, DEVICE_MASTER);
            syncer.addComponent((ICommunication&)alive, canIntern, DEVICE_MASTER);

            wait(0.1);

            // Attach the Syncer to all components
            gasPedal.attach(syncer);
            brakePedal.attach(syncer);
            //rpmFrontLeft.attach(syncer);
            //rpmFrontRight.attach(syncer);
            alive.attach(syncer);

            alive.setAlive(true);
        }
    
        // Called repeately after bootup
        void loop() {
            wait(LOOP_WAIT_TIME);
        }
};

Pedal runtime;

#endif