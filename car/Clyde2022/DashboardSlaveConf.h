#ifndef DASHBOARDSLAVECONF_H
#define DASHBOARDSLAVECONF_H

#include "carpi.h"

#include "hardware/Pins_Dashboard.h"

#define BUTTON_RESEND_INTERVAL 3 // hz

#define REPORT_CAN_ERROR

CANService canService(DASHBOARD_CAN);

//LED's
HardwareLed ledRed(DASHBOARD_PIN_LED_RED, COMPONENT_LED_ERROR);
HardwareLed ledYellow(DASHBOARD_PIN_LED_YELLOW, COMPONENT_LED_ISSUE);
HardwareLed ledGreen(DASHBOARD_PIN_LED_GREEN, COMPONENT_LED_READY_TO_DRIVE);

// Buttons
HardwareInterruptButton buttonReset(DASHBOARD_PIN_BUTTON_RESET, COMPONENT_BUTTON_RESET);
HardwareInterruptButton buttonStart(DASHBOARD_PIN_BUTTON_START, COMPONENT_BUTTON_START);
HardwareInterruptButton buttonCal(DASHBOARD_PIN_CAL, COMPONENT_BUTTON_CAL);

class Dashboard : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            canService.setSenderId(DEVICE_DASHBOARD);

            canService.addComponent((ICommunication*)&ledRed);
            canService.addComponent((ICommunication*)&ledYellow);
            canService.addComponent((ICommunication*)&ledGreen);
            canService.addComponent((ICommunication*)&buttonReset);
            canService.addComponent((ICommunication*)&buttonStart);
            canService.addComponent((ICommunication*)&buttonCal);

            _resendTimer.reset();
            _resendTimer.start();
        }

        // Called repeately after bootup
        void loop() {
            canService.run();

            canService.sendMessage((ICommunication*)&buttonReset, DEVICE_MASTER);
            canService.sendMessage((ICommunication*)&buttonStart, DEVICE_MASTER);

            _resendTimer.reset();
            _resendTimer.start();
            while (_resendTimer.read() < BUTTON_RESEND_INTERVAL) {
                canService.run();

                button_state_t startState = buttonStart.getState();
                if (startState != _lastSentStart) {
                    _lastSentStart = startState;
                    canService.sendMessage((ICommunication*)&buttonStart, DEVICE_MASTER);
                }

                button_state_t resetState = buttonReset.getState();
                if (resetState != _lastSentReset) {
                    _lastSentReset = resetState;
                    canService.sendMessage((ICommunication*)&buttonReset, DEVICE_MASTER);
                }

                wait(0.01);
            }
        }
    
    private:
        Timer _resendTimer;

        button_state_t _lastSentStart = NOT_PRESSED;
        button_state_t _lastSentReset = NOT_PRESSED;
};

Dashboard runtime;

#endif