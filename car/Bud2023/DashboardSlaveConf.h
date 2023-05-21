#ifndef DASHBOARDSLAVECONF_H
#define DASHBOARDSLAVECONF_H

// #define REPORT_CAN_ERROR

#include "carpi.h"

#include "hardware/Pins_Dashboard.h"

#define BUTTON_RESEND_INTERVAL 3 // hz

CANService canService(DASHBOARD_CAN);

// LED's
HardwareLed ledRed(DASHBOARD_PIN_LED_RED, COMPONENT_LED_RED);
HardwareLed ledGreen(DASHBOARD_PIN_LED_GREEN, COMPONENT_LED_GREEN);
HardwareLed ledBlue(DASHBOARD_PIN_LED_BLUE, COMPONENT_LED_BLUE);
HardwareLed ledCal(DASHBOARD_PIN_LED_CAL, COMPONENT_LED_CAL);
HardwareLed ledRtd(DASHBOARD_PIN_LED_RTD, COMPONENT_LED_READY_TO_DRIVE);
HardwareLed ledLct(DASHBOARD_PIN_LED_LCT, COMPONENT_LED_LAUNCH_CONTROL);
// HardwareLed ledImd(DASHBOARD_PIN_LED_IMD, COMPONENT_LED_IMD);
// HardwareLed ledBms(DASHBOARD_PIN_LED_BMS, COMPONENT_LED_BMS);

// Buttons
HardwareInterruptButton buttonReset(DASHBOARD_PIN_BUTTON_RESET, COMPONENT_BUTTON_RESET);
HardwareInterruptButton buttonStart(DASHBOARD_PIN_BUTTON_START, COMPONENT_BUTTON_START);
HardwareInterruptButton buttonCal(DASHBOARD_PIN_CAL, COMPONENT_BUTTON_CAL);

class Dashboard : public Carpi
{
public:
    // Called once at bootup
    void setup()
    {
        canService.setSenderId(DEVICE_DASHBOARD);

        canService.addComponent((ICommunication *)&ledRed);
        canService.addComponent((ICommunication *)&ledGreen);
        canService.addComponent((ICommunication *)&ledBlue);
        canService.addComponent((ICommunication *)&ledCal);
        canService.addComponent((ICommunication *)&ledRtd);
        canService.addComponent((ICommunication *)&ledLct);
        canService.addComponent((ICommunication *)&buttonReset);
        canService.addComponent((ICommunication *)&buttonStart);
        canService.addComponent((ICommunication *)&buttonCal);
        // canService.addComponent((ICommunication *)&buttonTsOn);
        //       canService.addComponent((ICommunication *)&ledImd);
        //         canService.addComponent((ICommunication *)&ledBms);

        _resendTimer.reset();
        _resendTimer.start();
    }

    // Called repeately after bootup
    void loop()
    {
        canService.run();

        canService.sendMessage((ICommunication *)&buttonReset, DEVICE_MASTER);
        canService.sendMessage((ICommunication *)&buttonStart, DEVICE_MASTER);
        canService.sendMessage((ICommunication *)&buttonCal, DEVICE_MASTER);
        //        canService.sendMessage((ICommunication *)&buttonTsOn, DEVICE_MASTER);

        _resendTimer.reset();
        _resendTimer.start();
        while (_resendTimer.read() < BUTTON_RESEND_INTERVAL)
        {
            canService.run();

            button_state_t startState = buttonStart.getState();
            if (startState != _lastSentStart)
            {
                _lastSentStart = startState;
                canService.sendMessage((ICommunication *)&buttonStart, DEVICE_MASTER);
            }

            button_state_t resetState = buttonReset.getState();
            if (resetState != _lastSentReset)
            {
                _lastSentReset = resetState;
                canService.sendMessage((ICommunication *)&buttonReset, DEVICE_MASTER);
            }

            button_state_t calState = buttonCal.getState();
            if (calState != _lastSentCal)
            {
                _lastSentCal = resetState;
                canService.sendMessage((ICommunication *)&buttonCal, DEVICE_MASTER);
            }

            /*
                button_state_t tsOnState = buttonTsOn.getState();
                if (tsOnState != _lastSentCal)
                {
                    _lastSentTsOn = resetState;
                    canService.sendMessage((ICommunication *)&buttonCal, DEVICE_MASTER);
                }
            */
            wait_ms(10);
        }
    }

private:
    Timer _resendTimer;

    button_state_t _lastSentStart = NOT_PRESSED;
    button_state_t _lastSentReset = NOT_PRESSED;
    button_state_t _lastSentCal = NOT_PRESSED;
    button_state_t _lastSentTsOn = NOT_PRESSED;
};

Dashboard runtime;

#endif
