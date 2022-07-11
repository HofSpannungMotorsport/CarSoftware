#ifndef PEDALSLAVECONF_H
#define PEDALSLAVECONF_H

#define EXPERIMENTELL_ASR_ACTIVE

// Automatic
#ifdef EXPERIMENTELL_ASR_ACTIVE
    #define ENABLE_POWER_MENU
#endif

//#define CAN_DEBUG
#include "carpi.h"

#include "hardware/Pins_Pedal.h"


CANService canService(PEDAL_CAN);

#define PEDAL_SEND_RATE 120 // Hz

// Pedals
HardwarePedal gasPedal(PEDAL_PIN_ROTATION_ANGLE_GAS_1, PEDAL_PIN_ROTATION_ANGLE_GAS_2, COMPONENT_PEDAL_GAS, STD_GAS_1_MIN, STD_GAS_1_MAX, STD_GAS_2_MIN, STD_GAS_2_MAX);
HardwarePedal brakePedal(PEDAL_PIN_ROTATION_ANGLE_BRAKE, COMPONENT_PEDAL_BRAKE, STD_BRAKE_MIN, STD_BRAKE_MAX);

// RPM Sensor
#ifdef EXPERIMENTELL_ASR_ACTIVE
    //HardwareRpmSensor rpmFrontLeft(PEDAL_PIN_RPM_SENSOR_FL, COMPONENT_RPM_FRONT_LEFT);
    HardwareRpmSensor rpmFrontRight(PEDAL_PIN_RPM_SENSOR_FR, COMPONENT_RPM_FRONT_RIGHT);
    #warning "Don't forget to activate ASR on Master too!!!"
#endif

class Pedal : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            canService.setSenderId(DEVICE_PEDAL);

            canService.addComponent((ICommunication*)&gasPedal);
            canService.addComponent((ICommunication*)&brakePedal);
            canService.addComponentToSendLoop((ICommunication*)&gasPedal);
            canService.addComponentToSendLoop((ICommunication*)&brakePedal);

            #ifdef EXPERIMENTELL_ASR_ACTIVE
                //canService.addComponent((ICommunication*)&rpmFrontLeft);
                canService.addComponent((ICommunication*)&rpmFrontRight);
                //canService.addComponentToSendLoop((ICommunication*)&rpmFrontLeft);
                canService.addComponentToSendLoop((ICommunication*)&rpmFrontRight);
            #endif

            wait(0.1);
        }
    
        // Called repeately after bootup
        void loop() {
            canService.run();

            wait(1.0/(float)PEDAL_SEND_RATE);
        }
};

Pedal runtime;

#endif