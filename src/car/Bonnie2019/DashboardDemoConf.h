#ifndef DASHBOARDDEMOCONF_H
#define DASHBOARDDEMOCONF_H

#include "mbed.h"

#define REFRESH_RATE 1

#ifndef MESSAGE_REPORT
    #define MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

#include "hardware/Pins_Dashboard_PCB.h"
#include "../../components/hardware/HardwareInterruptButton.h"
#include "../../components/hardware/HardwareLed.h"

//LED's
HardwareLed ledRed(DASHBOARD_PIN_LED_RED, LED_ERROR);
HardwareLed ledYellow(DASHBOARD_PIN_LED_YELLOW, LED_ISSUE);
HardwareLed ledGreen(DASHBOARD_PIN_LED_GREEN, LED_READY_TO_DRIVE);


// Buttons
HardwareInterruptButton buttonReset(DASHBOARD_PIN_BUTTON_RESET, BUTTON_RESET);
HardwareInterruptButton buttonStart(DASHBOARD_PIN_BUTTON_START, BUTTON_START);

class Dashboard {
    public:
        // Called once at bootup
        void setup() {
            ledRed.setState(LED_ON);
            ledYellow.setState(LED_ON);
            ledGreen.setState(LED_ON);
        }

        // Called repeately after bootup
        void loop() {
            if (buttonStart.getState() == PRESSED ||
                buttonStart.getState() == LONG_CLICKED ||
                buttonReset.getState() == PRESSED ||
                buttonReset.getState() == LONG_CLICKED) {
                if (!_buttonPressedBefore) {
                    _buttonPressedBefore = true;

                    ledRed.setBrightness(1);
                    ledRed.setBlinking(BLINKING_FAST);

                    ledYellow.setBrightness(1);
                    ledYellow.setBlinking(BLINKING_FAST);

                    ledGreen.setBrightness(1);
                    ledGreen.setBlinking(BLINKING_FAST);
                }
            } else {
                if (_buttonPressedBefore) {
                    _buttonPressedBefore = false;

                    ledRed.setBlinking(BLINKING_OFF);
                    ledYellow.setBlinking(BLINKING_OFF);
                    ledGreen.setBlinking(BLINKING_OFF);

                    ledRed.setBrightness((float)_last.red / 255);
                    ledYellow.setBrightness((float)_last.yellow / 255);
                    ledGreen.setBrightness((float)_last.green / 255);
                }

                if (_last.upRed) {
                    _last.red++;
                    if (_last.red == 255) {
                        _last.upRed = false;
                    }
                } else {
                    _last.red--;
                    if (_last.red == 0) {
                        _last.upRed = true;
                    }
                }

                if (_last.upYellow) {
                    _last.yellow++;
                    if (_last.yellow == 255) {
                        _last.upYellow = false;
                    }
                } else {
                    _last.yellow--;
                    if (_last.yellow == 0) {
                        _last.upYellow = true;
                    }
                }

                if (_last.upGreen) {
                    _last.green++;
                    if (_last.green == 255) {
                        _last.upGreen = false;
                    }
                } else {
                    _last.green--;
                    if (_last.green == 0) {
                        _last.upGreen = true;
                    }
                }

                ledRed.setBrightness((float)_last.red / 255);
                ledYellow.setBrightness((float)_last.yellow / 255);
                ledGreen.setBrightness((float)_last.green / 255);
            }

            wait(1 / REFRESH_RATE / 255);
        }
    
    private:
        bool _buttonPressedBefore = true;

        struct _last {
            uint8_t red = 0, yellow = 85, green = 170;
            bool upRed = true, upYellow = true, upGreen = true;
        } _last;
};

Dashboard runtime;

#endif // DASHBOARDDEMOCONF_H