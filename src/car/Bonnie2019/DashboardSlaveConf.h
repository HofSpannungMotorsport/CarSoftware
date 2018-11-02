#ifndef BONNIE_2019_DASHBOARD_CONF_H
#define BONNIE_2019_DASHBOARD_CONF_H

#include "mbed.h"
#include "hardware/Pins_Dashboard.h"
#include "../../components/hardware/HardwareInterruptButton.h"
#include "../../components/hardware/HardwareLed.h"
#include "../../can/LEDMessageHandler.h"

static LEDMessageHandler ledBridge;
//static ButtonMessageHandler ledBridge;

HardwareLed ledGreen(DASHBOARD_PIN_LED_1);
HardwareLed ledYellow(DASHBOARD_PIN_LED_2);
HardwareLed ledRed(DASHBOARD_PIN_LED_3);


// Buttons
//BTN-Oins: D9, D7
HardwareInterruptButton buttonLower(DASHBOARD_PIN_BUTTON_LOWER);
HardwareInterruptButton buttonUpper(DASHBOARD_PIN_BUTTON_UPPER);

void initBoardHardware() {
    // assign the components to CANService here
    
}

#endif
