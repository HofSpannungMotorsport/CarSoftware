#ifndef BONNIE_2019_PEDAL_CONF_H
#define BONNIE_2019_PEDAL_CONF_H

#include "mbed.h"
#include "hardware/Pins_Pedal.h"
#include "../../components/hardware/HardwarePedal.h"
#include "../../can/PedalMessageHandler.h"

#define PEDAL_SEND_RATE 60 // Hz

HardwarePedal gasPedal(PEDAL_PIN_ROTATION_ANGLE_GAS_1, PEDAL_PIN_ROTATION_ANGLE_GAS_2, PEDAL_GAS);
HardwarePedal brakePedal(PEDAL_PIN_ROTATION_ANGLE_BRAKE, PEDAL_BRAKE);
PedalMessageHandler pedalMessageHandler;

class Pedal {
    public:
        // Called once at bootup
        void setup() {
            canService.addComponent((void*)&gasPedal, (IMessageHandler<CANMessage>*)&pedalMessageHandler, NORMAL);
            canService.addComponent((void*)&brakePedal, (IMessageHandler<CANMessage>*)&pedalMessageHandler, NORMAL);
            canService.addComponentToSendLoop((void*)&gasPedal);
            canService.addComponentToSendLoop((void*)&brakePedal);
        }
    
        // Called repeately after bootup
        void loop() {
            Timer refreshTimer;
            refreshTimer.reset();
            refreshTimer.start();
        
            canService.run();

            while(refreshTimer.read() < (1 / PEDAL_SEND_RATE));
        }
};

Pedal runtime;

#endif