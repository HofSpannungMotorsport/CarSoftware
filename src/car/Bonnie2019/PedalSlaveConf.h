#ifndef PEDALSLAVECONF_H
#define PEDALSLAVECONF_H

#include "mbed.h"

#ifndef MESSAGE_REPORT
    #define MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

#include "../../can/can_config.h"
#include "../../can/CANService.h"
CANService canService(CAN1_CONF);

#include "hardware/Pins_Pedal.h"
#include "../../components/hardware/HardwarePedal.h"
#include "../../can/PedalMessageHandler.h"
#include "../../components/hardware/HardwareRpmSensor.h"
#include "../../can/RpmSensorMessageHandler.h"

#define PEDAL_SEND_RATE 120 // Hz

// Pedals
HardwarePedal gasPedal(PEDAL_PIN_ROTATION_ANGLE_GAS_1, PEDAL_PIN_ROTATION_ANGLE_GAS_2, PEDAL_GAS);
HardwarePedal brakePedal(PEDAL_PIN_ROTATION_ANGLE_BRAKE, PEDAL_BRAKE);
PedalMessageHandler pedalMessageHandler;

// RPM Sensor
HardwareRpmSensor rpmFrontLeft(PEDAL_PIN_RPM_SENSOR_FL, RPM_FRONT_LEFT);
HardwareRpmSensor rpmFrontRight(PEDAL_PIN_RPM_SENSOR_FR, RPM_FRONT_RIGHT);
RpmSensorMessageHandler rpmSensorMessageHandler;

class Pedal {
    public:
        // Called once at bootup
        void setup() {
            canService.addComponent((void*)&gasPedal, (IMessageHandler<CANMessage>*)&pedalMessageHandler, NORMAL);
            canService.addComponent((void*)&brakePedal, (IMessageHandler<CANMessage>*)&pedalMessageHandler, NORMAL);
            canService.addComponentToSendLoop((void*)&gasPedal);
            canService.addComponentToSendLoop((void*)&brakePedal);

            canService.addComponent((void*)&rpmFrontLeft, (IMessageHandler<CANMessage>*)&rpmSensorMessageHandler, NORMAL);
            canService.addComponent((void*)&rpmFrontRight, (IMessageHandler<CANMessage>*)&rpmSensorMessageHandler, NORMAL);
            canService.addComponentToSendLoop((void*)&rpmFrontLeft);
            canService.addComponentToSendLoop((void*)&rpmFrontRight);
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