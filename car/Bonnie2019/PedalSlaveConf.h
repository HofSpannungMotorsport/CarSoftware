#ifndef PEDALSLAVECONF_H
#define PEDALSLAVECONF_H

#include "carpi.h"

#include "hardware/Pins_Pedal_PCB.h"

CANService canService(PEDAL_CAN);

#define PEDAL_SEND_RATE 120 // Hz

// Pedals
HardwarePedal gasPedal(PEDAL_PIN_ROTATION_ANGLE_GAS_1, PEDAL_PIN_ROTATION_ANGLE_GAS_2, COMPONENT_PEDAL_GAS);
HardwarePedal brakePedal(PEDAL_PIN_ROTATION_ANGLE_BRAKE, COMPONENT_PEDAL_BRAKE);

// RPM Sensor
HardwareRpmSensor rpmFrontLeft(PEDAL_PIN_RPM_SENSOR_FL, COMPONENT_RPM_FRONT_LEFT);
HardwareRpmSensor rpmFrontRight(PEDAL_PIN_RPM_SENSOR_FR, COMPONENT_RPM_FRONT_RIGHT);

class Pedal : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            canService.setSenderId(DEVICE_PEDAL);

            canService.addComponent((ICommunication*)&gasPedal);
            canService.addComponent((ICommunication*)&brakePedal);
            canService.addComponentToSendLoop((ICommunication*)&gasPedal);
            canService.addComponentToSendLoop((ICommunication*)&brakePedal);

            canService.addComponent((ICommunication*)&rpmFrontLeft);
            canService.addComponent((ICommunication*)&rpmFrontRight);
            canService.addComponentToSendLoop((ICommunication*)&rpmFrontLeft);
            canService.addComponentToSendLoop((ICommunication*)&rpmFrontRight);

            wait(0.1);
        }
    
        // Called repeately after bootup
        void loop() {
            Timer refreshTimer;
            refreshTimer.reset();
            refreshTimer.start();
        
            canService.run();

            while(refreshTimer.read() < (1 / (float)PEDAL_SEND_RATE));
        }
};

Pedal runtime;

#endif