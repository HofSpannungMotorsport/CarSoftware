#ifndef PEDALSLAVECONF_H
#define PEDALSLAVECONF_H

//#define SYNC_DEBUG
//#define CCAN_DEBUG
#include "carpi.h"

#include "hardware/Pins_Pedal_NEW_PCB.h"

// Communication
Sync syncer(DEVICE_PEDAL);
CCan canIntern(syncer, PEDAL_CAN);

// Registry
InternalRegistry registry(COMPONENT_SYSTEM_REGISTRY);

// Pedals
HardwarePedal gasPedal(PEDAL_PIN_ROTATION_ANGLE_GAS_1, PEDAL_PIN_ROTATION_ANGLE_GAS_2, COMPONENT_PEDAL_GAS, registry);
HardwarePedal brakePedal(PEDAL_PIN_ROTATION_ANGLE_BRAKE, COMPONENT_PEDAL_BRAKE, registry);

// RPM Sensor
//HardwareRpmSensor rpmFrontLeft(PEDAL_PIN_RPM_SENSOR_FL, COMPONENT_RPM_FRONT_LEFT);
//HardwareRpmSensor rpmFrontRight(PEDAL_PIN_RPM_SENSOR_FR, COMPONENT_RPM_FRONT_RIGHT);

// Alive
HardwareAlive alive(COMPONENT_ALIVE_PEDAL, PEDAL_PIN_ALIVE, registry);

class Pedal : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            // Pedal Pre-Calibration
            pedal_calibration_data_t gasCalibration(registry.getUInt16(STD_PEDAL_GAS_1_MIN), registry.getUInt16(STD_PEDAL_GAS_1_MAX), registry.getUInt16(STD_PEDAL_GAS_2_MIN), registry.getUInt16(STD_PEDAL_GAS_2_MAX));
            pedal_calibration_data_t brakeCalibration(registry.getUInt16(STD_PEDAL_BRAKE_MIN), registry.getUInt16(STD_PEDAL_BRAKE_MAX));

            gasPedal.setCalibration(gasCalibration);
            brakePedal.setCalibration(brakeCalibration);


            syncer.addComponent(gasPedal, canIntern, DEVICE_MASTER);
            syncer.addComponent(brakePedal, canIntern, DEVICE_MASTER);
            //syncer.addComponent(rpmFrontLeft, canIntern, DEVICE_MASTER);
            //syncer.addComponent(rpmFrontRight, canIntern, DEVICE_MASTER);
            syncer.addComponent(alive, canIntern, DEVICE_MASTER);
            syncer.finalize();

            wait(STARTUP_WAIT_TIME_SLAVE);

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
            syncer.run();
            wait(LOOP_WAIT_TIME);
        }
};

Pedal runtime;

#endif