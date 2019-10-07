#ifndef DASHBOARDDEMO_H
#define DASHBOARDDEMO_H

#include "carpi.h"

#define REFRESH_RATE 1

#define RED_MAX 0.8
#define YELLOW_MAX 0.8
#define GREEN_MAX 0.8

#include "../../Bonnie2019/hardware/Pins_Dashboard_PCB.h"

//LED's
HardwareLedPwm ledRed(DASHBOARD_PIN_LED_RED, COMPONENT_LED_ERROR);
HardwareLedPwm ledYellow(DASHBOARD_PIN_LED_YELLOW, COMPONENT_LED_ISSUE);
HardwareLedPwm ledGreen(DASHBOARD_PIN_LED_GREEN, COMPONENT_LED_READY_TO_DRIVE);


// Buttons
HardwareInterruptButton buttonReset(DASHBOARD_PIN_BUTTON_RESET, COMPONENT_BUTTON_RESET);
HardwareInterruptButton buttonStart(DASHBOARD_PIN_BUTTON_START, COMPONENT_BUTTON_START);

class Dashboard {
    public:
        // Called once at bootup
        void setup() {
            pcSerial.printf("Dashboard Demo 2");
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

                    ledRed.setBrightness(RED_MAX);
                    ledRed.setBlinking(BLINKING_FAST);

                    ledYellow.setBrightness(YELLOW_MAX);
                    ledYellow.setBlinking(BLINKING_FAST);

                    ledGreen.setBrightness(GREEN_MAX);
                    ledGreen.setBlinking(BLINKING_FAST);
                }
            } else {
                if (_buttonPressedBefore) {
                    _buttonPressedBefore = false;

                    ledRed.setBlinking(BLINKING_OFF);
                    ledYellow.setBlinking(BLINKING_OFF);
                    ledGreen.setBlinking(BLINKING_OFF);

                    ledRed.setBrightness((float)_last.red / 255 * RED_MAX);
                    ledYellow.setBrightness((float)_last.yellow / 255 * YELLOW_MAX);
                    ledGreen.setBrightness((float)_last.green / 255 * GREEN_MAX);
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

                ledRed.setBrightness((float)_last.red / 255.0 * RED_MAX);
                ledYellow.setBrightness((float)_last.yellow / 255.0 * YELLOW_MAX);
                ledGreen.setBrightness((float)_last.green / 255.0 * GREEN_MAX);

                //pcSerial.printf("%i\t%i\t%i\n", _last.red, _last.yellow, _last.green);
            }

            wait((1.0 / (float)REFRESH_RATE / 255.0));
        }
    
    private:
        bool _buttonPressedBefore = true;

        struct _last {
            uint8_t red = 0, yellow = 85, green = 170;
            bool upRed = true, upYellow = true, upGreen = true;
        } _last;
};

Dashboard DashboardDemoObject;
void DashboardDemo() {
    DashboardDemoObject.setup();

    while(true) {
        DashboardDemoObject.loop();
    }
}

#endif // DASHBOARDDEMO_H