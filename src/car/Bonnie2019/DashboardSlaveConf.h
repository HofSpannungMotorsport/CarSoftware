#ifndef BONNIE_2019_DASHBOARD_CONF_H
#define BONNIE_2019_DASHBOARD_CONF_H

#include "mbed.h"
#include "hardware/Pins_Dashboard_PCB.h"
#include "../../components/hardware/HardwareInterruptButton.h"
#include "../../can/ButtonMessageHandler.h"
#include "../../components/hardware/HardwareLed.h"
#include "../../can/LEDMessageHandler.h"

//LED's
HardwareLed ledRed(DASHBOARD_PIN_LED_RED, LED_ERROR);
HardwareLed ledYellow(DASHBOARD_PIN_LED_YELLOW, LED_ISSUE);
HardwareLed ledGreen(DASHBOARD_PIN_LED_GREEN, LED_READY_TO_DRIVE);
LEDMessageHandler ledMessageHandler;


// Buttons
HardwareInterruptButton buttonReset(DASHBOARD_PIN_BUTTON_RESET, BUTTON_RESET);
HardwareInterruptButton buttonStart(DASHBOARD_PIN_BUTTON_START, BUTTON_START);
ButtonMessageHandler buttonMessageHandler;

class Dashboard {
    public:
        // Called once at bootup
        void setup() {
            canService.addComponent((void*)&ledGreen, (IMessageHandler<CANMessage>*)&ledMessageHandler, NORMAL);
            canService.addComponent((void*)&ledGreen, (IMessageHandler<CANMessage>*)&ledMessageHandler, NORMAL);
            canService.addComponent((void*)&ledGreen, (IMessageHandler<CANMessage>*)&ledMessageHandler, NORMAL);
            canService.addComponent((void*)&buttonReset, (IMessageHandler<CANMessage>*)&buttonMessageHandler, NORMAL);
            canService.addComponent((void*)&buttonStart, (IMessageHandler<CANMessage>*)&buttonMessageHandler, NORMAL);
        }

        // Called repeately after bootup
        void loop() {
            canService.run();

            if (buttonReset.getStateChanged()) {
                canService.sendMessage((void*)&buttonReset);
            }

            if (buttonStart.getStateChanged()) {
                canService.sendMessage((void*)&buttonStart);
            }
        }
};

Dashboard runtime;

#endif